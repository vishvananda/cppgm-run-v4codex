#include "semantic/analyzer.h"
#include <stdexcept>
namespace cppgm { namespace semantic {
void Analyzer::explicit_instantiation(NodeId n, ScopeId s)
{
    auto source = ast[n].first;
    if (ast[source].kind != syntax::Kind::ClassForward) {
        declaration(source,s); return;
    }
    auto e = resolve(ast[source].detail,s,Lookup::Qualifier);
    if (!e || !entities[e].specialization || !entities[e].class_info)
        throw std::runtime_error("explicit instantiation requires a class specialization");
    auto owner = entities[specialization_pattern(e)].owner;
    if (scopes[s].kind != ScopeKind::Namespace || !encloses(s,owner))
        throw std::runtime_error("explicit instantiation outside enclosing namespace");
    auto name = ast[source].detail;
    if (ast[name].first == ast[name].last && ast[name].op != OP_COLON2) {
        auto current = owner;
        while (current != s) {
            auto parent = scopes[current].parent;
            bool inline_namespace = false;
            if (scopes[current].name) for (auto edge = scopes[parent].first_inline; edge; edge = edges[edge].inline_next)
                inline_namespace |= edges[edge].target == current;
            if (!inline_namespace) throw std::runtime_error("unqualified instantiation outside template namespace");
            current = parent;
        }
    }
    if ((ast[ast[source].first].op == KW_UNION) != (entities[e].key == KW_UNION))
        throw std::runtime_error("explicit instantiation class key mismatch");
    if (ast[n].flags & 1) return;
    std::vector<EntityId> work(1,e); Index seen;
    for (std::size_t i = 0; i < work.size(); ++i) {
        auto cls = work[i];
        if (seen.get(cls)) continue;
        seen.put(cls,1); complete_class(cls);
        if (!entities[cls].complete) throw std::runtime_error("explicit instantiation of incomplete class");
        for (auto d = scopes[entities[cls].scope].first_decl; d; d = declarations[d].next) {
            auto member = declarations[d].entity;
            if (entities[member].owner != entities[cls].scope) continue;
            bool defined = instantiate_member_definition(member);
            if (entities[member].member_info) {
                auto m = entities[member].member_info;
                if (!members[m].body && !members[m].synthetic) continue;
                members[m].retained_root = true;
                if (members[m].constructor || members[m].destructor) members[m].complete_entry = true;
                demand_member(member);
            } else if (entities[member].class_info) {
                if (entities[member].complete || defined) work.push_back(member);
            } else if (entities[member].is_static && defined) demand_template_storage(member);
        }
    }
}
} }
