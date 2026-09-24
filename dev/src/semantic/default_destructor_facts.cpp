#include "semantic/analyzer.h"
#include <stdexcept>
namespace cppgm { namespace semantic {
namespace {
// Deletion is a property of the destructor's definition, independent of the
// access privileges of the declaration that first demands it.
struct PropertyAccess {
    bool& naming; bool saved_naming; ScopeId& scope; ScopeId saved_scope;
    PropertyAccess(bool& n, ScopeId& s) : naming(n), saved_naming(n), scope(s), saved_scope(s)
        { naming = false; scope = 0; }
    ~PropertyAccess() { naming = saved_naming; scope = saved_scope; }
};
}
void Analyzer::check_default_destruction(TypeId type, ScopeId scope, bool variant)
{
    if (!default_destruction_valid(type,scope,variant)) throw std::runtime_error("deleted defaulted destructor");
}
bool Analyzer::default_destruction_valid(TypeId type, ScopeId scope, bool variant)
{
    while (types[type].kind == TypeKind::Array) type = types[type].child;
    if (pattern_class_type(type)) { check_pattern_destruction(types[type].entity,scope); return true; }
    if (dependent_type(type) || !class_value(type)) return true;
    auto cls = types[type].entity;
    require_destructor_class(cls);
    auto info = entities[cls].class_info;
    if (class_facts[info].storage) {
        // The enclosing class owns anonymous-union variant restrictions. Its
        // artificial storage object has no separately invoked destructor.
        auto cs = entities[cls].scope;
        for (auto d = scopes[cs].first_decl; d; d = declarations[d].next) {
            auto field = declarations[d].entity;
            if (nonstatic_field(field) && entities[field].owner == cs)
                if (!default_destruction_valid(entities[field].type,scope,true)) return false;
        }
        return true;
    }
    auto e = destructor_declaration(type);
    return !members[entities[e].member_info].deleted && accessible(e,scope,entities[e].owner) &&
        default_destructor_valid(e) && (!variant || trivial_destructor(type));
}
void Analyzer::check_default_destructor(EntityId e)
{
    if (!default_destructor_valid(e)) throw FailedSemanticFact(SemanticFact::DefaultDestructorProperties,e,entities[e].source);
}
bool Analyzer::default_destructor_valid(EntityId e)
{
    auto m = entities[e].member_info;
    if (!m || !members[m].destructor || !members[m].synthetic) return true;
    auto cls = scopes[entities[e].owner].entity;
    if (class_facts[entities[cls].class_info].storage) return true;
    auto state = members[m].destructor_properties;
    if (state == FactState::Success) return true;
    if (state == FactState::Failure) return false;
    if (state == FactState::Active) throw std::runtime_error("recursive default destructor properties");
    PropertyAccess access(explicit_instantiation_naming,access_override);
    members[m].destructor_properties = FactState::Active;
    try {
        require_destructor_class(cls);
        auto cs = entities[cls].scope;
        bool valid = true;
        for (auto b = class_facts[entities[cls].class_info].first_base; b; b = bases[b].next)
            valid &= default_destruction_valid(entities[bases[b].base].type,cs);
        for (auto d = scopes[cs].first_decl; d; d = declarations[d].next) {
            auto field = declarations[d].entity;
            if (nonstatic_field(field) && entities[field].owner == cs)
                valid &= default_destruction_valid(entities[field].type,cs,entities[cls].key == KW_UNION);
        }
        members[m].destructor_properties = valid ? FactState::Success : FactState::Failure;
        return valid;
    } catch (const UnavailableSemanticFact&) {
        members[m].destructor_properties = FactState::NotStarted; throw;
    } catch (...) { members[m].destructor_properties = FactState::Failure; throw; }
}
void Analyzer::check_pattern_destruction(EntityId cls, ScopeId scope)
{
    auto cs = entities[cls].scope;
    if (auto e = template_pattern_members.get(key(cls,unsigned(PatternMemberKind::Destructor)))) {
        if (members[entities[e].member_info].deleted) throw std::runtime_error("deleted local destructor");
        check_access(e,scope,cs);
        if (!members[entities[e].member_info].synthetic) return;
    }
    auto k = key(cls,unsigned(PatternPropertyKind::Destruction));
    auto state = FactState(template_pattern_property_states.get(k));
    if (state == FactState::Success) return;
    if (state == FactState::Failure) throw FailedSemanticFact(SemanticFact::DefaultDestructorProperties,cls,entities[cls].source);
    if (state == FactState::Active) throw std::runtime_error("recursive local destructor properties");
    PropertyAccess access(explicit_instantiation_naming,access_override);
    template_pattern_property_states.put(k,unsigned(FactState::Active));
    try {
        for (auto b = template_pattern_bases.get(cls); b; b = bases[b].next)
            check_default_destruction(entities[bases[b].base].type,cs);
        for (auto d = scopes[cs].first_decl; d; d = declarations[d].next) {
            auto field = declarations[d].entity;
            if (nonstatic_field(field) && entities[field].owner == cs)
                check_default_destruction(entities[field].type,cs,entities[cls].key == KW_UNION);
        }
        template_pattern_property_states.put(k,unsigned(FactState::Success));
    } catch (...) { template_pattern_property_states.put(k,unsigned(FactState::Failure)); throw; }
}
} }
