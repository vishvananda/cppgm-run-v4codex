#pragma once
#include "semantic/model.h"
namespace cppgm { namespace semantic {
// Immutable semantic facts and mutable application state have distinct owners.
// Only changed facts allocate a record; fixed template uses retain a source ID.
class ExpressionStore {
    struct Use { std::uint32_t fact = 0, incoming = 0; };
    std::vector<Use> uses;
    std::vector<unsigned char> states;
    std::vector<Expression> values = std::vector<Expression>(1);
    Index conversion_variants;
public:
    Expression operator[](NodeId n) const {
        Expression value = values[uses[n].fact];
        value.incoming = uses[n].incoming;
        value.ready = states[n]&1; value.evaluated = states[n]&2;
        return value;
    }
    void resize(std::size_t n) { uses.resize(n); states.resize(n); }
    void incoming(NodeId n, std::uint32_t id) { uses[n].incoming = id; }
    void evaluated(NodeId n, bool value) { states[n] = (states[n]&1) | (value ? 2 : 0); }
    void ready(NodeId n, bool value) { states[n] = (states[n]&2) | unsigned(value); }
    void inherit(NodeId n, NodeId source);
    void inherit_conversions(NodeId n, NodeId source, std::uint32_t conversions);
    void set(NodeId n, Expression value);
    std::size_t fact_count() const { return values.size()-1; }
    std::size_t use_count() const { return uses.size(); }
    std::size_t inherited = 0, changed = 0, variants = 0;
};
} }
