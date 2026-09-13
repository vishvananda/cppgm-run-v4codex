#include "semantic/expression_store.h"
namespace cppgm { namespace semantic {
ExpressionStore::Use& ExpressionStore::write_use(NodeId n)
{
    auto id = use_index[n];
    if (!id) { id = uses.size(); uses.push_back(Use()); use_index[n] = id; }
    return uses[id];
}
void ExpressionStore::incoming(NodeId n, std::uint32_t id) { write_use(n).incoming = id; }
void ExpressionStore::evaluated(NodeId n, bool value)
{
    auto& use = write_use(n); use.state = (use.state&1) | (value ? 2 : 0);
}
void ExpressionStore::ready(NodeId n, bool value)
{
    auto& use = write_use(n); use.state = (use.state&2) | unsigned(value);
}
void ExpressionStore::inherit(NodeId n, NodeId source)
{
    auto original = uses[use_index[source]];
    auto& use = write_use(n); use = original;
    use.incoming = 0; use.state = 0; ++inherited;
}
void ExpressionStore::set(NodeId n, Expression value)
{
    if (value.inputs == CallInputs::Context) {
        value.arguments = argument_slice(value.arguments); value.inputs = CallInputs::Source;
    }
    auto& use = write_use(n);
    const auto& prior = values[use.fact];
    bool same = value.type == prior.type && value.conversions == prior.conversions && value.count == prior.count &&
        value.arguments == prior.arguments && value.argument_count == prior.argument_count &&
        value.category == prior.category && value.form == prior.form && value.inputs == prior.inputs &&
        value.null_pointer_constant == prior.null_pointer_constant;
    use.entity = value.entity; use.object = value.object_use; use.incoming = value.incoming;
    use.state = unsigned(value.ready) | (unsigned(value.evaluated)<<1);
    if (same) return;
    Properties facts; facts.type = value.type; facts.conversions = value.conversions; facts.count = value.count;
    facts.arguments = value.arguments; facts.argument_count = value.argument_count;
    facts.category = value.category; facts.form = value.form; facts.inputs = value.inputs;
    facts.null_pointer_constant = value.null_pointer_constant;
    use.fact = values.size(); values.push_back(facts); ++changed;
}
void ExpressionStore::inherit_conversions(NodeId n, NodeId source, std::uint32_t conversions)
{
    auto base = uses[use_index[source]].fact;
    auto key = (std::uint64_t(base)<<32) | conversions;
    auto fact = conversion_variants.get(key);
    if (!fact) {
        auto value = values[base]; value.conversions = conversions;
        fact = values.size(); values.push_back(value); conversion_variants.put(key,fact); ++variants;
    }
    write_use(n).fact = fact;
}
} }
