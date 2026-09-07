#pragma once
#include "abi/itanium/graph.h"
#include "abi/itanium/nesting.h"

namespace abi_mangle {
// One encoder owns the substitution sequence for one symbol. Flat sparse
// slots are local to that symbol; external-name literals use a new encoder.
class Encoder {
    std::string owned_output;
public:
    explicit Encoder(Graph& graph);
    std::string target(const Target& target);
    void function(const Function& function);
    std::string& output;
private:
    Encoder(Graph& graph, std::string& destination, unsigned nesting);
    Graph& g;
    struct Substitution { Id key = 0, value = 0; };
    std::vector<Substitution> substitutions;
    void grow_substitutions();
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
    bool modifier(const Node& node);
    void argument(Id id);
    void expression(Id id);
    void literal(const Node& node);
    void context(Id id);
    void local_component(Id id);
    void external(Id id);
    void args(const Node& node);
};
} // namespace abi_mangle
