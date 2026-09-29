#include "semantic/analyzer.h"
#include <stdexcept>
namespace cppgm { namespace semantic {
unsigned Analyzer::capture_object(EntityId object)
{
    auto id = closure_functions.get(current_function);
    if (!id || unevaluated_depth != body_evaluation_depth) return 0;
    if (object) {
        auto e = entities[object];
        if ((e.kind != EntityKind::Variable && e.kind != EntityKind::Parameter) ||
            e.is_static || e.external_decl || e.thread_local_storage ||
            scopes[e.owner].kind == ScopeKind::Namespace || scopes[e.owner].kind == ScopeKind::Class ||
            encloses(entities[current_function].scope,e.owner)) return 0;
    }
    return require_capture(id,object);
}
unsigned Analyzer::require_capture(unsigned id, EntityId object)
{
    auto identity = key(id,object);
    if (auto known = closure_capture_index.get(identity)) return known;
    auto closure = closures[id];
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
