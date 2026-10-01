#include "semantic/analyzer.h"
#include "support/type_traits.h"
namespace cppgm { namespace semantic {
EntityId Analyzer::builtin_type_template(IdentifierId name)
{
    if (!definitions || !name) return 0;
    auto trait = builtin_trait(ids.spelling(name));
    if (!template_type_transform(trait)) return 0;
    // A builtin alias has a real template identity and typed parameter head.
    // Construct it only on lookup, with no invented source or parsed body.
    auto environment = make_scope(ScopeKind::Template,global,0,0,false);
    auto parameter = [&](ScopeId scope, TypeId value_type, bool pack) {
        auto p = make_entity(value_type ? EntityKind::Parameter : EntityKind::Type,scope,0,0);
        entities[p].template_parameter = true; entities[p].parameter_pack = pack;
        entities[p].key = value_type ? TOK_INVALID : KW_TYPENAME;
        entities[p].type = value_type ? value_type : types.named(p);
        return p;
    };
    auto head = [&](ScopeId scope, const std::vector<EntityId>& params) {
        TemplateFunction record; record.environment = scope; record.offset = template_parameters.size(); record.count = params.size();
        for (unsigned j = 0; j < params.size(); ++j) {
            template_parameters.push_back(params[j]); parameter_ordinals.put(params[j],j+1);
        }
        auto id = templates.size(); templates.push_back(record); return id;
    };
    std::vector<EntityId> params; std::vector<ArgumentId> args;
    if (trait == BuiltinTrait::TypePackElement) {
        auto index = parameter(environment,types.fundamental(FT_UNSIGNED_LONG_INT),false);
        auto pack = parameter(environment,0,true);
        params = {index,pack};
        args = {parameter_argument(index),types.pack_expansion(entities[pack].type,0)};
    } else {
        auto sequence = parameter(environment,0,false);
        entities[sequence].key = KW_TEMPLATE;
        entities[sequence].class_info = class_facts.size(); class_facts.push_back(ClassFacts());
        auto nested = make_scope(ScopeKind::Template,environment,0,0,false);
        auto element = parameter(nested,0,false);
        auto values = parameter(nested,entities[element].type,true);
        entities[sequence].template_info = head(nested,{element,values});
        auto type = parameter(environment,0,false);
        auto count = parameter(environment,entities[type].type,false);
        params = {sequence,type,count};
        args = {entities[sequence].type,entities[type].type,parameter_argument(count)};
    }
    auto alias = make_entity(EntityKind::Alias,global,name,0);
    entities[alias].template_info = head(environment,params);
    TypeQuery query; query.kind = QueryKind::BuiltinTrait; query.name = name;
    query.value = unsigned(trait); query.arguments = intern_arguments(args);
    entities[alias].type = types.decltype_type(intern_query(query,{}),false);
    bind(global,name,alias);
    return alias;
}
TypeQueryFact Analyzer::query_template_type_trait(const TypeQuery& query)
{
    auto args = argument_packs[query.arguments];
    auto invalid = TypeQueryFact::failed(TypeQueryFact::Failure::InvalidOperands);
    TypeQueryFact result;
    if (BuiltinTrait(query.value) == BuiltinTrait::TypePackElement) {
        if (!args.count) return invalid;
        auto index = argument_types[args.offset];
        if (!value_argument(index) && types[index].kind == TypeKind::PackExpansion) { result.dependent = true; return result; }
        if (!value_argument(index)) return invalid;
        for (unsigned j = 1; j < args.count; ++j) {
            auto type = argument_types[args.offset+j];
            if (value_argument(type) || (types[type].kind == TypeKind::Named && template_entity(types[type].entity))) return invalid;
        }
        if (dependent_argument(index)) { result.dependent = true; return result; }
        auto converted = convert_argument(index,types.fundamental(FT_UNSIGNED_LONG_INT));
        if (!converted) return invalid;
        auto value = constants[query_value(argument_query(converted))];
        if (!value.valid) return invalid;
        auto position = integer_value(value);
        bool dependent = false;
        for (unsigned j = 1; j < args.count; ++j) {
            auto type = argument_types[args.offset+j];
            if (types[type].kind == TypeKind::PackExpansion) { result.dependent = true; return result; }
            if (position == j-1) { result.expression.type = type; return result; }
            dependent |= dependent_type(type);
        }
        if (dependent) { result.dependent = true; return result; }
        return invalid;
    }
    for (unsigned j = 0; j < args.count; ++j) {
        auto arg = argument_types[args.offset+j];
        if (!value_argument(arg) && types[arg].kind == TypeKind::PackExpansion) { result.dependent = true; return result; }
    }
    if (args.count != 3) return invalid;
    auto target = argument_types[args.offset], type = argument_types[args.offset+1], count = argument_types[args.offset+2];
    if (value_argument(target) || value_argument(type) || !value_argument(count)) return invalid;
    auto entity = types[target].kind == TypeKind::Named ? template_entity(types[target].entity) : 0;
    if (!entity && !dependent_type(target)) return invalid;
    if (!dependent_type(type) && (types[type].kind != TypeKind::Fundamental || !integral(type))) return invalid;
    if (!entity || entities[entity].template_parameter || entities[entity].template_member ||
        dependent_type(type) || dependent_argument(count)) { result.dependent = true; return result; }
    auto converted = convert_argument(count,type);
    if (!converted) return invalid;
    auto value = constants[query_value(argument_query(converted))];
    // Share the integer-pack generator's explicit work/memory ceiling.
    if (!value.valid || negative_constant(value) || integer_value(value) > 1048576) return invalid;
    auto length = std::uint64_t(integer_value(value));
    std::vector<ArgumentId> generated; generated.reserve(length+1); generated.push_back(type);
    for (std::uint64_t i = 0; i < length; ++i) {
        TypeQuery item; item.type = types.unqualified(type); item.value = i;
        generated.push_back(value_argument_id(intern_query(item,{})));
    }
    result.expression.type = apply_type_template(entity,generated);
    return result.expression.type ? result : invalid;
}
} }
