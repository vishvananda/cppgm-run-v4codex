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
    DefinitionAccess access(explicit_instantiation_naming,access_override);
    try {
        bool value = false;
        auto trivial = constructor ? BuiltinTrait::TrivialConstructor : assignment ? BuiltinTrait::TrivialAssign : BuiltinTrait::TrivialCopy;
        if (nothrow && legacy_type_property(unsigned(trivial),t)) value = true;
        else {
            auto cls = type.entity;
            auto scope = entities[cls].scope;
            auto info = entities[cls].class_info;
            std::vector<EntityId> family;
            if (constructor) {
                if (auto ctor = default_constructor(t,scope,false)) family.push_back(ctor);
            } else {
                ensure_transfers(t,assignment);
                auto binding = assignment ? local(scope,operator_name(OP_ASS)) : class_facts[info].constructor;
                auto kind = assignment ? TransferKind::CopyAssignment : TransferKind::CopyConstructor;
                for (auto e : candidates(binding))
                    if (entities[e].owner == scope && members[entities[e].member_info].transfer == kind) family.push_back(e);
            }
            value = !family.empty();
            for (auto e : family) {
                if (nothrow) value &= function_nonthrowing(e);
                else {
                    auto member = members[entities[e].member_info];
                    value &= (member.synthetic || member.deleted) && !member.defaulted_late && !member.inherited_constructor;
                }
            }
            if (value && !nothrow) {
                value = !dynamic_class(cls);
                for (auto b = class_facts[info].first_base; value && b; b = bases[b].next)
                    value = !bases[b].virtual_base && legacy_type_property(operation,entities[bases[b].base].type);
                for (auto d = scopes[scope].first_decl; value && d; d = declarations[d].next) {
                    auto field = declarations[d].entity;
                    if (!nonstatic_field(field) || entities[field].owner != scope) continue;
                    auto element = entities[field].type;
                    while (types[element].kind == TypeKind::Array) element = types[element].child;
                    if (constructor && entities[field].initializer) value = false;
                    // Deletion due to a reference/const field is separate from
                    // triviality. Only class subobjects select special members.
                    if (class_value(element)) value &= legacy_type_property(operation,types.unqualified(element));
                }
            }
        }
        builtin_type_properties.put(identity,unsigned(value ? BooleanFact::True : BooleanFact::False));
        return value;
    } catch (const UnavailableSemanticFact&) {
        builtin_type_properties.put(identity,unsigned(BooleanFact::NotStarted)); throw;
    } catch (...) { builtin_type_properties.put(identity,unsigned(BooleanFact::Failure)); throw; }
}
} }
