#include "semantic/analyzer.h"
#include <stdexcept>
namespace cppgm { namespace semantic {
using syntax::Kind;
// Explicit declarations publish selection on the same canonical entity used by
// incomplete primary references. No completed primary fact is reinterpreted.
void Analyzer::select_explicit_specialization(EntityId e, NodeId source)
{
    if (!entities[e].explicit_specialization) {
        auto spec = entities[e].specialization;
        if (entities[e].complete || entities[e].definition ||
            (spec && specializations[spec].body != FactState::NotStarted))
            throw std::runtime_error("specialization after instantiation");
        entities[e].explicit_specialization = true;
        ++explicit_selections;
        entities[e].template_member = false;
        entities[e].inline_function = false;
        entities[e].source = source;
        if (entities[e].kind == EntityKind::Variable) {
            entities[e].initializer = 0; entities[e].constant = Constant();
        }
        if (auto m = entities[e].member_info) {
            // Class completion can retain a primary inline body without
            // demanding it. The selected explicit body owns a different fact.
            members[m].body = members[m].declarator = members[m].source = 0;
            members[m].body_environment = 0; members[m].in_class_body = false;
        }
    }
}
EntityId Analyzer::declare_class_specialization(NodeId n, ScopeId s)
{
    auto name = ast[n].detail;
    auto owner = name_owner(name,s);
    auto list = child(ast[name].last,Kind::TemplateArguments);
    auto primary = local(owner,terminal(name),Lookup::Tag);
    if (!list || !primary || !entities[primary].template_info || !entities[primary].class_info || !encloses(s,owner))
        throw std::runtime_error("explicit class specialization requires an enclosing primary");
    std::vector<TypeId> args;
    for (auto a = ast[list].first; a; a = ast[a].next) {
        auto arg = template_argument_node(a,s);
        if (dependent_argument(arg)) throw std::runtime_error("dependent explicit specialization");
        args.push_back(arg);
    }
    auto e = specialize_class(primary,args);
    bool first = !entities[e].explicit_specialization;
    select_explicit_specialization(e,n);
    if (first) {
        // An explicit specialization has its own lexical scope, without the
        // primary's parameter bindings. The prior scope has no completed facts.
        entities[e].scope = make_scope(ScopeKind::Class,owner,entities[e].name,e,false);
        bind(entities[e].scope,entities[e].name,e);
    }
    return e;
}
EntityId Analyzer::declare_function_specialization(NodeId name, ScopeId s, TypeId type)
{
    auto owner = name_owner(name,s);
    auto list = child(ast[name].last,Kind::TemplateArguments);
    std::vector<TypeId> supplied;
    for (auto a = ast[list].first; a; a = ast[a].next) supplied.push_back(template_argument_node(a,s));
    EntityId selected = 0;
    for (auto primary : candidates(local(owner,terminal(name)))) {
        if (!entities[primary].template_info || entities[primary].specialization) continue;
        ++candidate_work;
        auto head = templates[entities[primary].template_info];
        auto f = types[entities[primary].type], target = types[type];
        if (supplied.size() > head.count || f.count != target.count || f.variadic != target.variadic) continue;
        Index bindings, cache;
        bool valid = true;
        for (unsigned j = 0; j < supplied.size(); ++j) {
            auto p = template_parameters[head.offset+j];
            auto arg = supplied[j];
            if (entities[p].kind == EntityKind::Type) valid = !value_argument(arg);
            else {
                auto t = substitute_type(entities[p].type,bindings,cache);
                arg = t && value_argument(arg) ? convert_argument(arg,t) : 0;
                valid = arg != 0;
            }
            if (!valid) break;
            bindings.put(p,arg);
        }
        if (!valid || !deduce_type(entities[primary].type,type,bindings)) continue;
        std::vector<TypeId> args;
        for (unsigned j = 0; j < head.count; ++j) {
            auto arg = bindings.get(template_parameters[head.offset+j]);
            if (!arg) break;
            args.push_back(arg);
        }
        if (!template_defaults(primary,args)) continue;
        auto instance = specialize(primary,args);
        if (!instance || entities[instance].type != type) continue;
        if (selected && !template_more_specialized(instance,selected)) {
            if (template_more_specialized(selected,instance)) continue;
            throw std::runtime_error("ambiguous explicit function specialization");
        }
        selected = instance;
    }
    if (!selected) throw std::runtime_error("explicit function specialization has no matching primary");
    return selected;
}
} }
