#include "semantic/analyzer.h"
#include "support/builtin_registry.h"
#include <stdexcept>
namespace cppgm { namespace semantic {
using syntax::Kind;
ArgumentId Analyzer::value_argument_id(QueryId query)
{
    if (!query) return 0;
    auto fact = query_fact(query);
    if (fact.state == FactState::Failure) return 0;
    // Keep object/function identity and value category until the parameter
    // supplies its target. Reading an lvalue here would lose reference NTTPs;
    // an overload set similarly needs its target function signature first.
    if (!fact.dependent && fact.expression.category == ValueCategory::Prvalue &&
        integral(fact.expression.type) && types[type_queries[query].type].kind != TypeKind::LRef &&
        type_queries[query].kind != QueryKind::IntegerPack) {
        auto value = constants[query_value(query)];
        if (!value.valid || !integral(value.type)) {
            if (immediate_query_probe) return 0;
            throw std::runtime_error("integral constant template argument required");
        }
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
    auto k = key(template_signature_shape(type),intern_arguments({depth,ordinal+1}));
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
    if (ast.kind(n) == Kind::PackExpression) {
        auto operand = ast.first(n), callee = ast.first(operand);
        auto name = ast.detail(callee);
        if (ast.kind(operand) == Kind::Call && ast.kind(callee) == Kind::IdExpression &&
            ast.first(name) == ast.last(name) && integer_pack_builtin(ids.spelling(terminal(name)))) {
            auto list = ast.next(callee), count = ast.first(list);
            if (!count || ast.next(count)) throw std::runtime_error("integer_pack takes one bound");
            TypeQuery q; q.kind = QueryKind::IntegerPack;
            auto id = intern_query(q,{expression_query(count,scope)});
            return types.compound(TypeKind::PackExpansion,0,value_argument_id(id));
        }
        return types.compound(TypeKind::PackExpansion,0,template_argument_node(operand,scope));
    }
    if (ast.kind(n) == Kind::Call && ast.kind(ast.first(n)) == Kind::IdExpression) {
        auto callee = ast.first(n), name = ast.detail(callee);
        auto list = ast.next(callee);
        if (ast.kind(name) == Kind::Name && ast.kind(list) != Kind::BracedInit) {
            auto binding = bind_template_name(name,scope); auto e = binding.entity;
            if (e && (entities[e].kind == EntityKind::Type || entities[e].kind == EntityKind::Alias)) {
                // T(Args...) in a template argument is a function type when
                // every operand denotes a type. The parsed parentheses are
                // shared with functional-cast syntax; semantic construction
                // resolves this ambiguity once, without replaying grammar.
                std::vector<TypeId> parameters; bool function = true;
                for (auto a = ast.first(list); a; a = ast.next(a)) {
                    auto arg = template_argument_node(a,scope);
                    if (!arg || value_argument(arg)) { function = false; break; }
                    parameters.push_back(arg);
                }
                if (function) return types.signature(types.function(type_name(name,scope),parameters,false));
            }
        }
    }
    if (ast.kind(n) == Kind::TypeId) {
        auto specs = ast.first(n), spec = ast.first(specs);
        if (!ast.next(spec) && !ast.next(specs) && ast.detail(spec)) {
            auto name = ast.detail(spec);
            if (!child(ast.last(name),Kind::TemplateArguments)) {
                auto binding = bind_template_name(name,scope);
                auto e = template_entity(binding.dependent ? binding.entity : resolve(name,scope));
                if (e) { check_access(e,scope,name_owner(name,scope)); auto injected = injected_template_type(e,scope); return injected ? injected : types.named(e); }
                if (binding.dependent && (ast.flags(ast.last(name)) & 1))
                    return type_name(name,scope,0,false,true);
            }
        }
        auto type = types.signature(type_id(n,scope));
        auto d = ast.next(ast.first(n));
        return declarator_pack(d) ? types.compound(TypeKind::PackExpansion,0,type) : type;
    }
    // A dependent class alias may not have been recognizable to the parser.
    if (ast.kind(n) == Kind::IdExpression) {
        auto name = ast.detail(n);
        auto binding = bind_template_name(name,scope);
        if (binding.dependent && (ast.flags(ast.last(name)) & 1) && !child(ast.last(name),Kind::TemplateArguments))
            return type_name(name,scope,0,false,true);
        auto e = binding.entity;
        if (e && entities[e].template_pattern && entities[e].kind == EntityKind::Variable &&
            (types[entities[e].type].cv & 1) && integral(entities[e].type)) {
            auto init = entities[e].initializer;
            while (ast.kind(init) == Kind::Initializer) init = ast.first(init);
            // [temp.dep.type] permits chains initialized by the parameter
            // itself. Expressions containing that parameter remain distinct.
            if (ast.kind(init) == Kind::IdExpression)
                return convert_argument(template_argument_node(init,entities[e].owner),entities[e].type);
        }
        if (!child(ast.last(ast.detail(n)),Kind::TemplateArguments))
            if (auto target = template_entity(e)) { check_access(target,scope,name_owner(ast.detail(n),scope)); return types.named(target); }
        if (e && (entities[e].kind == EntityKind::Type || entities[e].kind == EntityKind::Alias))
            return type_name(ast.detail(n),scope);
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
    if (query_fact(query).state == FactState::Failure) return 0;
    // A cast owns its result type even while dependence defers its expression
    // fact. Reapplying the same conversion must preserve canonical identity.
    auto source_type = type_queries[query].kind == QueryKind::Cast ? type_queries[query].type : query_fact(query).expression.type;
    if (source_type && types.unqualified(source_type) == types.unqualified(target) &&
        (type_queries[query].kind == QueryKind::Value || query_fact(query).dependent)) return arg;
    if (dependent_type(target) || query_fact(query).dependent) {
        TypeQuery q; q.kind = QueryKind::Cast; q.type = target;
        // An implicit conversion has its own query opcode, retaining the
        // non-narrowing obligation until both type and value are concrete.
        q.op = TOK_INVALID;
        return value_argument_id(intern_query(q,{query}));
    }
    if (types[target].kind == TypeKind::MemberPointer) return member_address_template_argument(query,target);
    if (pointer(target) || types[target].kind == TypeKind::LRef || fundamental(target,FT_NULLPTR_T))
        return address_template_argument(query,target);
    if (class_value(source_type)) {
        TypeQuery q; q.kind = QueryKind::Cast; q.type = target;
        q.op = TOK_INVALID; q.context = type_queries[query].context;
        auto converted = intern_query(q,{query});
        if (!constants[query_value(converted)].valid) return 0;
        return value_argument_id(converted);
    }
    auto value = constant_indirect(constants[query_value(query)]);
    if (!value.valid || !integral(target)) return 0;
    if ((scoped_enum(value.type) || scoped_enum(target)) && types.unqualified(value.type) != types.unqualified(target)) return 0;
    auto converted = convert(value,types.unqualified(target));
    if (!same_integer_value(value,converted)) return 0;
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
    if (value_argument(arg) && type_queries[argument_query(arg)].kind == QueryKind::Value)
        entities[e].type = type_queries[argument_query(arg)].type;
    if (value_argument(arg) && !dependent_argument(arg)) entities[e].constant = constants[query_value(argument_query(arg))];
    bind(scope,entities[e].name,e);
    return e;
}
} }
