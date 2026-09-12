#include "semantic/analyzer.h"
#include <stdexcept>
namespace cppgm { namespace semantic {
bool Analyzer::type_destructor_nonthrowing(TypeId type)
{
    while (types[type].kind == TypeKind::Array) type = types[type].child;
    if (!class_value(type)) return true;
    if (auto dtor = type_destructor(type)) return function_nonthrowing(dtor);
    return implicit_destructor_nonthrowing(types[type].entity);
}
bool Analyzer::implicit_destructor_nonthrowing(EntityId cls)
{
    auto info = entities[cls].class_info;
    auto state = class_facts[info].destructor_exception_state;
    if (state == 1) throw std::logic_error("cyclic destructor exception query");
    if (state) return state == 2;
    // Exception specifications inspect completed subobject types, independently
    // of body demand and destruction-action construction. No body is demanded.
    class_facts[info].destructor_exception_state = 1;
    bool nonthrowing = true;
    for (auto edge = class_facts[info].first_base; edge && nonthrowing; edge = bases[edge].next)
        nonthrowing = type_destructor_nonthrowing(entities[bases[edge].base].type);
    for (auto d = scopes[entities[cls].scope].first_decl; d && nonthrowing; d = declarations[d].next) {
        EntityId e = declarations[d].entity;
        if (nonstatic_field(e) && entities[e].owner == entities[cls].scope)
            nonthrowing = type_destructor_nonthrowing(entities[e].type);
    }
    class_facts[info].destructor_exception_state = nonthrowing ? 2 : 3;
    return nonthrowing;
}
} }
