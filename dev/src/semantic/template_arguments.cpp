#include "semantic/analyzer.h"
#include <stdexcept>
namespace cppgm { namespace semantic {
using syntax::Kind;
ArgumentId Analyzer::value_argument_id(QueryId query)
{
    if (!query) return 0;
    // A class expression retains its typed construction query until the
    // non-type parameter supplies the target of its user-defined conversion.
    if (!query_fact(query).dependent && !class_value(query_fact(query).expression.type)) {
        auto value = constants[query_value(query)];
        if (!value.valid || !integral(value.type)) throw std::runtime_error("integral constant template argument required");
        TypeQuery q; q.type = types.unqualified(value.type); q.value = value.bits;
        query = intern_query(q,{});
    }
    if (query >= 0x80000000U) throw std::runtime_error("template value identity capacity exceeded");
    return query | 0x80000000U;
}
ArgumentId Analyzer::parameter_argument(EntityId parameter)
{
    if (entities[parameter].kind == EntityKind::Type) return entities[parameter].type;
    TypeQuery q; q.kind = QueryKind::TemplateValueParameter;
    q.entity = parameter; q.type = entities[parameter].type;
    return value_argument_id(intern_query(q,{}));
}
ArgumentId Analyzer::canonical_argument(EntityId parameter, unsigned ordinal, Index& bindings, Index& cache, unsigned depth)
{
    while (canonical_parameters.size() <= ordinal) {
        auto e = make_entity(EntityKind::Type,0,0,0);
        entities[e].template_parameter = true; entities[e].type = types.named(e);
        canonical_parameters.push_back(entities[e].type);
    }
    auto position = depth ? intern_arguments({depth,ordinal+1}) : ordinal+1;
    if (entities[parameter].key == KW_TEMPLATE) {
        auto shape = template_head_shape(parameter,bindings,cache,depth+1);
        auto identity = key(shape,intern_arguments({depth,ordinal+1}));
        auto e = canonical_template_parameters.get(identity);
        if (!e) {
            e = make_entity(EntityKind::Type,0,0,0);
            entities[e].template_parameter = true; entities[e].key = KW_TEMPLATE;
            entities[e].type = types.named(e); entities[e].template_info = entities[parameter].template_info;
            entities[e].class_info = class_facts.size(); class_facts.push_back(ClassFacts());
            canonical_template_parameters.put(identity,e);
        }
        return entities[e].type;
    }
    if (entities[parameter].kind == EntityKind::Type) {
        if (!depth) return canonical_parameters[ordinal];
        auto e = canonical_nested_parameters.get(position);
        if (!e) {
            e = make_entity(EntityKind::Type,0,0,0);
            entities[e].template_parameter = true; entities[e].type = types.named(e);
            canonical_nested_parameters.put(position,e);
        }
        return entities[e].type;
    }
    auto type = substitute_type(entities[parameter].type,bindings,cache);
    if (!type) throw std::runtime_error("missing template parameter type environment");
    auto k = key(type,intern_arguments({depth,ordinal+1}));
    auto e = canonical_value_parameters.get(k);
    if (!e) {
        e = make_entity(EntityKind::Parameter,0,0,0);
        entities[e].template_parameter = true; entities[e].type = type;
        canonical_value_parameters.put(k,e);
    }
    return parameter_argument(e);
}
ArgumentId Analyzer::template_argument_node(NodeId n, ScopeId scope)
{
    auto occurrence = ast.nodes.occurrences[n];
    auto arg = template_argument_sources.get(occurrence.source);
    if (!arg) {
        arg = template_argument_node_impl(n,scope);
        if (!occurrence.context && arg) template_argument_sources.put(occurrence.source,arg);
        return arg;
    }
    // An ellipsis is substituted by the argument-list owner, which knows the
    // pack boundaries. Scalar/type arguments consume this retained fact once.
    if (occurrence.context && dependent_argument(arg) &&
        (value_argument(arg) || types[arg].kind != TypeKind::PackExpansion)) {
        Index bindings, cache;
        return substitute_argument(arg,bindings,cache,template_type_contexts.get(occurrence.context));
    }
    return arg;
}
ArgumentId Analyzer::template_argument_node_impl(NodeId n, ScopeId scope)
{
    if (ast[n].kind == Kind::PackExpression)
        return types.compound(TypeKind::PackExpansion,0,template_argument_node(ast[n].first,scope));
    if (ast[n].kind == Kind::TypeId) {
        auto specs = ast[n].first, spec = ast[specs].first;
        if (!ast[spec].next && !ast[specs].next && ast[spec].detail) {
            auto name = ast[spec].detail;
            if (!child(ast[name].last,Kind::TemplateArguments)) {
                auto binding = bind_template_name(name,scope);
                auto e = template_entity(binding.dependent ? binding.entity : resolve(name,scope));
                if (e) { check_access(e,scope,name_owner(name,scope)); auto injected = injected_template_type(e,scope); return injected ? injected : types.named(e); }
            }
        }
        auto type = types.signature(type_id(n,scope));
        auto d = ast[ast[n].first].next;
        return child(d,Kind::ParameterPack) ? types.compound(TypeKind::PackExpansion,0,type) : type;
    }
    // A dependent class alias may not have been recognizable to the parser.
    if (ast[n].kind == Kind::IdExpression) {
        auto name = ast[n].detail;
        auto binding = bind_template_name(name,scope);
        if (binding.dependent && (ast[ast[name].last].flags & 1) && !child(ast[name].last,Kind::TemplateArguments))
            return type_name(name,scope);
        auto e = binding.entity;
        if (!child(ast[ast[n].detail].last,Kind::TemplateArguments))
            if (auto target = template_entity(e)) { check_access(target,scope,name_owner(ast[n].detail,scope)); return types.named(target); }
        if (e && (entities[e].kind == EntityKind::Type || entities[e].kind == EntityKind::Alias))
            return type_name(ast[n].detail,scope);
    }
    return value_argument_id(expression_query(n,scope));
}
bool Analyzer::dependent_argument(ArgumentId arg)
{
    return value_argument(arg) ? query_fact(argument_query(arg)).dependent : dependent_type(arg);
}
ArgumentId Analyzer::substitute_argument(ArgumentId arg, const Index& bindings, Index& cache, std::uint32_t frame)
{
    return value_argument(arg) ? value_argument_id(substitute_query(argument_query(arg),bindings,cache,frame)) :
        substitute_type(arg,bindings,cache,frame);
}
ArgumentId Analyzer::convert_argument(ArgumentId arg, TypeId target)
{
    if (!value_argument(arg)) return 0;
    auto query = argument_query(arg);
    // A cast owns its result type even while dependence defers its expression
    // fact. Reapplying the same conversion must preserve canonical identity.
    auto source_type = type_queries[query].kind == QueryKind::Cast ? type_queries[query].type : query_fact(query).expression.type;
    if (source_type && types.unqualified(source_type) == types.unqualified(target)) return arg;
    if (dependent_type(target) || query_fact(query).dependent) {
        TypeQuery q; q.kind = QueryKind::Cast; q.type = target;
        // An implicit conversion has its own query opcode, retaining the
        // non-narrowing obligation until both type and value are concrete.
        q.op = TOK_INVALID;
        return value_argument_id(intern_query(q,{query}));
    }
    if (class_value(source_type)) {
        TypeQuery q; q.kind = QueryKind::Cast; q.type = target;
        q.op = TOK_INVALID; q.context = type_queries[query].context;
        auto converted = intern_query(q,{query});
        if (!constants[query_value(converted)].valid) return 0;
        return value_argument_id(converted);
    }
    auto value = constants[query_value(query)];
    if (!value.valid || !integral(target)) return 0;
    if ((scoped_enum(value.type) || scoped_enum(target)) && types.unqualified(value.type) != types.unqualified(target)) return 0;
    auto converted = convert(value,types.unqualified(target));
    __int128 before = is_unsigned(value.type) ? __int128(value.bits) : __int128(static_cast<std::int64_t>(value.bits));
    __int128 after = is_unsigned(converted.type) ? __int128(converted.bits) : __int128(static_cast<std::int64_t>(converted.bits));
    if (before != after) return 0;
    TypeQuery q; q.type = converted.type; q.value = converted.bits;
    return value_argument_id(intern_query(q,{}));
}
EntityId Analyzer::bind_argument(ScopeId scope, EntityId parameter, ArgumentId arg)
{
    auto e = make_entity(value_argument(arg) ? EntityKind::Enumerator : EntityKind::Alias,scope,entities[parameter].name,0);
    if (argument_pack(arg)) {
        entity_pack_arguments.put(e,arg); entities[e].parameter_pack = true;
        entities[e].type = entities[parameter].type;
        bind(scope,entities[e].name,e); return e;
    }
    entities[e].type = argument_type(arg);
    if (value_argument(arg) && !dependent_argument(arg)) entities[e].constant = constants[query_value(argument_query(arg))];
    bind(scope,entities[e].name,e);
    return e;
}
} }
