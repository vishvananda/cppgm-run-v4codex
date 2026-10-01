#include "semantic/analyzer.h"
#include "support/type_traits.h"
namespace cppgm { namespace semantic {
bool Analyzer::reference_temporary_property(unsigned operation, TypeId target, TypeId source)
{
    auto trait = BuiltinTrait(operation);
    auto kind = types[target].kind;
    if (kind != TypeKind::LRef && kind != TypeKind::RRef) return false;
    if (fundamental(source,FT_VOID)) return false;
    auto source_kind = types[source].kind;
    if (source_kind == TypeKind::Function && (types[source].cv || types[source].ref != RefQualifier::None)) return false;
    Expression x; x.type = value_type(source);
    x.category = source_kind == TypeKind::LRef || types[x.type].kind == TypeKind::Function ? ValueCategory::Lvalue :
        source_kind == TypeKind::RRef ? ValueCategory::Xvalue : ValueCategory::Prvalue;
    if (x.category == ValueCategory::Prvalue && !class_value(x.type) && types[x.type].kind != TypeKind::Array)
        x.type = types.unqualified(x.type);
    if (class_value(x.type)) complete_class(types[x.type].entity);
    if (class_value(types[target].child)) complete_class(types[types[target].child].entity);
    auto c = trait == BuiltinTrait::ReferenceConvertsTemporary ? conversion_value(x,target) :
        direct_initialization_conversion(x,target);
    if (!valid_fixed_conversion(x,0,c,global)) return false;
    auto value = x;
    if (c.kind == Conversion::Kind::User) {
        auto returned = types[entities[c.function].type].child;
        value.type = value_type(returned);
        value.category = types[returned].kind == TypeKind::LRef ? ValueCategory::Lvalue :
            types[returned].kind == TypeKind::RRef ? ValueCategory::Xvalue : ValueCategory::Prvalue;
        c = user_conversions[c.materialization].result;
    }
    bool temporary = c.temporary || c.kind == Conversion::Kind::Construction || value.category == ValueCategory::Prvalue;
    if (temporary && class_value(value.type) && value.category == ValueCategory::Prvalue &&
        !default_destruction_valid(value.type,global)) return false;
    if (temporary && !default_destruction_valid(types[target].child,global)) return false;
    return temporary;
}
} }
