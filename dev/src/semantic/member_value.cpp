#include "semantic/analyzer.h"
#include <stdexcept>
namespace cppgm { namespace semantic {
Expression Analyzer::member_value(EntityId e, unsigned object_cv, ValueCategory category)
{
    auto kind = entities[e].kind;
    if (kind != EntityKind::Variable && kind != EntityKind::Enumerator && !function_binding(e))
        throw std::runtime_error("member expression requires a value");
    Expression result; result.entity = e;
    auto declared = entities[e].type;
    result.type = value_type(declared);
    bool reference = types[declared].kind == TypeKind::LRef || types[declared].kind == TypeKind::RRef;
    if (nonstatic_field(e) && !reference)
        result.type = types.qualify(result.type,object_cv & (entities[e].mutable_field ? 2 : 3));
    if (kind == EntityKind::Enumerator) result.category = ValueCategory::Prvalue;
    else if (reference || entities[e].is_static) result.category = ValueCategory::Lvalue;
    else if (function_binding(e)) result.category = ValueCategory::Prvalue;
    else result.category = category == ValueCategory::Lvalue ? ValueCategory::Lvalue : ValueCategory::Xvalue;
    if (kind == EntityKind::Overload) result.form = ExpressionForm::Overload;
    return result;
}
} }
