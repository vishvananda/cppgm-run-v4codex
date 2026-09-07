#include "abi/itanium/encoder.h"
#include <algorithm>
#include <stdexcept>

namespace abi_mangle {
Encoder::Encoder(Graph& graph) : output(owned_output), g(graph), substitutions(16) {}
Encoder::Encoder(Graph& graph, std::string& destination, unsigned nesting)
    : output(destination), g(graph), substitutions(16), depth(nesting) {}
void Encoder::source(Id name) {
    const auto text = g.text(name);
    output += std::to_string(text.size); output.append(text.data, text.size);
}
void Encoder::parameter(std::uint64_t index) {
    output += 'T';
    if (index) output += std::to_string(index - 1);
    output += '_';
}
void Encoder::qualifiers(unsigned bits) {
    if ((bits & 12) == 12) throw std::runtime_error("conflicting ref qualifiers");
    if (bits & 2) output += 'V';
    if (bits & 1) output += 'K';
    if (bits & 4) output += 'R';
    if (bits & 8) output += 'O';
}
void Encoder::tags(const std::vector<Id>& input) {
    std::vector<Id> ordered(input);
    g.canonical_tags(ordered);
    for (Id tag : ordered) { output += 'B'; source(tag); }
}
void Encoder::integer(std::uint64_t bits, bool negative) {
    if (negative) { output += 'n'; bits = 0 - bits; }
    output += std::to_string(bits);
}
void Encoder::grow_substitutions() {
    std::vector<Substitution> old;
    old.swap(substitutions); substitutions.resize(old.size() * 2);
    for (const Substitution& entry : old) {
        if (!entry.key) continue;
        std::size_t p = (entry.key * 2654435761u) & (substitutions.size() - 1);
        while (substitutions[p].key) p = (p + 1) & (substitutions.size() - 1);
        substitutions[p] = entry;
    }
}
bool Encoder::use(Id id) {
    ++g.stats.substitution_lookups;
    std::size_t p = (id * 2654435761u) & (substitutions.size() - 1);
    while (substitutions[p].key && substitutions[p].key != id)
        p = (p + 1) & (substitutions.size() - 1);
    Id slot = substitutions[p].value;
    if (!slot) return false;
    ++g.stats.substitution_hits;
    output += 'S';
    if (slot > 1) {
        std::string digits;
        unsigned v = slot - 2;
        do { digits += "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ"[v % 36]; v /= 36; } while (v);
        output.append(digits.rbegin(), digits.rend());
    }
    output += '_';
    return true;
}
void Encoder::enter(Id id) {
    if (!candidate(id)) return;
    if ((next + 1) * 2 > substitutions.size()) grow_substitutions();
    std::size_t p = (id * 2654435761u) & (substitutions.size() - 1);
    while (substitutions[p].key && substitutions[p].key != id)
        p = (p + 1) & (substitutions.size() - 1);
    if (!substitutions[p].key) {
        substitutions[p].key = id; substitutions[p].value = ++next;
        ++g.stats.substitutions;
    }
}
bool Encoder::standard_namespace(Id id) const {
    return id && g[id].kind == Kind::Name && !g[id].a && g.spelling(g[id].b) == "std";
}
bool Encoder::candidate(Id id) const {
    const Node& n = g[id];
    return n.kind != Kind::Builtin && n.kind != Kind::Standard &&
        !(n.kind == Kind::Parameter && !n.b) && !standard_namespace(id);
}
bool Encoder::nested(Id id) const {
    // Prefix wrappers do not require recursive lookahead before the guarded
    // encoder walk. Graph construction ensures all edges point backwards.
    while (g[id].kind == Kind::Template || g[id].kind == Kind::Tagged) id = g[id].a;
    const Node& n = g[id];
    if (n.kind == Kind::Name) return n.a && !standard_namespace(n.a);
    return false;
}
void Encoder::args(const Node& n) {
    output += 'I';
    for (Id i = 0; i < n.count; ++i) argument(g.child(n, i));
    output += 'E';
}
void Encoder::prefix(Id id, bool register_self) {
    const Node n = g[id];
    if (candidate(id) && use(id)) return;
    Nesting nesting(depth);
    ++g.stats.emitted_nodes;
    switch (n.kind) {
    case Kind::Name:
        if (standard_namespace(id)) output += "St";
        else { if (n.a) prefix(n.a); source(n.b); }
        break;
    case Kind::Standard:
        output += abi_standard_substitution_code(static_cast<AbiStandardSubstitutionKind>(n.a));
        break;
    case Kind::Template: prefix(n.a); args(n); break;
    case Kind::Tagged:
        // A tag belongs to the final unqualified component, not its own scope.
        if (g[n.a].kind == Kind::Name) {
            const Node base = g[n.a];
            if (base.a) prefix(base.a);
            source(base.b);
        } else prefix(n.a, false);
        tags(g.children(id)); break;
    case Kind::Local: case Kind::Lambda:
        context(n.a); local_component(id); break;
    default: type(id); register_self = false; break;
    }
    if (register_self) enter(id);
}
bool Encoder::modifier(const Node& n) {
    switch (n.kind) {
    case Kind::Pointer: output += 'P'; break;
    case Kind::Reference: output += 'R'; break;
    case Kind::RvalueReference: output += 'O'; break;
    case Kind::Cv: qualifiers(n.b); break;
    case Kind::Pack: output += "Dp"; break;
    case Kind::Vendor: output += 'U'; source(n.b); break;
    case Kind::Array:
        output += 'A';
        if (n.b) expression(n.b); else output += std::to_string(n.value);
        output += '_'; break;
    case Kind::Vector: output += "Dv" + std::to_string(n.value) + '_'; break;
    default: return false;
    }
    return true;
}
void Encoder::type(Id id) {
    std::vector<Id> wrappers;
    for (;;) {
        if (candidate(id) && use(id)) {
            for (auto i = wrappers.rbegin(); i != wrappers.rend(); ++i) enter(*i);
            return;
        }
        const Node n = g[id];
        ++g.stats.emitted_nodes;
        if (!modifier(n)) break;
        wrappers.push_back(id); id = n.a;
    }
    const Node n = g[id];
    Nesting nesting(depth);
    switch (n.kind) {
    case Kind::Name: case Kind::Template: case Kind::Tagged: case Kind::Standard:
        if (nested(id)) output += 'N';
        prefix(id, false);
        if (nested(id)) output += 'E';
        break;
    case Kind::Builtin:
        output += abi_builtin_type_code(static_cast<AbiBuiltinTypeKind>(n.a)); break;
    case Kind::Parameter: parameter(n.value); break;
    case Kind::Transform:
        output += 'u'; source(n.b); output += 'I';
        for (Id i = 0; i < n.count; ++i) type(g.child(n, i));
        output += 'E'; break;
    case Kind::FunctionType:
        qualifiers(n.b & 3); output += 'F'; type(n.a);
        for (Id i = 0; i < n.count; ++i) type(g.child(n, i));
        if (!n.count && !n.c) output += 'v';
        if (n.c) output += 'z';
        qualifiers(n.b & 12);
        output += 'E'; break;
    case Kind::MemberPointer: output += 'M'; type(n.a); type(n.b); break;
    case Kind::Decltype: output += "DT"; expression(n.a); output += 'E'; break;
    case Kind::Local: case Kind::Lambda:
        context(n.a); local_component(id); break;
    default: throw std::runtime_error("fact is not an ABI type");
    }
    enter(id);
    for (auto i = wrappers.rbegin(); i != wrappers.rend(); ++i) enter(*i);
}
void Encoder::context(Id id) {
    if (g[id].kind == Kind::RawContext) output += g.spelling(g[id].a);
    else { output += 'Z'; function(entity_function(g, id)); output += 'E'; }
}
void Encoder::local_component(Id id) {
    const Node n = g[id];
    if (n.kind == Kind::Local) {
        source(n.b);
        if (n.value) {
            output += n.value > 10 ? "__" : "_";
            output += std::to_string(n.value - 1);
            if (n.value > 10) output += '_';
        }
    } else {
        output += "Ul";
        if (!n.count) output += 'v';
        for (Id i = 0; i < n.count; ++i) type(g.child(n, i));
        output += 'E' + std::to_string(n.value) + '_';
    }
}
std::string mangle(Graph& graph, const Target& target) {
    Encoder encoder(graph);
    return encoder.target(target);
}
} // namespace abi_mangle
