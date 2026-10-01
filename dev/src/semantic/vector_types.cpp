#include "semantic/analyzer.h"
#include <stdexcept>
namespace cppgm { namespace semantic {
TypeId Analyzer::vector_type(TypeId lane, QueryId query, bool extended)
{
    auto dependent_lane = dependent_type(lane), dependent_width = query_fact(query).dependent;
    auto type = types[lane];
    if (!dependent_lane && (type.kind != TypeKind::Fundamental || !arithmetic(lane) ||
        (!extended && fundamental(lane,FT_BOOL)) || (type.cv & 4))) return 0;
    std::uint64_t count = 0;
    if (!dependent_width) {
        auto value = constants[query_value(query)];
        if (!value.valid || !integral(value.type) || scoped_enum(value.type) || negative_constant(value) ||
            !integer_value(value) || integer_value(value) > ~std::uint64_t(0)) return 0;
        count = std::uint64_t(integer_value(value));
        if (!extended && (count & (count-1))) return 0;
    }
    if (dependent_lane || dependent_width)
        return types.compound(extended ? TypeKind::DependentExtVector : TypeKind::DependentVector,lane,query);
    auto element = size(lane);
    if (!extended && count % element) return 0;
    // Extended vector storage rounds up to a power of two. Validate before
    // multiplication/rounding so even a layout-only query cannot overflow.
    if (extended && count > (std::uint64_t(1) << 63)/element) return 0;
    return types.qualify(types.compound(extended ? TypeKind::ExtVector : TypeKind::Vector,
        types.unqualified(lane),count),type.cv);
}
std::uint64_t Analyzer::vector_elements(TypeId id)
{
    auto type = types[id];
    return type.kind == TypeKind::ExtVector ? type.bound : type.bound/size(type.child);
}
TypeId Analyzer::vector_attributes(TypeId type, NodeId owner, ScopeId scope)
{
    for (auto a = ast[owner].first; a; a = ast[a].next) {
        if (ast[a].kind != syntax::Kind::VectorAttribute) continue;
        auto operand = ast[a].first;
        if (pattern_scope(scope)) bind_template_expression(operand,scope);
        auto query = expression_query(operand,scope);
        type = vector_type(type,query,ast[a].flags & 1);
        if (!type) {
            if (template_type_probe && !pattern_scope(scope)) return 0;
            throw std::runtime_error("invalid vector lane or width");
        }
    }
    return type;
}
} }
