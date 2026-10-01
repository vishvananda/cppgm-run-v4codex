#include "semantic/analyzer.h"
#include "semantic/definition_access.h"
#include "support/type_traits.h"
#include <stdexcept>
namespace cppgm { namespace semantic {
bool Analyzer::legacy_type_property(unsigned operation, TypeId t)
{
    auto trait = BuiltinTrait(operation);
    bool assignment = trait == BuiltinTrait::TrivialAssign || trait == BuiltinTrait::NothrowAssign;
    bool constructor = trait == BuiltinTrait::TrivialConstructor || trait == BuiltinTrait::NothrowConstructor;
    bool nothrow = trait >= BuiltinTrait::NothrowConstructor;
    while (types[t].kind == TypeKind::Array) t = types[t].child;
    auto type = types[t];
    bool ref = type.kind == TypeKind::LRef || type.kind == TypeKind::RRef;
    if (assignment && (ref || (type.cv & 1))) return false;
    if (ref) return !constructor;
    if (type.kind == TypeKind::Function || fundamental(t,FT_VOID)) return false;
    if (!class_value(t)) return true;
    complete_class(type.entity);
    if (!entities[type.entity].complete) throw std::runtime_error("legacy trait requires a complete class");
    // The legacy traits describe members, not the validity of an expression
    // using them. Access, deletion and a nontrivial destructor do not make a
    // structurally trivial constructor/copy/assignment nontrivial.
    t = types.unqualified(t);
    auto identity = key(operation,t);
    auto state = BooleanFact(builtin_type_properties.get(identity));
    if (state == BooleanFact::True || state == BooleanFact::False) return state == BooleanFact::True;
    if (state == BooleanFact::Active || state == BooleanFact::Failure)
        throw std::runtime_error("recursive/failed legacy type property");
    builtin_type_properties.put(identity,unsigned(BooleanFact::Active));
    ++legacy_trait_work;
    DefinitionAccess access(explicit_instantiation_naming,access_override);
    try {
        bool value = false;
        auto trivial = constructor ? BuiltinTrait::TrivialConstructor : assignment ? BuiltinTrait::TrivialAssign : BuiltinTrait::TrivialCopy;
        if (nothrow && legacy_type_property(unsigned(trivial),t)) value = true;
        else {
            auto cls = type.entity;
            auto scope = entities[cls].scope;
            auto info = entities[cls].class_info;
            if (constructor) {
                auto ctor = default_constructor(t,scope,false);
                value = ctor && (nothrow ? function_nonthrowing(ctor) : legacy_member_trivial(ctor));
            } else {
                ensure_transfers(t,assignment);
                auto binding = assignment ? local(scope,operator_name(OP_ASS)) : class_facts[info].constructor;
                auto kind = assignment ? TransferKind::CopyAssignment : TransferKind::CopyConstructor;
                for (auto e : candidates(binding)) {
                    if (entities[e].owner != scope || members[entities[e].member_info].transfer != kind) continue;
                    value = nothrow ? function_nonthrowing(e) : legacy_member_trivial(e);
                    if (!value) break;
                }
            }
        }
        builtin_type_properties.put(identity,unsigned(value ? BooleanFact::True : BooleanFact::False));
        return value;
    } catch (const UnavailableSemanticFact&) {
        builtin_type_properties.put(identity,unsigned(BooleanFact::NotStarted)); throw;
    } catch (...) { builtin_type_properties.put(identity,unsigned(BooleanFact::Failure)); throw; }
}
bool Analyzer::legacy_member_trivial(EntityId e)
{
    auto state = BooleanFact(legacy_member_properties.get(e));
    if (state == BooleanFact::True || state == BooleanFact::False) return state == BooleanFact::True;
    if (state == BooleanFact::Active || state == BooleanFact::Failure)
        throw std::runtime_error("recursive/failed special-member triviality");
    legacy_member_properties.put(e,unsigned(BooleanFact::Active));
    ++legacy_member_work;
    try {
        auto member = members[entities[e].member_info];
        auto cls = scopes[entities[e].owner].entity;
        auto info = entities[cls].class_info;
        bool value = (member.synthetic || member.deleted) && !member.defaulted_late &&
            !member.inherited_constructor && !dynamic_class(cls);
        bool constructor = member.constructor && member.transfer == TransferKind::None;
        bool assignment = member.transfer == TransferKind::CopyAssignment;
        auto signature = types[entities[e].type];
        auto source = constructor ? 0 : value_type(types.parameters[signature.offset]);
        auto subobject = [&](TypeId type, bool initialized, bool mutable_field) {
            if (constructor && initialized) return false;
            while (types[type].kind == TypeKind::Array) type = types[type].child;
            if (!class_value(type)) return true;
            auto cv = types[source].cv & (mutable_field ? 2 : 3);
            // Follow the member selected by this function's actual parameter
            // cv/category. A different overload in the subobject's family
            // does not determine the enclosing function's triviality.
            auto selected = constructor ? default_constructor(type,entities[cls].scope,false) :
                select_transfer(type,types.qualify(type,cv),ValueCategory::Lvalue,assignment);
            return selected && legacy_member_trivial(selected);
        };
        for (auto b = class_facts[info].first_base; value && b; b = bases[b].next)
            value = !bases[b].virtual_base && subobject(entities[bases[b].base].type,false,false);
        auto scope = entities[cls].scope;
        for (auto d = scopes[scope].first_decl; value && d; d = declarations[d].next) {
            auto field = declarations[d].entity;
            if (!nonstatic_field(field) || entities[field].owner != scope) continue;
            value = subobject(entities[field].type,entities[field].initializer != 0,entities[field].mutable_field);
        }
        legacy_member_properties.put(e,unsigned(value ? BooleanFact::True : BooleanFact::False));
        return value;
    } catch (const UnavailableSemanticFact&) {
        legacy_member_properties.put(e,unsigned(BooleanFact::NotStarted)); throw;
    } catch (...) { legacy_member_properties.put(e,unsigned(BooleanFact::Failure)); throw; }
}
} }
