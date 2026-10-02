#include "semantic/analyzer.h"
#include <stdexcept>
namespace cppgm { namespace semantic {
using syntax::Kind;
void Analyzer::finish_object_initializer(EntityId e, NodeId init, NodeId d, NodeId specs, ScopeId s,
    ScopeId definition_scope, TypeId t, bool external)
{
    auto owner = entities[e].owner;
    bool function = entities[e].kind == EntityKind::Function, alias = entities[e].kind == EntityKind::Alias;
    auto canonical = entities[e].type;
    bool member_initializer = calls && init && !function && scopes[s].kind == ScopeKind::Class && !entities[e].is_static;
    if (calls && init && !function && scopes[s].kind == ScopeKind::Class && entities[e].is_static &&
        !entities[e].inline_variable && !spec_has(specs, KW_CONSTEXPR) && (!(types[t].cv & 1) || !integral(t)))
        throw std::runtime_error("in-class static initializer requires const integral or constexpr member");
    if (calls && !function && !entities[e].is_static && scopes[owner].kind == ScopeKind::Class && entities[e].access != Access::Public)
        class_facts[entities[scopes[owner].entity].class_info].aggregate = false;
    if (member_initializer) {
        auto cls = scopes[s].entity;
        auto info = entities[cls].class_info;
        class_facts[info].aggregate = false;
        class_facts[info].has_member_initializer = true;
        if (entities[cls].key == KW_UNION) {
            if (class_facts[info].variant_initializer) throw std::runtime_error("multiple default union variant initializers");
            class_facts[info].variant_initializer = e;
        }
    }
    if (calls && init && !function && !member_initializer) initialize(init, canonical, definition_scope);
    if (calls && init && !function && !alias && !member_initializer) {
        retain_initializer_references(e);
        prepare_context_reference(e);
    }
    if (calls && !function && types[canonical].kind == TypeKind::Array &&
        (spec_has(specs,KW_CONSTEXPR) || (init && !member_initializer && !entities[e].is_static &&
            !entities[e].external_decl && scopes[owner].kind != ScopeKind::Namespace && scopes[owner].kind != ScopeKind::Class)))
        prepare_constant_array(e,spec_has(specs,KW_CONSTEXPR));
    if (calls && !entities[e].initializer && !function && !alias && (scopes[s].kind != ScopeKind::Class || entities[e].inline_variable) && !external) default_initialize(e,d);
    if (init && !alias && !function && (!calls || (types[t].kind != TypeKind::LRef && types[t].kind != TypeKind::RRef)) && (integral(t) || floating_type(value_type(t))) && !member_initializer) {
        EvaluationScope mode(*this,!calls || spec_has(specs,KW_CONSTEXPR) ||
            (types[t].cv == 1 && integral(t)) || entities[e].is_static || scopes[owner].kind == ScopeKind::Namespace);
        auto mode_uses = evaluation_mode_uses;
        Constant v = calls ? convert(constant_initialize(init,t,definition_scope),t) : convert(evaluate(init, definition_scope),t);
        if (calls && spec_has(specs, KW_CONSTEXPR) && !v.valid) throw std::runtime_error("nonconstant constexpr initializer");
        if (v.valid) {
            // A reference may preserve an address constant while reads through
            // its volatile-qualified referent are never constant values.
            if (!(types[v.type].cv & 2) && (!floating_type(v.type) || spec_has(specs,KW_CONSTEXPR)) &&
                (types[t].cv == 1 || types[t].kind == TypeKind::LRef || types[t].kind == TypeKind::RRef))
            {
                entities[e].constant = v;
                if (evaluation_mode_uses != mode_uses) mode_sensitive_objects.put(e,1);
            }
        }
    }
    if (calls && !function && spec_has(specs,KW_CONSTEXPR) &&
        (!(integral(t) || floating_type(t)) || types[t].kind == TypeKind::LRef || types[t].kind == TypeKind::RRef) && types[t].kind != TypeKind::Array)
        check_constant_object(e);
    if (calls && !entities[e].initializer && !function && (integral(t) || floating_type(value_type(t))) && spec_has(specs, KW_CONSTEXPR))
        throw std::runtime_error("constexpr object requires initializer");
    if (calls && init && spec_has(specs, KW_CONSTEXPR) && integral(t) && ast[ast[init].first].kind == Kind::Literal)
        facts.edit(ast[init].first).type = t;
    if (calls && !function && !alias && !member_initializer && (scopes[s].kind != ScopeKind::Class || entities[e].inline_variable) && !external) register_destruction(e);
    if (definitions && !function && !alias && entities[e].definition && scopes[s].kind == ScopeKind::Namespace)
        demand_class_constant_storage(t);
    if (definitions && !unevaluated_depth && local_static(e))
        local_static_relocations.push_back({e,definition_scope});
}
} }
