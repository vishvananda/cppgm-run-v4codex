#pragma once
#include "abi/itanium/graph.h"

namespace abi_mangle {
// One encoder owns the substitution sequence for one symbol. Direct-indexed
// slots are reset only for that symbol; external-name literals use a new encoder.
class Encoder {
public:
    explicit Encoder(Graph& graph);
    std::string target(const Target& target);
    void function(const Function& function);
    std::string output;
private:
    Graph& g;
    std::vector<Id> substitutions;
    Id next = 0;
    unsigned depth = 0;
    void source(Id name);
    void parameter(std::uint64_t index);
    void qualifiers(unsigned bits);
    void tags(const std::vector<Id>& tags);
    void integer(std::uint64_t bits, bool negative);
    bool use(Id id);
    void enter(Id id);
    bool standard_namespace(Id id) const;
    bool nested(Id id) const;
    bool candidate(Id id) const;
    void prefix(Id id, bool register_self = true);
    void type(Id id);
    void argument(Id id);
    void expression(Id id);
    void literal(const Node& node);
    void context(Id id);
    void local_component(Id id);
    void external(Id id);
    void args(const Node& node);
};
} // namespace abi_mangle
