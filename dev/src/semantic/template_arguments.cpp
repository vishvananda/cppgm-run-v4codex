#include "semantic/analyzer.h"
#include <stdexcept>
namespace cppgm { namespace semantic {
using syntax::Kind;
ArgumentId Analyzer::value_argument_id(QueryId query)
{
    if (!query) return 0;
    if (!query_fact(query).dependent) {
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
ArgumentId Analyzer::canonical_argument(EntityId parameter, unsigned ordinal, const Index& bindings, Index& cache)
{
    while (canonical_parameters.size() <= ordinal) {
        auto e = make_entity(EntityKind::Type,0,0,0);
        entities[e].template_parameter = true; entities[e].type = types.named(e);
        canonical_parameters.push_back(entities[e].type);
    }
    if (entities[parameter].kind == EntityKind::Type) return canonical_parameters[ordinal];
    auto type = substitute_type(entities[parameter].type,bindings,cache);
    auto k = key(type,ordinal+1);
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
    if (ast[n].kind == Kind::PackExpression)
        return types.compound(TypeKind::PackExpansion,0,template_argument_node(ast[n].first,scope));
    if (ast[n].kind == Kind::TypeId) {
        auto type = type_id(n,scope);
        auto d = ast[ast[n].first].next;
        return child(d,Kind::ParameterPack) ? types.compound(TypeKind::PackExpansion,0,type) : type;
    }
    // A dependent class alias may not have been recognizable to the parser.
    if (ast[n].kind == Kind::IdExpression) {
        auto binding = bind_template_name(ast[n].detail,scope);
        auto e = binding.entity;
        if (e && (entities[e].kind == EntityKind::Type || entities[e].kind == EntityKind::Alias))
            return source_type(e);
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
    auto source_type = query_fact(query).expression.type;
    if (source_type && types.unqualified(source_type) == types.unqualified(target)) return arg;
    if (dependent_type(target) || query_fact(query).dependent) {
        TypeQuery q; q.kind = QueryKind::Cast; q.type = target;
        // An implicit conversion has its own query opcode, retaining the
        // non-narrowing obligation until both type and value are concrete.
        q.op = TOK_INVALID;
        return value_argument_id(intern_query(q,{query}));
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
