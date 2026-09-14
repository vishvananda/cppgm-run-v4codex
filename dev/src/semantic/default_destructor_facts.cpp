#include "semantic/analyzer.h"
#include <stdexcept>
namespace cppgm { namespace semantic {
void Analyzer::check_default_destruction(TypeId type, ScopeId scope, bool variant)
{
    while (types[type].kind == TypeKind::Array) type = types[type].child;
    if (pattern_class_type(type)) { check_pattern_destruction(types[type].entity,scope); return; }
    if (dependent_type(type) || !class_value(type)) return;
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
                check_default_destruction(entities[field].type,scope,true);
        }
        return;
    }
    default_destructor(type,scope,false);
    if (variant && !trivial_destructor(type)) throw std::runtime_error("nontrivial variant deletes defaulted destructor");
}
void Analyzer::check_default_destructor(EntityId e)
{
    auto m = entities[e].member_info;
    if (!m || !members[m].destructor || !members[m].synthetic) return;
    auto cls = scopes[entities[e].owner].entity;
    if (class_facts[entities[cls].class_info].storage) return;
    auto state = members[m].destructor_properties;
    if (state == FactState::Success) return;
    if (state == FactState::Failure) throw FailedSemanticFact(SemanticFact::DefaultDestructorProperties,e,entities[e].source);
    if (state == FactState::Active) throw std::runtime_error("recursive default destructor properties");
    members[m].destructor_properties = FactState::Active;
    try {
        require_destructor_class(cls);
        auto cs = entities[cls].scope;
        for (auto b = class_facts[entities[cls].class_info].first_base; b; b = bases[b].next)
            check_default_destruction(entities[bases[b].base].type,cs);
        for (auto d = scopes[cs].first_decl; d; d = declarations[d].next) {
            auto field = declarations[d].entity;
            if (nonstatic_field(field) && entities[field].owner == cs)
                check_default_destruction(entities[field].type,cs,entities[cls].key == KW_UNION);
        }
        members[m].destructor_properties = FactState::Success;
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
