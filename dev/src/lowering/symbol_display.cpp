#include "lowering/procedural.h"
namespace cppgm { namespace lowering {
std::string Procedural::display_scope(semantic::ScopeId id)
{
    if (!id || id == sem.global) return {};
    const auto& s = sem.scopes[id];
    if (s.kind == semantic::ScopeKind::Class) {
        const auto& closure = sem.closure(s.entity);
        if (closure.function) {
            const auto& range = static_cast<const syntax::Ast&>(ast).lambda_regions[ast[closure.source].literal];
            auto enclosing = closure.enclosing ? spelling(sem.entities[closure.enclosing].name) : "global";
            return "__lambda_"+enclosing+"_t"+std::to_string(range.begin)+"_"+std::to_string(range.end);
        }
    }
    auto parent = display_scope(s.parent);
    if (!s.name || s.kind == semantic::ScopeKind::Template) return parent;
    if (!parent.empty()) parent += "__";
    return parent+spelling(s.name);
}
std::string Procedural::display_symbol(EntityId id, bool base)
{
    const auto& e = sem.entities[id];
    // The earlier source-view tool has a fixed unqualified naming contract.
    // The object-capable LowIR view publishes scoped declaration spellings.
    auto result = e.c_linkage || linkage.presentation ? std::string() : display_scope(e.owner);
    if (!result.empty()) result += "__";
    result = "@"+result+spelling(e.name);
    if (!linkage.presentation && e.kind == semantic::EntityKind::Function && display_ordinals[id] > 1 && !e.c_linkage)
        result += "__ov"+std::to_string(display_ordinals[id]);
    if (base && !linkage.presentation) result += "__base_entry";
    for (char& c : result)
        if (c != '@' && c != '_' && !(c >= 'a' && c <= 'z') &&
            !(c >= 'A' && c <= 'Z') && !(c >= '0' && c <= '9')) c = '_';
    return result;
}
} }
