#include "semantic/expression_store.h"
namespace cppgm { namespace semantic {
void ExpressionStore::inherit(NodeId n, NodeId source)
{
    uses[n].fact = uses[source].fact;
    uses[n].incoming = 0; states[n] = 0; ++inherited;
}
void ExpressionStore::set(NodeId n, Expression value)
{
    auto& use = uses[n];
    const auto& prior = values[use.fact];
    bool same = value.object_use == prior.object_use && value.type == prior.type && value.entity == prior.entity &&
        value.conversions == prior.conversions && value.count == prior.count &&
        value.arguments == prior.arguments && value.argument_count == prior.argument_count &&
        value.category == prior.category && value.form == prior.form && value.null_pointer_constant == prior.null_pointer_constant;
    use.incoming = value.incoming; states[n] = unsigned(value.ready) | (unsigned(value.evaluated)<<1);
    if (same) return;
    value.incoming = 0; value.ready = false; value.evaluated = false;
    use.fact = values.size(); values.push_back(value); ++changed;
}
void ExpressionStore::inherit_conversions(NodeId n, NodeId source, std::uint32_t conversions)
{
    auto base = uses[source].fact;
    auto key = (std::uint64_t(base)<<32) | conversions;
    auto fact = conversion_variants.get(key);
    if (!fact) {
        auto value = values[base]; value.conversions = conversions;
        fact = values.size(); values.push_back(value); conversion_variants.put(key,fact); ++variants;
    }
    uses[n].fact = fact;
}
} }
