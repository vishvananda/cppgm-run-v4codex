#include "semantic/analyzer.h"
namespace cppgm { namespace semantic {
TypeId Analyzer::bit_integer_type(QueryId query, bool unsign)
{
    if (query_fact(query).dependent)
        return types.compound(TypeKind::DependentBitInt,types.fundamental(unsign ? FT_UNSIGNED_INT : FT_INT),query);
    auto value = constants[query_value(query)];
    if (!value.valid || !integral(value.type) || scoped_enum(value.type) || negative_constant(value)) return 0;
    auto width = integer_value(value);
    // The shared constant and native scalar engines retain at most 128 bits.
    // Diagnose unsupported precision, never silently widen or discard bits.
    if (width < (unsign ? 1u : 2u) || width > 128) return 0;
    return types.bit_integer(unsigned(width),unsign);
}
} }
