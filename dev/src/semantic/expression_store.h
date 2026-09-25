#pragma once
#include "semantic/model.h"
namespace cppgm { namespace semantic {
// Immutable semantic properties are shared by source identity. Concrete value,
// receiver, incoming-conversion and evaluation state belong to individual uses.
class ExpressionStore {
    struct Properties {
        TypeId type = 0;
        std::uint32_t conversions = 0, count = 0, arguments = 0, argument_count = 0;
        ValueCategory category = ValueCategory::Prvalue;
        ExpressionForm form = ExpressionForm::Ordinary;
        CallInputs inputs = CallInputs::Concrete;
        bool null_pointer_constant : 1;
        bool discarded_form : 1;
        Properties() : null_pointer_constant(false), discarded_form(false) {}
    };
    struct Use {
        std::uint32_t fact = 0, incoming = 0;
        EntityId entity = 0; std::uint32_t object = 0;
        unsigned char state = 0;
    };
    std::vector<std::uint32_t> use_index;
    std::vector<Use> uses = std::vector<Use>(1);
    std::vector<Properties> values = std::vector<Properties>(1);
    Index conversion_variants;
    Use& write_use(NodeId n);
public:
    Expression operator[](NodeId n) const {
        const auto& use = uses[use_index[n]]; const auto& facts = values[use.fact];
        Expression value;
        value.type = facts.type; value.conversions = facts.conversions; value.count = facts.count;
        value.arguments = facts.arguments; value.argument_count = facts.argument_count;
        value.category = facts.category; value.form = facts.form; value.inputs = facts.inputs;
        value.null_pointer_constant = facts.null_pointer_constant; value.discarded_form = facts.discarded_form;
        if (value.inputs == CallInputs::Source) { value.inputs = CallInputs::Context; value.arguments = n; }
        value.entity = use.entity; value.object_use = use.object; value.incoming = use.incoming;
        value.ready = use.state&1; value.evaluated = use.state&2; return value;
    }
    void resize(std::size_t n) { use_index.resize(n); }
    void incoming(NodeId n, std::uint32_t id);
    void evaluated(NodeId n, bool value);
    void ready(NodeId n, bool value);
    void inherit(NodeId n, NodeId source);
    void inherit_conversions(NodeId n, NodeId source, std::uint32_t conversions);
    void set(NodeId n, Expression value);
    std::uint32_t argument_slice(NodeId n) const { return values[uses[use_index[n]].fact].arguments; }
    std::size_t fact_count() const { return values.size()-1; }
    std::size_t use_count() const { return uses.size()-1; }
    std::size_t slot_count() const { return use_index.size(); }
    std::size_t inherited = 0, changed = 0, variants = 0;
};
} }
