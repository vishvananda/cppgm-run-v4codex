#include "semantic/analyzer.h"
#include <stdexcept>
namespace cppgm { namespace semantic {
using syntax::Kind;
// Constant variable templates are a fixture-required extension to PA15's
// C++11 slice. Their retained initializer is a typed query, not replayed grammar.
EntityId Analyzer::declare_variable_template(NodeId d, NodeId init, TypeId type, ScopeId s, NodeId source)
{
    auto name = decl_name(d), id = terminal(name);
    auto owner = scopes[s].parent;
    if (scopes[owner].kind != ScopeKind::Namespace || !init || !(types[type].cv & 1) ||
        (!dependent_type(type) && !integral(type))) throw std::runtime_error("constant variable template required");
    auto list = child(ast[name].last,Kind::TemplateArguments);
    auto primary = local(owner,id);
    if (primary && (!list || !entities[primary].template_info || entities[primary].kind != EntityKind::Variable))
        throw std::runtime_error("conflicting variable template");
    if (list && !primary) throw std::runtime_error("variable partial specialization without primary");
    auto e = make_entity(EntityKind::Variable,owner,id,source);
    entities[e].type = type; template_facts(e,s);
    auto head = templates[entities[e].template_info];
    for (unsigned j = 0; j < head.count; ++j) {
        auto p = template_parameters[head.offset+j];
        if (auto value = entities[p].initializer) template_default_types.put(p,template_argument_node(ast[value].first,s));
    }
    if (list) {
        std::vector<TypeId> args;
        for (auto a = ast[list].first; a; a = ast[a].next) args.push_back(template_argument_node(a,s));
        if (!template_defaults(primary,args)) throw std::runtime_error("invalid variable partial arguments");
        auto& selected = templates[entities[e].template_info];
        selected.primary = primary; selected.explicit_arguments = intern_arguments(args);
        variable_partial_next.put(e,variable_partial_heads.get(primary)); variable_partial_heads.put(primary,e);
    } else { bind(s,id,e); bind(owner,id,e); }
    auto operand = ast[init].first;
    if (ast[operand].kind == Kind::BracedInit || ast[operand].kind == Kind::ParenInitializer) operand = ast[operand].first;
    variable_template_queries.put(e,expression_query(operand,s));
    record(s,e,d,type,EntityKind::Variable);
    return e;
}
EntityId Analyzer::variable_template_name(NodeId part, EntityId e, ScopeId s, bool initialize)
{
    if (!e || entities[e].kind != EntityKind::Variable || !entities[e].template_info) return e;
    auto list = child(part,Kind::TemplateArguments);
    if (!list) throw std::runtime_error("variable template requires arguments");
    std::vector<TypeId> args; bool dependent = false;
    for (auto a = ast[list].first; a; a = ast[a].next) {
        auto arg = template_argument_node(a,s); args.push_back(arg); dependent |= dependent_argument(arg);
    }
    if (dependent) return e; // The source query retains the full argument slice.
    return specialize_variable(e,args,initialize);
}
EntityId Analyzer::specialize_variable(EntityId primary, const std::vector<TypeId>& input, bool initialize)
{
    auto args = input;
    if (!template_defaults(primary,args)) throw std::runtime_error("invalid variable template arguments");
    auto pack = intern_arguments(args), index = specialization_index.get(key(primary,pack));
    if (!index) {
        Specialization spec; spec.pattern = primary; spec.arguments = pack;
        spec.entity = make_entity(EntityKind::Variable,entities[primary].owner,entities[primary].name,entities[primary].source);
        spec.declaration = FactState::Success;
        index = specializations.size(); specializations.push_back(spec);
        specialization_index.put(key(primary,pack),index); entities[spec.entity].specialization = index;
        auto head = templates[entities[primary].template_info]; Index bindings, cache;
        for (unsigned j = 0; j < head.count; ++j) bindings.put(template_parameters[head.offset+j],args[j]);
        entities[spec.entity].type = substitute_type(entities[primary].type,bindings,cache);
    }
    auto e = specializations[index].entity;
    if (initialize && specializations[index].body == FactState::Success) ++variable_reuses;
    if (!initialize || entities[e].explicit_specialization || specializations[index].body == FactState::Success) return e;
    if (specializations[index].body != FactState::NotStarted) throw std::runtime_error("recursive or failed variable initializer");
    specializations[index].body = FactState::Active;
    ++variable_initializers;
    try {
        auto selected = primary; Index bindings, cache;
        auto head = templates[entities[primary].template_info];
        for (unsigned j = 0; j < head.count; ++j) bindings.put(template_parameters[head.offset+j],args[j]);
        for (auto p = variable_partial_heads.get(primary); p; p = variable_partial_next.get(p)) {
            ++variable_candidates;
            auto candidate = templates[entities[p].template_info];
            auto pattern = argument_packs[candidate.explicit_arguments]; Index trial;
            bool match = pattern.count == args.size();
            for (unsigned j = 0; match && j < pattern.count; ++j) {
                auto a = argument_types[pattern.offset+j];
                match = value_argument(a) ? a == args[j] : !value_argument(args[j]) && deduce_type(a,args[j],trial);
            }
            for (unsigned j = 0; match && j < candidate.count; ++j) match = trial.get(template_parameters[candidate.offset+j]) != 0;
            if (!match) continue;
            if (selected != primary) throw std::runtime_error("ambiguous variable partial specialization");
            selected = p; bindings = std::move(trial);
        }
        auto type = substitute_type(entities[selected].type,bindings,cache);
        auto query = substitute_query(variable_template_queries.get(selected),bindings,cache);
        auto value = convert(constants[query_value(query)],type);
        if (!value.valid) throw std::runtime_error("nonconstant variable template initializer");
        entities[e].type = type; entities[e].constant = value; entities[e].definition = entities[selected].source;
        specializations[index].body = FactState::Success;
    } catch (...) { specializations[index].body = FactState::Failure; throw; }
    return e;
}
} }
