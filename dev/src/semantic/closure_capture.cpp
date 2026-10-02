#include "semantic/analyzer.h"
#include <stdexcept>
namespace cppgm { namespace semantic {
void Analyzer::bind_template_captures(NodeId source, ScopeId scope, EntityId function)
{
    using syntax::Kind;
    std::vector<EntityId> candidates; Index seen;
    auto add = [&](EntityId e) {
        if (e) {
            if (entities[e].template_parameter || entities[e].is_static || entities[e].external_decl ||
                entities[e].thread_local_storage || scopes[entities[e].owner].kind == ScopeKind::Namespace ||
                encloses(entities[function].scope,entities[e].owner)) return;
            if (scopes[entities[e].owner].kind == ScopeKind::Class) e = 0;
            else if (entities[e].kind != EntityKind::Variable && entities[e].kind != EntityKind::Parameter) return;
        }
        if (!seen.get(e)) { seen.put(e,1); candidates.push_back(e); }
    };
    for (auto c = ast[child(source,Kind::LambdaIntroducer)].first; c; c = ast[c].next) {
        if (ast[c].op == KW_THIS) add(0);
        else {
            auto name = ast[c].op == OP_AMP ? ast[ast[c].detail].text : ast[c].text;
            if (name) if (auto e = lookup(scope,name,Lookup::Ordinary)) add(e);
        }
    }
    std::vector<NodeId> work(1,child(source,Kind::Compound));
    while (!work.empty()) {
        auto n = work.back(); work.pop_back(); auto node = ast[n];
        if (node.kind == Kind::Sizeof || node.kind == Kind::SizeofPack || node.kind == Kind::Decltype ||
            node.kind == Kind::Noexcept || node.kind == Kind::TypeTrait ||
            (node.kind == Kind::DeclSpecifier && node.op == KW_DECLTYPE)) continue;
        if (node.kind == Kind::Lambda) {
            // Nested bodies already own their capture edges. Propagate those
            // crossing this operator, without walking the nested body again.
            auto recipe = closure_capture_recipes[closure_capture_patterns.get(ast.nodes.occurrences[n].source)];
            for (unsigned i = 0; i < recipe.count; ++i) add(closure_capture_candidates[recipe.begin+i]);
            continue;
        }
        if (node.kind == Kind::IdExpression) {
            auto binding = template_bindings[template_binding_index.get(ast.nodes.occurrences[node.detail].source)];
            if (binding.entity) add(binding.entity);
            continue;
        }
        if (node.kind == Kind::KeywordLiteral && node.op == KW_THIS) { add(0); continue; }
        if (node.detail) work.push_back(node.detail);
        for (auto c = node.first; c; c = ast[c].next) work.push_back(c);
    }
    ClosureCapturePattern recipe; recipe.begin = closure_capture_candidates.size(); recipe.count = candidates.size();
    closure_capture_candidates.insert(closure_capture_candidates.end(),candidates.begin(),candidates.end());
    closure_capture_patterns.put(ast.nodes.occurrences[source].source,closure_capture_recipes.size());
    closure_capture_recipes.push_back(recipe);
}
void Analyzer::prepare_template_captures(unsigned id, ScopeId scope)
{
    auto source = closures[id].source;
    auto recipe = closure_capture_recipes[closure_capture_patterns.get(ast.nodes.occurrences[source].source)];
    auto frame = template_type_contexts.get(ast.nodes.occurrences[source].context);
    for (unsigned i = 0; i < recipe.count; ++i) {
        auto e = closure_capture_candidates[recipe.begin+i];
        if (!e) { require_capture(id,0); continue; }
        if (entities[e].template_pattern) e = substitution_binding(frame,e);
        if (!closures[id].capture_default && entities[e].constant.valid) continue;
        if (!encloses(entities[e].owner,scope)) continue;
        if (auto pack = entity_pack_arguments.get(e)) {
            auto elements = pack_arguments(pack);
            for (unsigned j = 0; j < elements.count; ++j) require_capture(id,argument_types[elements.offset+j]);
        } else require_capture(id,e);
    }
}
unsigned Analyzer::capture_object(EntityId object)
{
    auto id = closure_functions.get(current_function);
    bool default_use = active_default_fact && unevaluated_depth == 1;
    if (!default_use && (!current_function || unevaluated_depth != body_evaluation_depth)) return 0;
    if (object) {
        auto e = entities[object];
        if ((e.kind != EntityKind::Variable && e.kind != EntityKind::Parameter) ||
            e.is_static || e.external_decl || e.thread_local_storage ||
            scopes[e.owner].kind == ScopeKind::Namespace || scopes[e.owner].kind == ScopeKind::Class) return 0;
        if (default_use) throw std::runtime_error("automatic object used in default argument");
        if (encloses(entities[current_function].scope,e.owner)) return 0;
        if (!id) throw std::runtime_error("automatic object used across function boundary");
    }
    if (!id) return 0;
    return require_capture(id,object);
}
unsigned Analyzer::require_capture(unsigned id, EntityId object)
{
    auto identity = key(id,object);
    if (auto known = closure_capture_index.get(identity)) return known;
    auto closure = closures[id];
    if (object && closure.default_argument)
        throw std::runtime_error("default argument cannot capture an automatic object");
    // A capture chain may traverse enclosing lambdas, but an ordinary local
    // class member is a function boundary, not an implicit capture edge.
    if (object && !closure.parent &&
        !encloses(entities[closure.enclosing].scope,entities[object].owner))
        throw std::runtime_error("capture crosses an ordinary function boundary");
    if (class_facts[entities[closure.entity].class_info].layout_state != FactState::NotStarted)
        throw std::logic_error("capture added after closure layout");
    if (!closure.capture_default)
        throw std::runtime_error("object requires lambda capture");
    if (!object && !closure.this_type) throw std::runtime_error("this outside nonstatic member");
    ClosureCapture capture; capture.object = object;
    capture.by_copy = object && closure.capture_default == 2;
    if (closure.parent && (!object || !encloses(entities[closure.enclosing].scope,entities[object].owner)))
        capture.source = require_capture(closure.parent,object);
    auto scope = entities[closure.entity].scope;
    // Fields are ABI storage, not source bindings. Lookup retains the original
    // declaration; each evaluated use records its capture identity separately.
    auto name = ids.intern(TextView("__capture",9));
    capture.field = make_entity(EntityKind::Variable,scope,name,0);
    capture.source_type = object ? value_type(entities[object].type) : closure.this_type;
    if (capture.source && object) {
        auto outer = closure_captures[capture.source];
        capture.source_type = value_type(entities[outer.field].type);
        if (outer.by_copy) capture.source_type = types.qualify(capture.source_type,types[entities[closure.enclosing].type].cv);
    }
    auto type = !object || capture.by_copy ? capture.source_type : types.compound(TypeKind::LRef,capture.source_type);
    if (capture.by_copy && types[type].kind == TypeKind::Function)
        type = types.compound(TypeKind::LRef,type);
    entities[capture.field].type = type;
    record(scope,capture.field,0,type,EntityKind::Variable);
    auto index = closure_captures.size(); closure_captures.push_back(capture);
    closure_capture_index.put(identity,index);
    if (closure.last_capture) closure_captures[closure.last_capture].next = index;
    else closures[id].first_capture = index;
    closures[id].last_capture = index;
    // Capturing an address invalidates the scalar's private-storage proof even
    // while checking an as-yet undemanded call operator.
    if (object && !capture.by_copy && types[entities[object].type].kind == TypeKind::MemberPointer) member_pointer_exposed.put(object,1);
    if (object && !capture.by_copy && private_scalar(object)) { scalar_observations.put(object,1); ++scalar_observation_count; }
    return index;
}
TypeId Analyzer::capture_type(unsigned id)
{
    auto capture = closure_captures[id];
    auto type = value_type(entities[capture.field].type);
    if (capture.by_copy) type = types.qualify(type,types[entities[current_function].type].cv);
    return type;
}
void Analyzer::prepare_capture_initializers(unsigned id, ScopeId scope)
{
    // Construction belongs to evaluation of the lambda expression, independently
    // of whether its call operator is ever demanded. Nested expressions record
    // the dependency on their enclosing operator through the normal demand owner.
    for (auto i = closures[id].first_capture; i; i = closure_captures[i].next) {
        auto capture = closure_captures[i];
        if (!capture.by_copy) continue;
        Expression source; source.type = capture.source_type; source.category = ValueCategory::Lvalue;
        TypeId target = entities[capture.field].type;
        while (types[target].kind == TypeKind::Array) { target = types[target].child; source.type = types[source.type].child; }
        auto c = class_value(target) ? transfer_initialization(source,target,InitializationMode::Direct) : standard_conversion(source,target);
        if (!c.valid()) throw std::runtime_error("invalid copy capture");
        c = prepare_typed_conversion(source,c,scope,true);
        if (class_value(target)) default_destructor(target,scope);
        closure_captures[i].conversion = conversions.size(); conversions.push_back(c);
    }
}
void Analyzer::prepare_captures(unsigned id, ScopeId scope)
{
    using syntax::Kind;
    auto first = ast[child(closures[id].source,Kind::LambdaIntroducer)].first;
    closures[id].has_introducer = first != 0;
    if (!first) return;
    if (!closures[id].enclosing) throw std::runtime_error("capture requires block scope");
    Index explicit_names, parameters;
    auto d = child(closures[id].source,Kind::LambdaDeclarator);
    for (auto p = ast[child(d,Kind::Parameters)].first; p; p = ast[p].next)
        if (ast[p].kind == Kind::Parameter) {
            auto name = terminal(decl_name(ast[ast[p].first].next));
            if (name) parameters.put(name,1);
        }
    for (auto n = first; n; n = ast[n].next) {
        auto op = ast[n].op;
        if ((op == OP_AMP && !ast[n].detail) || op == OP_ASS) {
            if (n != first) throw std::runtime_error("capture default must be first");
            closures[id].capture_default = op == OP_AMP ? 1 : 2; continue;
        }
        EntityId object = 0;
        if (op == KW_THIS && closures[id].capture_default == 2)
            throw std::runtime_error("explicit this with value default is not C++11");
        if (op != KW_THIS) {
            auto name = op == OP_AMP ? ast[ast[n].detail].text : ast[n].text;
            if (!name) throw std::runtime_error("invalid capture");
            if (parameters.get(name)) throw std::runtime_error("capture conflicts with lambda parameter");
            if ((op == OP_AMP && closures[id].capture_default == 1) ||
                (op != OP_AMP && closures[id].capture_default == 2)) throw std::runtime_error("redundant capture");
            object = lookup(scope,name,Lookup::Ordinary);
            if (!object || (entities[object].kind != EntityKind::Variable && entities[object].kind != EntityKind::Parameter) ||
                entities[object].is_static || entities[object].external_decl || entities[object].thread_local_storage ||
                scopes[entities[object].owner].kind == ScopeKind::Namespace ||
                scopes[entities[object].owner].kind == ScopeKind::Class)
                throw std::runtime_error("capture requires automatic local");
        }
        if (explicit_names.get(object)) throw std::runtime_error("duplicate capture");
        explicit_names.put(object,1);
        auto pack = entity_pack_arguments.get(object);
        if (bool(pack) != bool(child(n,Kind::ParameterPack))) throw std::runtime_error("invalid capture pack expansion");
        auto saved = closures[id].capture_default;
        closures[id].capture_default = op == OP_AMP || op == KW_THIS ? 1 : 2;
        if (pack) {
            auto elements = pack_arguments(pack);
            for (unsigned lane = 0; lane < elements.count; ++lane)
                require_capture(id,argument_types[elements.offset+lane]);
        } else require_capture(id,object);
        closures[id].capture_default = saved;
    }
}
} }
