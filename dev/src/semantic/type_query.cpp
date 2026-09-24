#include "semantic/analyzer.h"
#include <stdexcept>
namespace cppgm { namespace semantic {
using syntax::Kind;
QueryId Analyzer::intern_query(TypeQuery q, const std::vector<QueryId>& children)
{
    std::uint64_t hash = 1469598103934665603ULL;
    auto add = [&](std::uint64_t x) { hash = (hash ^ x) * 1099511628211ULL; };
    add(unsigned(q.kind)); add(q.op); add(q.type); add(q.entity); add(q.name);
    add(q.context); add(q.naming); add(q.arguments); add(q.value); add(q.null_pointer_constant);
    for (auto c : children) add(c);
    if (query_slots.empty() || type_queries.size()*2 >= query_slots.size()) {
        query_slots.assign(query_slots.empty() ? 32 : query_slots.size()*2,0);
        for (QueryId i = 1; i < type_queries.size(); ++i) {
            auto pos = query_hashes[i] & (query_slots.size()-1);
            while (query_slots[pos]) pos = (pos+1)&(query_slots.size()-1);
            query_slots[pos] = i;
        }
    }
    auto pos = hash & (query_slots.size()-1);
    while (auto id = query_slots[pos]) {
        auto p = type_queries[id];
        bool same = query_hashes[id] == hash && p.kind == q.kind && p.op == q.op && p.type == q.type &&
            p.entity == q.entity && p.name == q.name && p.context == q.context && p.naming == q.naming &&
            p.arguments == q.arguments && p.value == q.value && p.count == children.size() &&
            p.null_pointer_constant == q.null_pointer_constant;
        for (unsigned i = 0; same && i < p.count; ++i) same = query_edges[p.offset+i] == children[i];
        if (same) return id;
        pos = (pos+1)&(query_slots.size()-1);
    }
    q.offset = query_edges.size(); q.count = children.size();
    query_edges.insert(query_edges.end(),children.begin(),children.end());
    auto id = type_queries.size(); query_slots[pos] = id;
    type_queries.push_back(q); query_hashes.push_back(hash); query_facts.push_back(TypeQueryFact()); return id;
}
QueryId Analyzer::expression_query(NodeId n, ScopeId s, bool callee)
{
    auto& source_index = callee ? query_callee_sources : query_sources;
    if (auto old = source_index.get(key(s,n))) return old;
    TypeQuery q; std::vector<QueryId> children;
    auto node = ast[n]; auto first = node.first;
    switch (node.kind) {
    case Kind::Lambda:
        // An unevaluated lambda is forbidden in C++11. The source binder may
        // still ask for a dependent initializer's type before instantiation.
        if (template_type_probe) return 0;
        throw std::runtime_error("lambda in unevaluated operand");
    case Kind::PackExpression:
        q.kind = QueryKind::Expansion; children.push_back(expression_query(first,s)); break;
    case Kind::New: {
        q.kind = QueryKind::New; q.context = s;
        while (scopes[q.context].kind == ScopeKind::Template || scopes[q.context].kind == ScopeKind::Block)
            q.context = scopes[q.context].parent;
        q.type = type_id(child(n,Kind::TypeId),s); q.value = child(n,Kind::Global) != 0;
        TypeQuery type; type.kind = QueryKind::TypeValue; type.type = q.type;
        std::vector<QueryId> args(1,intern_query(type,{}));
        auto init = child(n,Kind::Initializer);
        auto list = ast[init].first;
        for (auto a = ast[list].first; a; a = ast[a].next) args.push_back(expression_query(a,s));
        TypeQuery call; call.kind = QueryKind::Call; call.context = q.context;
        children.push_back(intern_query(call,args));
        auto placement = ast[child(n,Kind::Placement)].first;
        for (auto a = ast[placement].first; a; a = ast[a].next) children.push_back(expression_query(a,s));
        break;
    }
    case Kind::IdExpression: {
        auto name = node.detail;
        if (callee && (fundamental_cast_type(node.op) || ast[name].kind == Kind::TypeId)) {
            q.kind = QueryKind::TypeValue; q.type = fundamental_cast_type(node.op);
            if (!q.type) q.type = type_id(name,s);
            break;
        }
        auto last = ast[name].first;
        while (ast[last].next && ast[last].next != ast[name].last) last = ast[last].next;
        if (definitions && !ast.nodes.occurrences[n].context && last != ast[name].last && bind_template_name(name,s,last).dependent) {
            auto owner = type_name(name,s,last);
            if (!owner && template_type_probe) return 0;
            q.kind = QueryKind::QualifiedValue; q.type = owner; q.name = terminal(name); q.context = s;
            while (scopes[q.context].kind == ScopeKind::Template || scopes[q.context].kind == ScopeKind::Block)
                q.context = scopes[q.context].parent;
            if (auto list = child(ast[name].last,Kind::TemplateArguments)) {
                std::vector<ArgumentId> args;
                for (auto a = ast[list].first; a; a = ast[a].next)
                    append_template_argument(a,s,template_argument_node(a,s),args);
                q.arguments = intern_arguments(args);
            }
            break;
        }
        auto e = resolve(name,s);
        if (!e && (!callee || ast[name].first != ast[name].last || ast[name].op == OP_COLON2))
            throw std::runtime_error("unbound name in type query");
        auto entity = entities[e];
        if (template_type_probe && entity.template_pattern && !entity.type) return 0;
        if (e && (entity.kind == EntityKind::Alias || entity.kind == EntityKind::Type)) {
            if (!callee) throw std::runtime_error("type name used as value in type query");
            q.kind = QueryKind::TypeValue; q.type = entity.type;
        } else if (entity.template_parameter) {
            q.kind = QueryKind::TemplateValueParameter; q.entity = e; q.type = entity.type;
        } else if (auto ordinal = signature_parameters.get(e)) {
            q.kind = QueryKind::Parameter; q.type = entity.type; q.value = ordinal-1;
        } else {
            q.kind = QueryKind::Name; q.entity = e;
            if (function_binding(e)) q.naming = naming_class(name_owner(name,s));
            if (!function_binding(e)) q.type = entity.type;
            // The source method owns implicit-object cv. Keep the declared
            // field type separately for unparenthesized decltype.
            if (e && nonstatic_field(e)) {
                auto object = template_object_context(s);
                auto implicit = implicit_object_type(s);
                q.value = object.available ? object.cv : implicit ? types[types[implicit].child].cv : 0;
            }
            auto list = child(ast[name].last,Kind::TemplateArguments);
            std::vector<TypeId> arguments;
            for (auto a = ast[list].first; a; a = ast[a].next) {
                auto type = template_argument_node(a,s);
                if (template_type_probe && !type) return 0;
                arguments.push_back(type);
            }
            if (list) q.arguments = intern_arguments(arguments);
            if (ast[name].first == ast[name].last && ast[name].op != OP_COLON2) q.name = terminal(name);
        }
        break;
    }
    case Kind::Literal: case Kind::KeywordLiteral: {
        if (template_type_probe && node.op == KW_THIS) return 0;
        auto value = expression(n,s); q.type = value.type;
        if (node.kind == Kind::Literal && ast.literals[node.literal].kind == LiteralKind::string && !ast.literals[node.literal].suffix) {
            query_literal_sources.put(node.literal,n);
            q.kind = QueryKind::String; q.value = node.literal; break;
        }
        if (node.kind == Kind::Literal && ast.literals[node.literal].suffix) {
            if (template_type_probe) return 0;
            throw std::runtime_error("literal operator call is not an integral constant query");
        }
        auto constant = evaluate(n,s); q.value = constant.valid ? constant.bits : 0;
        q.null_pointer_constant = node.kind == Kind::Literal && constant.valid && !constant.bits && integral(q.type) &&
            ast.literals[node.literal].kind != LiteralKind::character;
        break;
    }
    case Kind::Parenthesized:
        q.kind = QueryKind::Parenthesized; children.push_back(expression_query(first,s,callee)); break;
    case Kind::Conditional:
        q.kind = QueryKind::Conditional; q.context = s;
        while (scopes[q.context].kind == ScopeKind::Template || scopes[q.context].kind == ScopeKind::Block)
            q.context = scopes[q.context].parent;
        for (auto c = first; c; c = ast[c].next) children.push_back(expression_query(c,s));
        break;
    case Kind::Cast: {
        if (node.op != OP_LPAREN && node.op != KW_STATIC_CAST &&
            node.op != KW_CONST_CAST && node.op != KW_REINTERPET_CAST) {
            if (template_type_probe) return 0;
            throw std::runtime_error("cast is not supported in a constant type query");
        }
        q.kind = QueryKind::Cast; q.op = node.op; q.type = type_id(first,s); q.context = s;
        auto child = expression_query(ast[first].next,s);
        if (template_type_probe) {
            if (!q.type || !child) return 0;
            auto type = query_fact(child).expression.type;
            if (!type && !query_fact(child).dependent) return 0;
        }
        children.push_back(child); break;
    }
    case Kind::Unary: case Kind::Postfix: case Kind::Binary: case Kind::Subscript:
        q.kind = node.kind == Kind::Unary || node.kind == Kind::Postfix ? QueryKind::Unary : QueryKind::Binary; q.op = node.op;
        if (node.kind == Kind::Unary && node.op == OP_AMP && ast[first].kind == Kind::IdExpression)
            q.value = ast[ast[first].detail].first != ast[ast[first].detail].last;
        if (node.kind == Kind::Subscript) q.op = OP_LSQUARE;
        q.name = operator_name(q.op); q.context = s;
        while (scopes[q.context].kind == ScopeKind::Template || scopes[q.context].kind == ScopeKind::Block)
            q.context = scopes[q.context].parent;
        if (q.op != OP_LSQUARE && q.op != OP_ASS && q.op != OP_ARROW) {
            auto ordinary = lookup(s,q.name);
            if (function_binding(ordinary)) q.entity = ordinary;
        }
        for (auto c = first; c; c = ast[c].next) children.push_back(expression_query(c,s));
        if (node.kind == Kind::Postfix) {
            TypeQuery zero; zero.type = types.fundamental(FT_INT);
            children.push_back(intern_query(zero,{}));
        }
        break;
    case Kind::Call:
        q.kind = QueryKind::Call; q.context = s;
        while (scopes[q.context].kind == ScopeKind::Template || scopes[q.context].kind == ScopeKind::Block)
            q.context = scopes[q.context].parent;
        children.push_back(expression_query(first,s,true));
        for (auto a = ast[ast[first].next].first; a; a = ast[a].next) children.push_back(expression_query(a,s));
        break;
    case Kind::Member: {
        q.kind = QueryKind::Member; q.op = node.op; q.context = s;
        auto name = ast[ast[first].next].detail;
        q.name = terminal(name);
        if (auto list = child(ast[name].last,Kind::TemplateArguments)) {
            std::vector<ArgumentId> args;
            for (auto a = ast[list].first; a; a = ast[a].next)
                append_template_argument(a,s,template_argument_node(a,s),args);
            q.arguments = intern_arguments(args);
        }
        children.push_back(expression_query(first,s));
        if (ast[name].first != ast[name].last) {
            // Retain the typed naming class, including a dependent qualifier.
            auto object = query_fact(children[0]).expression.type;
            if (node.op == OP_ARROW && pointer(object)) object = types[object].child;
            auto scope = class_value(object) || pattern_class_type(object) ? entities[types[object].entity].scope : s;
            auto qualifier = ast[name].first;
            while (ast[qualifier].next != ast[name].last) qualifier = ast[qualifier].next;
            q.type = type_name(name,scope,qualifier);
            if (!q.type) throw std::runtime_error("invalid query member qualifier");
        }
        break;
    }
    case Kind::SizeofPack: {
        auto occurrence = ast.nodes.occurrences[n];
        auto e = occurrence.context ? pack_size_entities.get(occurrence.source) : 0;
        if (!e) e = lookup(s,node.text);
        if (!e || !entities[e].parameter_pack) throw std::runtime_error("sizeof... requires a pack");
        if (!occurrence.context) pack_size_entities.put(occurrence.source,e);
        auto pack = entity_pack_arguments.get(e);
        if (!pack && occurrence.context) pack = unexpanded_argument(template_type_contexts.get(occurrence.context),e);
        if (pack) { q.type = types.fundamental(FT_UNSIGNED_LONG_INT); q.value = pack_arguments(pack).count; }
        else {
            q.kind = QueryKind::SizeofPack; q.entity = e;
            if (!entities[e].template_parameter) q.value = signature_parameters.get(e)-1;
        }
        break;
    }
    case Kind::Sizeof: case Kind::TypeTrait:
        q.kind = QueryKind::Sizeof; q.op = node.op;
        if (ast[first].kind == Kind::TypeId) {
            q.type = type_id(first,s);
            if (template_type_probe && !q.type) return 0;
        }
        else children.push_back(expression_query(first,s));
        break;
    default:
        if (template_type_probe) return 0;
        throw std::runtime_error("unsupported dependent type query operation");
    }
    if (template_type_probe) {
        if (q.kind == QueryKind::TypeValue && !q.type) return 0;
        for (auto child : children) if (!child) return 0;
    }
    auto id = intern_query(q,children); source_index.put(key(s,n),id); return id;
}
QueryId Analyzer::substitute_query(QueryId id, const Index& bindings, Index& cache, std::uint32_t owner)
{
    if (!query_fact(id).dependent) return id;
    auto cache_key = owner ? key(owner,id) : (std::uint64_t(1)<<63)|id;
    auto& results = owner ? specialization_query_cache : cache;
    if (auto old = results.get(cache_key)) return old;
    auto q = type_queries[id];
    if (q.kind == QueryKind::SizeofPack) {
        auto frame = owner;
        while (frame && substitution_frames[frame].expansion) frame = substitution_frames[frame].parent;
        auto arg = q.arguments ? substitute_argument(argument_types[argument_packs[q.arguments].offset],bindings,cache,frame) :
            frame ? substitution_argument(frame,q.entity) : bindings.get(q.entity);
        int count = -1;
        if (arg && !argument_pack(arg))
            arg = make_argument_pack({types.compound(TypeKind::PackExpansion,0,arg)});
        if (arg) {
            auto pack = pack_arguments(arg); count = pack.count;
            for (unsigned j = 0; j < pack.count; ++j) {
                auto element = argument_types[pack.offset+j];
                if (!value_argument(element) && types[element].kind == TypeKind::PackExpansion) {
                    count = -1; break;
                }
            }
            if (count < 0) {
                TypeQuery symbolic; symbolic.kind = QueryKind::SizeofPack;
                if (pack.count == 1) {
                    auto element = types[argument_types[pack.offset]].bound;
                    if (value_argument(element)) {
                        auto query = type_queries[argument_query(element)];
                        if (query.kind == QueryKind::TemplateValueParameter) symbolic.entity = query.entity;
                    } else if (types[element].kind == TypeKind::Named && entities[types[element].entity].template_parameter)
                        symbolic.entity = types[element].entity;
                }
                if (!symbolic.entity) symbolic.arguments = intern_arguments({arg});
                auto result = intern_query(symbolic,{}); results.put(cache_key,result); return result;
            }
        } else if (q.entity && !entities[q.entity].template_parameter)
            count = expansion_count(expansion_parameters(entities[q.entity].type),bindings,frame);
        if (count < 0) return id;
        TypeQuery value; value.type = types.fundamental(FT_UNSIGNED_LONG_INT); value.value = count;
        auto result = intern_query(value,{}); results.put(cache_key,result); return result;
    }
    if (q.kind == QueryKind::TemplateValueParameter) {
        auto arg = owner ? substitution_argument(owner,q.entity) : bindings.get(q.entity);
        if (!arg) return 0;
        if (!value_argument(arg)) throw std::logic_error("type bound to a value parameter");
        auto result = argument_query(arg); results.put(cache_key,result); return result;
    }
    if (owner && q.context) q.context = substitution_scope(owner,q.context);
    if (owner && q.naming) q.naming = substitution_scope(owner,q.naming);
    if (owner && q.entity) q.entity = substitution_binding(owner,q.entity);
    if (q.type) {
        q.type = substitute_type(q.type,bindings,cache,owner);
        if (!q.type) return 0;
    }
    if (q.arguments) {
        auto pack = argument_packs[q.arguments]; std::vector<TypeId> args;
        for (unsigned j = 0; j < pack.count; ++j)
            substitute_arguments(argument_types[pack.offset+j],bindings,cache,owner,args);
        for (auto arg : args) if (!arg) return 0;
        q.arguments = intern_arguments(args);
    }
    std::vector<QueryId> children;
    for (unsigned i = 0; i < q.count; ++i) {
        auto source = query_edges[q.offset+i]; auto child_query = type_queries[source];
        if (child_query.kind == QueryKind::Expansion) {
            auto pattern = query_edges[child_query.offset];
            auto params = expansion_parameters(0x80000000U|pattern);
            auto count = expansion_count(params,bindings,owner);
            if (count >= 0 && owner) {
                for (int j = 0; j < count; ++j) {
                    auto child = substitute_query(pattern,bindings,cache,expansion_frame(owner,params,j));
                    if (!child) return 0;
                    children.push_back(child);
                }
                continue;
            }
        }
        auto child = substitute_query(source,bindings,cache,owner);
        if (!child) return 0;
        children.push_back(child);
    }
    auto result = intern_query(q,children); results.put(cache_key,result); return result;
}
TypeQueryFact Analyzer::query_fact(QueryId id)
{
    if (query_facts[id].state == FactState::Success) return query_facts[id];
    if (query_facts[id].state == FactState::Failure) {
        if (immediate_query_probe && query_facts[id].failure != TypeQueryFact::Failure::None) return query_facts[id];
        throw std::runtime_error("failed type query");
    }
    if (query_facts[id].state == FactState::Active) throw std::runtime_error("recursive type query");
    query_facts[id].state = FactState::Active;
    struct QueryScope { unsigned& depth; QueryScope(unsigned& d) : depth(d) { ++depth; } ~QueryScope() { --depth; } } guard(unevaluated_depth);
    try {
    ++query_work;
    auto q = type_queries[id]; TypeQueryFact r;
    std::vector<TypeQueryFact> children;
    for (unsigned i = 0; i < q.count; ++i) {
        children.push_back(query_fact(query_edges[q.offset+i])); r.dependent |= children.back().dependent;
        if (children.back().state == FactState::Failure) {
            query_facts[id] = children.back(); return query_facts[id];
        }
    }
    r.dependent |= q.type && dependent_type(q.type);
    r.dependent |= q.kind == QueryKind::TemplateValueParameter || q.kind == QueryKind::SizeofPack || q.kind == QueryKind::Expansion;
    // A template access context belongs to the key, but does not alone make
    // fixed operands dependent: unknown_call(1) must fail at definition time.
    if (q.entity && entities[q.entity].template_pattern) {
        auto entity = entities[q.entity];
        // Runtime object identity does not make its fixed callable type or
        // overload choice dependent. Constants and dependent bit-field widths
        // still require substitution before value-sensitive queries complete.
        bool value_dependent = template_pattern_entities.get(q.entity) == 2 &&
            (field_fact(q.entity).bit_field || (types[q.type].cv & 1 && integral(q.type)));
        value_dependent |= entity.kind == EntityKind::Variable && entity.is_static;
        r.dependent |= !q.type || (entity.kind != EntityKind::Variable && entity.kind != EntityKind::Parameter) || value_dependent;
    }
    if (q.arguments) {
        auto pack = argument_packs[q.arguments];
        for (unsigned i = 0; i < pack.count; ++i) r.dependent |= dependent_argument(argument_types[pack.offset+i]);
    }
    auto& x = r.expression;
    if (q.kind == QueryKind::Sizeof || q.kind == QueryKind::SizeofPack)
        x.type = types.fundamental(q.op == KW_NOEXCEPT ? FT_BOOL : FT_UNSIGNED_LONG_INT);
    // Layout changes the value of sizeof, not its type. Fixed arithmetic
    // operands still impose definition-time obligations, including operands
    // that a later constant evaluation will short-circuit.
    if (r.dependent && (q.kind == QueryKind::Unary || q.kind == QueryKind::Binary || q.kind == QueryKind::Conditional)) {
        bool fixed = true;
        for (auto child : children) fixed &= child.expression.type && arithmetic(child.expression.type);
        if (fixed) {
            r = q.kind == QueryKind::Conditional ? query_conditional(q,children) : query_operator(q,children);
            r.dependent = true;
        }
    }
    bool inspect = !r.dependent;
    if (r.dependent && (q.kind == QueryKind::Name || q.kind == QueryKind::Parameter || q.kind == QueryKind::TemplateValueParameter) && q.type) inspect = true;
    if (r.dependent && q.kind == QueryKind::Member && !children.empty()) {
        auto type = children[0].expression.type;
        if (q.op == OP_ARROW && pointer(type)) type = types[type].child;
        inspect = pattern_class_type(type);
    }
    if (r.dependent && q.kind == QueryKind::Call && !children.empty()) {
        auto fn = children[0].expression;
        auto family = fn.form == ExpressionForm::Overload ? fn.entity : pattern_class_type(fn.type) ?
            lookup(entities[types[fn.type].entity].scope,operator_name(OP_LPAREN),Lookup::Ordinary,true) : 0;
        inspect = family != 0;
        for (unsigned i = 1; inspect && i < children.size(); ++i) inspect &= !children[i].dependent;
        for (auto e : candidates(family)) {
            auto f = types[entities[e].type];
            if (entities[e].template_info || f.kind != TypeKind::Function) { inspect = false; break; }
            for (unsigned i = 0; i < f.count; ++i) inspect &= !dependent_type(types.parameters[f.offset+i]);
        }
    }
    if (inspect) switch (q.kind) {
    case QueryKind::New: r = query_new(q,children); break;
    case QueryKind::String: x.type = q.type; x.category = ValueCategory::Lvalue; break;
    case QueryKind::Value: x.type = q.type; x.null_pointer_constant = q.null_pointer_constant; break;
    case QueryKind::TemplateValueParameter:
        x.type = q.type; r.declared_type = q.type; x.entity = q.entity; break;
    case QueryKind::TypeValue: x.type = q.type; r.declared_type = q.type; break;
    case QueryKind::QualifiedValue: {
        if (!class_value(q.type)) throw std::runtime_error("value qualifier is not a class");
        auto cls = types[q.type].entity; complete_class(cls);
        auto entity = lookup(entities[cls].scope,q.name,Lookup::Ordinary,true);
        if (!entity) throw std::runtime_error("qualified value not found");
        auto kind = entities[entity].kind;
        if (kind != EntityKind::Variable && kind != EntityKind::Enumerator && !function_binding(entity))
            throw std::runtime_error("qualified type used as value");
        check_access(entity,q.context,entities[cls].scope);
        x.type = value_type(entities[entity].type); x.entity = entity;
        r.declared_type = entities[entity].type;
        if (kind != EntityKind::Enumerator) x.category = ValueCategory::Lvalue;
        if (function_binding(entity)) {
            // A template overload family is a concrete lookup result. Its
            // dependent candidate signatures do not make the callee dependent.
            x.form = ExpressionForm::Overload; x.type = 0;
            record_object(x,0,0,0); object_uses[x.object_use].naming_scope = entities[cls].scope;
            if (entities[entity].kind != EntityKind::Function || entities[entity].template_info) r.declared_type = 0;
        }
        break;
    }
    case QueryKind::Parameter: case QueryKind::Name:
        if (!r.dependent && q.entity && entities[q.entity].kind == EntityKind::Variable && entities[q.entity].template_info && q.arguments) {
            auto pack = argument_packs[q.arguments];
            q.entity = specialize_variable(q.entity,std::vector<TypeId>(argument_types.begin()+pack.offset,argument_types.begin()+pack.offset+pack.count));
            q.type = entities[q.entity].type;
        }
        x.type = value_type(q.type); r.declared_type = q.type;
        x.category = ValueCategory::Lvalue; x.entity = q.entity;
        if (q.entity && nonstatic_field(q.entity))
            x = member_value(q.entity,unsigned(q.value),ValueCategory::Lvalue);
        if (q.entity && function_binding(q.entity)) x.form = ExpressionForm::Overload;
        if (q.naming) { record_object(x,0,0,0); object_uses[x.object_use].naming_scope = q.naming; }
        if (q.entity && entities[q.entity].kind == EntityKind::Function && !entities[q.entity].template_info)
            r.declared_type = entities[q.entity].type;
        break;
    case QueryKind::Parenthesized: r = children[0]; r.declared_type = 0; break;
    case QueryKind::Conditional: r = query_conditional(q,children); break;
    case QueryKind::Cast:
        if (integral(q.type) && class_value(children[0].expression.type)) {
            auto c = conversion_function_value(children[0].expression,q.type,q.op != TOK_INVALID);
            if (!c.valid() || deleted_transfer(c.function)) throw std::runtime_error("invalid constant object conversion");
            check_access(c.function,q.context,entities[c.function].owner,children[0].expression.type);
            r.selected = c.function; x.conversions = conversions.size(); x.count = 1; conversions.push_back(c);
        } else if (q.op != TOK_INVALID) {
            auto c = explicit_builtin_conversion(children[0].expression,q.type,q.op,q.context);
            x.conversions = conversions.size(); x.count = 1; conversions.push_back(c);
            if (c.reference) x.category = types[q.type].kind == TypeKind::LRef ? ValueCategory::Lvalue : ValueCategory::Xvalue;
        } else if (!arithmetic(q.type) || !arithmetic(children[0].expression.type))
            throw std::runtime_error("implicit constant query cast requires arithmetic operands");
        x.type = value_type(q.type); break;
    case QueryKind::Member: {
        auto object = children[0].expression; auto type = object.type;
        if (q.op == OP_ARROW) {
            r.arrow = prepare_arrow(object,q.context,0,false);
            if (r.arrow) type = arrow_chains[r.arrow].type;
            if (!pointer(type)) throw std::runtime_error("type query arrow needs pointer");
            type = types[type].child;
        }
        if (!class_value(type) && !pattern_class_type(type)) throw std::runtime_error("type query member needs class");
        auto cls = types[type].entity;
        if (class_value(type)) complete_class(cls);
        if (q.type) {
            if (!class_value(q.type) && !pattern_class_type(q.type)) throw std::runtime_error("query member qualifier is not a class");
            cls = types[q.type].entity;
            if (class_value(q.type)) complete_class(cls);
        }
        auto e = lookup(entities[cls].scope,q.name,Lookup::Ordinary,true);
        if (!e) {
            if (pattern_class_type(type) && template_pattern_open_bases.get(cls)) { r.dependent = true; break; }
            r = TypeQueryFact::failed(TypeQueryFact::Failure::InvalidOperands); break;
        }
        x = member_value(e,types[type].cv,q.op == OP_ARROW ? ValueCategory::Lvalue : object.category);
        if (function_binding(e)) {
            x.form = ExpressionForm::Overload; x.type = 0;
            record_object(x,0,0,0); object_uses[x.object_use].naming_scope = entities[cls].scope;
        }
        else {
            check_access(e,q.context,entities[cls].scope,type); r.declared_type = entities[e].type;
            if (nonstatic_field(e)) record_member_receiver(x,0,type,e,entities[cls].scope,q.type!=0,q.context,false);
        }
        break;
    }
    case QueryKind::Unary: case QueryKind::Binary: r = query_operator(q,children); break;
    case QueryKind::Call: r = query_call(q,children); break;
    case QueryKind::Sizeof:
        if (q.op != KW_NOEXCEPT) size(q.type ? q.type : children[0].expression.type,q.op == KW_ALIGNOF);
        x.type = types.fundamental(q.op == KW_NOEXCEPT ? FT_BOOL : FT_UNSIGNED_LONG_INT); break;
    }
    if (r.state == FactState::Failure) {
        query_facts[id] = r;
        if (immediate_query_probe) return r;
        throw std::runtime_error("invalid type-query expression");
    }
    r.dependent |= r.expression.type && dependent_type(r.expression.type);
    r.state = FactState::Success; query_facts[id] = r; return r;
    } catch (...) { query_facts[id].state = FactState::Failure; throw; }
}
TypeId Analyzer::query_decltype(QueryId id, bool direct)
{
    auto value = query_fact(id);
    if (value.state == FactState::Failure) return 0;
    if (value.dependent) return types.decltype_type(id,direct);
    if (direct && value.declared_type) return value.declared_type;
    auto x = value.expression;
    if (!x.type) throw std::runtime_error("unresolved overload in decltype");
    return x.category == ValueCategory::Prvalue ? x.type :
        types.compound(x.category == ValueCategory::Lvalue ? TypeKind::LRef : TypeKind::RRef,x.type);
}
TypeId Analyzer::dependent_decltype(NodeId n, ScopeId s)
{
    auto query = expression_query(n,s);
    if (!query && template_type_probe) return 0;
    return query_decltype(query,ast[n].kind == Kind::IdExpression || ast[n].kind == Kind::Member);
}
} }
