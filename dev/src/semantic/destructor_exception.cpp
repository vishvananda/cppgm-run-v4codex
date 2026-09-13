#include "semantic/analyzer.h"
#include <stdexcept>
namespace cppgm { namespace semantic {
void Analyzer::require_destructor_class(EntityId cls)
{
    if (entities[cls].complete) return;
    if (definitions) complete_class(cls);
    if (!entities[cls].complete)
        throw UnavailableSemanticFact(SemanticFact::ClassDefinition,cls,entities[cls].source);
}
bool Analyzer::type_destructor_nonthrowing(TypeId type)
{
    while (types[type].kind == TypeKind::Array) type = types[type].child;
    if (!class_value(type)) return true;
    // The member declaration, not its definition/body, supplies this edge.
    // A specialization with no materialized members is not an empty class.
    require_destructor_class(types[type].entity);
    if (auto dtor = type_destructor(type)) return function_nonthrowing(dtor);
    return implicit_destructor_nonthrowing(types[type].entity);
}
bool Analyzer::implicit_destructor_nonthrowing(EntityId cls)
{
    auto info = entities[cls].class_info;
    auto state = class_facts[info].destructor_exception_state;
    if (state == BooleanFact::True || state == BooleanFact::False) return state == BooleanFact::True;
    if (state == BooleanFact::Failure)
        throw FailedSemanticFact(SemanticFact::DestructorException,cls,entities[cls].source);
    if (state == BooleanFact::Active) throw std::logic_error("cyclic destructor exception query");
    // Exception specifications inspect completed subobject types, independently
    // of body demand and destruction-action construction. No body is demanded.
    class_facts[info].destructor_exception_state = BooleanFact::Active;
    try {
    bool nonthrowing = true;
    for (auto edge = class_facts[info].first_base; edge && nonthrowing; edge = bases[edge].next)
        nonthrowing = type_destructor_nonthrowing(entities[bases[edge].base].type);
    for (auto d = scopes[entities[cls].scope].first_decl; d && nonthrowing; d = declarations[d].next) {
        EntityId e = declarations[d].entity;
        if (nonstatic_field(e) && entities[e].owner == entities[cls].scope)
            nonthrowing = type_destructor_nonthrowing(entities[e].type);
    }
    class_facts[info].destructor_exception_state = nonthrowing ? BooleanFact::True : BooleanFact::False;
    return nonthrowing;
    } catch (const UnavailableSemanticFact&) {
        class_facts[info].destructor_exception_state = BooleanFact::NotStarted; throw;
    } catch (...) {
        class_facts[info].destructor_exception_state = BooleanFact::Failure; throw;
    }
}
} }
