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
        entities[e].constexpr_function = false;
        entities[e].deleted_function = false;
        entities[e].source = source;
        if (entities[e].kind == EntityKind::Variable) {
            entities[e].initializer = 0; entities[e].constant = Constant();
        }
        if (auto m = entities[e].member_info) {
            // Class completion can retain a primary inline body without
            // demanding it. The selected explicit body owns a different fact.
            members[m].body = members[m].declarator = members[m].source = 0;
            members[m].body_environment = 0; members[m].in_class_body = false;
            members[m].deleted = false;
        }
    }
}
EntityId Analyzer::declare_class_specialization(NodeId n, ScopeId s)
{
    auto name = ast[n].detail;
    auto owner = name_owner(name,s);
    auto list = child(ast[name].last,Kind::TemplateArguments);
    auto primary = local(owner,terminal(name),Lookup::Tag);
    if (!list && primary && entities[primary].class_info && !entities[primary].template_info &&
        (entities[primary].template_member || entities[primary].explicit_specialization) && encloses(s,owner)) {
        // A member class specializes the declaration already published by its
        // enclosing specialization. It has no independent template argument list.
        select_explicit_specialization(primary,n);
        return primary;
    }
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
EntityId Analyzer::declare_function_specialization(NodeId name, ScopeId s, TypeId type, ScopeId declared_owner)
{
    auto owner = declared_owner ? declared_owner : name_owner(name,s);
    auto list = child(ast[name].last,Kind::TemplateArguments);
    std::vector<TypeId> supplied;
    for (auto a = ast[list].first; a; a = ast[a].next) supplied.push_back(template_argument_node(a,s));
    EntityId selected = 0; std::vector<EntityId> matching;
    for (auto primary : candidates(local(owner,terminal(name)))) {
        if (!entities[primary].template_info || entities[primary].specialization) continue;
        ++candidate_work;
        auto f = types[entities[primary].type], target = types[type];
        bool pack = f.count && types[types.parameters[f.offset+f.count-1]].kind == TypeKind::PackExpansion;
        if (f.variadic != target.variadic || (pack ? target.count < f.count-1 : f.count != target.count)) continue;
        // Explicit prefixes and completed specializations have distinct keys.
        // Reuse the same typed target deduction as taking a template's address.
        auto instance = supplied.empty() ? primary : specialize(primary,supplied,true);
        if (instance && entities[instance].template_info) instance = deduce_target(instance,type);
        if (!instance || entities[instance].type != type) continue;
        matching.push_back(instance);
        if (!selected || template_more_specialized(instance,selected)) selected = instance;
    }
    for (auto instance : matching) if (instance != selected && !template_more_specialized(selected,instance))
        throw std::runtime_error("ambiguous explicit function specialization");
    if (!selected) throw std::runtime_error("explicit function specialization has no matching primary");
    return selected;
}
} }
