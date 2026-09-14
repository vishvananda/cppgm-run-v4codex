#include "semantic/analyzer.h"
#include <stdexcept>

namespace cppgm { namespace semantic {
using syntax::Kind;
TypeId Analyzer::constexpr_member_type(TypeId t, NodeId specs, NodeId source, NodeId d, ScopeId owner)
{
    if (types[t].kind != TypeKind::Function || scopes[owner].kind != ScopeKind::Class ||
        spec_has(specs,KW_STATIC) || spec_has(child(source,Kind::MemberSpecifiers),KW_STATIC) ||
        !(spec_has(specs,KW_CONSTEXPR) || spec_has(child(source,Kind::MemberSpecifiers),KW_CONSTEXPR))) return t;
    auto name = decl_name(d);
    if (terminal(name) == scopes[owner].name || ast[ast[name].last].op == OP_COMPL) return t;
    if (ast[name].first != ast[name].last)
        for (auto e : candidates(local(owner,terminal(name))))
            if (entities[e].is_static && entities[e].type == types.signature(t)) return t;
    auto f = types[t];
    std::vector<TypeId> params(types.parameters.begin()+f.offset,types.parameters.begin()+f.offset+f.count);
    return types.function(f.child,params,f.variadic,f.cv | 1,f.ref);
}
bool Analyzer::literal_type(TypeId t)
{
    auto type = types[t];
    if (!t) return false;
    if (dependent_type(t)) return true; // Source declaration obligation, checked before substitution.
    if (type.kind == TypeKind::Array) return literal_type(type.child);
    if (type.kind == TypeKind::Pointer || type.kind == TypeKind::MemberPointer ||
        type.kind == TypeKind::LRef || type.kind == TypeKind::RRef) return true;
    if (type.kind == TypeKind::Fundamental) return type.fundamental != FT_VOID;
    if (type.kind != TypeKind::Named) return false;
    auto cls = type.entity;
    if (entities[cls].key == KW_ENUM) return true;
    require_destructor_class(cls);
    auto state = BooleanFact(literal_type_facts.get(cls));
    if (state == BooleanFact::True || state == BooleanFact::False) return state == BooleanFact::True;
    if (state == BooleanFact::Active) throw std::logic_error("recursive literal subobject type");
    if (state == BooleanFact::Failure) throw std::runtime_error("failed literal type fact");
    literal_type_facts.put(cls,unsigned(BooleanFact::Active));
    try {
        ++constexpr_validity_work;
        auto info = entities[cls].class_info;
        bool valid = trivial_destructor(t);
        bool constructor = class_facts[info].aggregate;
        for (auto e : candidates(class_facts[info].constructor)) {
            ++constexpr_validity_work;
            if (!transfer_member(e) && constexpr_constructor(e)) constructor = true;
        }
        if (!constructor && !class_facts[info].user_constructor)
            constructor = constexpr_constructor(default_constructor(t,entities[cls].scope,false));
        valid &= constructor;
        for (auto b = class_facts[info].first_base; b; b = bases[b].next) {
            ++constexpr_validity_work;
            valid &= literal_type(entities[bases[b].base].type);
        }
        for (auto d = scopes[entities[cls].scope].first_decl; d; d = declarations[d].next) {
            auto field = declarations[d].entity;
            if (!nonstatic_field(field) || entities[field].owner != entities[cls].scope) continue;
            ++constexpr_validity_work;
            auto field_type = entities[field].type;
            while (types[field_type].kind == TypeKind::Array) field_type = types[field_type].child;
            valid &= !(types[field_type].cv & 2) && literal_type(field_type);
        }
        literal_type_facts.put(cls,unsigned(valid ? BooleanFact::True : BooleanFact::False));
        return valid;
    } catch (const UnavailableSemanticFact&) {
        literal_type_facts.put(cls,unsigned(BooleanFact::NotStarted)); throw;
    } catch (...) { literal_type_facts.put(cls,unsigned(BooleanFact::Failure)); throw; }
}
bool Analyzer::constexpr_constructor(EntityId e)
{
    if (!e || !constructor_member(e) || members[entities[e].member_info].deleted) return false;
    auto member = members[entities[e].member_info];
    if (!member.synthetic) return entities[e].constexpr_function;
    auto state = BooleanFact(constexpr_constructor_facts.get(e));
    if (state == BooleanFact::True || state == BooleanFact::False) return state == BooleanFact::True;
    if (state == BooleanFact::Active) throw std::logic_error("recursive constexpr constructor property");
    if (state == BooleanFact::Failure) throw std::runtime_error("failed constexpr constructor property");
    auto cls = scopes[entities[e].owner].entity;
    require_destructor_class(cls);
    constexpr_constructor_facts.put(e,unsigned(BooleanFact::Active));
    // Classification selects only required subobject declarations. It must not
    // request their bodies, emission, layout or construction action plans.
    ++unevaluated_depth;
    try {
        ++constexpr_validity_work;
        bool copying = member.transfer != TransferKind::None;
        auto check = [&](TypeId type, bool initialized) {
            ++constexpr_validity_work;
            while (types[type].kind == TypeKind::Array) type = types[type].child;
            if (types[type].cv & 2) return false;
            if (copying && !class_value(type)) return true;
            if (!class_value(type)) return initialized;
            if (initialized) return true; // Expression validity belongs to the initializer/evaluator.
            auto selected = copying ? select_transfer(type,types.qualify(type,1),ValueCategory::Lvalue,false) :
                default_constructor(type,entities[cls].scope,false);
            return constexpr_constructor(selected);
        };
        bool valid = true;
        auto info = entities[cls].class_info;
        for (auto b = class_facts[info].first_base; b; b = bases[b].next)
            valid &= check(entities[bases[b].base].type,false);
        for (auto d = scopes[entities[cls].scope].first_decl; d; d = declarations[d].next) {
            auto field = declarations[d].entity;
            if (nonstatic_field(field) && entities[field].owner == entities[cls].scope) {
                if (entities[cls].key == KW_UNION && !copying && field != class_facts[info].variant_initializer) continue;
                valid &= check(entities[field].type,entities[field].initializer != 0);
            }
        }
        if (entities[cls].key == KW_UNION && !copying && !class_facts[info].variant_initializer &&
            !empty_class(entities[cls].type)) valid = false;
        constexpr_constructor_facts.put(e,unsigned(valid ? BooleanFact::True : BooleanFact::False));
        --unevaluated_depth; return valid;
    } catch (const UnavailableSemanticFact&) {
        --unevaluated_depth; constexpr_constructor_facts.put(e,unsigned(BooleanFact::NotStarted)); throw;
    } catch (...) { --unevaluated_depth; constexpr_constructor_facts.put(e,unsigned(BooleanFact::Failure)); throw; }
}
void Analyzer::check_constexpr_signature(EntityId e)
{
    if (!entities[e].constexpr_function || constexpr_signature_facts.get(e)) return;
    // PA16 checks dependent declarations once. A concrete specialization may
    // have nonliteral types and still be called at runtime ([dcl.constexpr]/6).
    if (entities[e].specialization || entities[e].template_member ||
        definition_owner(scopes[entities[e].owner].entity).specialization) return;
    auto f = types[entities[e].type];
    if (!entities[e].type) return;
    bool pattern_constructor = entities[e].template_pattern && scopes[entities[e].owner].kind == ScopeKind::Class &&
        terminal(decl_name(entities[e].source)) == scopes[entities[e].owner].name;
    bool valid = constructor_member(e) || pattern_constructor || literal_type(f.child);
    for (unsigned i = 0; i < f.count; ++i) {
        ++constexpr_validity_work;
        valid &= literal_type(types.parameters[f.offset+i]);
    }
    auto m = entities[e].member_info;
    if (m && !members[m].constructor && !entities[e].is_static) {
        valid &= !members[m].virtual_member;
        auto cls = scopes[entities[e].owner].entity;
        if (entities[cls].class_info) valid &= literal_type(entities[cls].type);
    }
    if (!valid) throw std::runtime_error("constexpr function requires literal parameter, result and member owner types");
    constexpr_signature_facts.put(e,1);
}
void Analyzer::check_constexpr_constructor(EntityId e)
{
    if (!entities[e].constexpr_function || !constructor_member(e) || entities[e].specialization || entities[e].template_member) return;
    if (definition_owner(scopes[entities[e].owner].entity).specialization) return;
    auto member = members[entities[e].member_info];
    if (member.deleted) return;
    if (member.synthetic) {
        if (!constexpr_constructor(e)) throw std::runtime_error("defaulted constexpr constructor leaves uninitialized subobjects");
        return;
    }
    if (member.actions_state != FactState::Success) return; // A declaration alone needs no definition.
    if (ast[entities[e].body].kind == Kind::FunctionTry) throw std::runtime_error("constexpr constructor function try block");
    if (member.delegated_constructor) {
        if (!constexpr_constructor(member.delegated_constructor)) throw std::runtime_error("nonconstexpr delegation");
        return;
    }
    Index initialized;
    for (unsigned i = 0; i < member.action_count; ++i) {
        auto action = subobject_actions[member.action_begin+i];
        ++constexpr_validity_work;
        if (action.field) initialized.put(action.field,1);
        if (action.constructor && !constexpr_constructor(action.constructor))
            throw std::runtime_error("nonconstexpr subobject constructor");
    }
    auto cls = scopes[entities[e].owner].entity;
    unsigned variants = 0;
    for (auto d = scopes[entities[cls].scope].first_decl; d; d = declarations[d].next) {
        auto field = declarations[d].entity;
        if (!nonstatic_field(field) || entities[field].owner != entities[cls].scope ||
            (field_fact(field).bit_field && !entities[field].name)) continue;
        ++constexpr_validity_work;
        if (initialized.get(field)) ++variants;
        else if (entities[cls].key != KW_UNION) throw std::runtime_error("constexpr constructor leaves member uninitialized");
    }
    if (entities[cls].key == KW_UNION && !empty_class(entities[cls].type) && variants != 1)
        throw std::runtime_error("constexpr union constructor needs one initialized variant");
}
void Analyzer::check_constexpr_class(EntityId cls)
{
    for (auto d = scopes[entities[cls].scope].first_decl; d; d = declarations[d].next) {
        auto e = declarations[d].entity;
        if (entities[e].kind != EntityKind::Function || entities[e].owner != entities[cls].scope || !entities[e].constexpr_function) continue;
        check_constexpr_signature(e);
        if (members[entities[e].member_info].synthetic) check_constexpr_constructor(e);
    }
}
} }
