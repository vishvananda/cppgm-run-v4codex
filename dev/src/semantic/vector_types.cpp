#include "semantic/analyzer.h"
#include <stdexcept>
namespace cppgm { namespace semantic {
TypeId Analyzer::vector_type(TypeId lane, QueryId query)
{
    if (dependent_type(lane) || query_fact(query).dependent)
        return types.compound(TypeKind::DependentVector,lane,query);
    auto value = constants[query_value(query)];
    if (!value.valid || !integral(value.type) || scoped_enum(value.type) ||
        negative_constant(value) || !integer_value(value) || integer_value(value) > ~std::uint64_t(0)) return 0;
    auto bytes = std::uint64_t(integer_value(value));
    auto type = types[lane];
    if (type.kind != TypeKind::Fundamental || !arithmetic(lane) || fundamental(lane,FT_BOOL) || (type.cv & 4)) return 0;
    auto element = size(lane);
    if (bytes % element || (bytes & (bytes-1))) return 0;
    return types.qualify(types.compound(TypeKind::Vector,types.unqualified(lane),bytes),type.cv);
}
TypeId Analyzer::vector_attributes(TypeId type, NodeId owner, ScopeId scope)
{
    for (auto a = ast[owner].first; a; a = ast[a].next) {
        if (ast[a].kind != syntax::Kind::VectorAttribute) continue;
        auto operand = ast[a].first;
        if (pattern_scope(scope)) bind_template_expression(operand,scope);
        auto query = expression_query(operand,scope);
        type = vector_type(type,query);
        if (!type) {
            if (template_type_probe) return 0;
            throw std::runtime_error("vector_size requires an arithmetic lane and a power-of-two byte multiple");
        }
    }
    return type;
}
} }
