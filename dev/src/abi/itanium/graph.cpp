#include "abi/itanium/graph.h"
#include <algorithm>
#include <limits>
#include <stdexcept>

namespace abi_mangle {
namespace {
void mix(std::uint64_t& h, std::uint64_t v) {
    h ^= v + 0x9e3779b97f4a7c15ull + (h << 6) + (h >> 2);
}
}
Graph::Graph() : nodes_(1), slots_(16, 0), hashes_(1) {}
Id Graph::string(const std::string& text) {
    return strings_.intern(cppgm::TextView(text.data(), text.size()));
}
std::string Graph::spelling(Id id) const {
    if (!id) return {};
    cppgm::TextView v = strings_.spelling(id);
    return std::string(v.data, v.size);
}
void Graph::grow() {
    slots_.assign(slots_.size() * 2, 0);
    for (Id i = 1; i < nodes_.size(); ++i) {
        std::size_t p = hashes_[i] & (slots_.size() - 1);
        while (slots_[p]) p = (p + 1) & (slots_.size() - 1);
        slots_[p] = i;
    }
}
Id Graph::make(Kind kind, Id a, Id b, Id c, std::uint64_t value,
               const std::vector<Id>& children) {
    // Tags are part of the unqualified template name, before its arguments.
    // Canonicalize them here so direct producers and text adapters agree.
    if (kind == Kind::Tagged) {
        const Node base = (*this)[a];
        if (base.kind == Kind::Template) {
            std::vector<Id> arguments = this->children(a);
            Id tagged_prefix = make(Kind::Tagged, base.a, 0, 0, 0, children);
            return make(Kind::Template, tagged_prefix, base.b, base.c, base.value, arguments);
        }
        std::vector<Id> tags(children);
        if (base.kind == Kind::Tagged) {
            auto inherited = this->children(a);
            tags.insert(tags.end(), inherited.begin(), inherited.end()); a = base.a;
        }
        std::sort(tags.begin(), tags.end(), [this](Id x, Id y) { return spelling(x) < spelling(y); });
        tags.erase(std::unique(tags.begin(), tags.end()), tags.end());
        if (tags != children || base.kind == Kind::Tagged)
            return make(Kind::Tagged, a, b, c, value, tags);
    }
    ++stats.requests;
    std::uint64_t h = 0xcbf29ce484222325ull;
    mix(h, static_cast<unsigned>(kind)); mix(h, a); mix(h, b); mix(h, c);
    mix(h, value); mix(h, children.size());
    for (Id child : children) mix(h, child);
    std::size_t p = h & (slots_.size() - 1);
    while (slots_[p]) {
        ++stats.probes;
        Id id = slots_[p];
        const Node& n = nodes_[id];
        if (hashes_[id] == h && n.kind == kind && n.a == a && n.b == b &&
            n.c == c && n.value == value && n.count == children.size() &&
            std::equal(children.begin(), children.end(), edges_.begin() + n.begin)) {
            ++stats.hits;
            return id;
        }
        p = (p + 1) & (slots_.size() - 1);
    }
    if (nodes_.size() >= std::numeric_limits<Id>::max() ||
        children.size() > std::numeric_limits<Id>::max() - edges_.size())
        throw std::runtime_error("ABI graph capacity exceeded");
    if (nodes_.size() * 2 > slots_.size()) {
        grow(); p = h & (slots_.size() - 1);
        while (slots_[p]) p = (p + 1) & (slots_.size() - 1);
    }
    Node n; n.kind = kind; n.a = a; n.b = b; n.c = c; n.value = value;
    n.begin = edges_.size(); n.count = children.size();
    Id id = nodes_.size();
    edges_.insert(edges_.end(), children.begin(), children.end());
    nodes_.push_back(n); hashes_.push_back(h); slots_[p] = id;
    return id;
}
Id Graph::name(Id parent, const std::string& source) {
    return make(Kind::Name, parent, string(source));
}
Id Graph::path(const std::string& qualified) {
    if (qualified.size() >= 2 && qualified.compare(qualified.size() - 2, 2, "::") == 0)
        throw std::runtime_error("missing terminal ABI source name");
    Id parent = 0;
    std::size_t p = qualified.compare(0, 2, "::") == 0 ? 2 : 0;
    while (p < qualified.size()) {
        std::size_t end = qualified.find("::", p);
        if (end == std::string::npos) end = qualified.size();
        if (end == p) throw std::runtime_error("empty ABI name component");
        parent = name(parent, qualified.substr(p, end - p));
        p = end + 2;
    }
    if (!parent) throw std::runtime_error("empty ABI name");
    return parent;
}
Id Graph::builtin(AbiBuiltinTypeKind kind) {
    return make(Kind::Builtin, static_cast<Id>(kind));
}
Id Graph::cv(Id type, unsigned qualifiers) {
    if (!qualifiers) return type;
    const Node n = (*this)[type];
    if (n.kind == Kind::Cv) { qualifiers |= n.b; type = n.a; }
    return make(Kind::Cv, type, qualifiers);
}
const Node& Graph::operator[](Id id) const {
    if (!id || id >= nodes_.size()) throw std::runtime_error("invalid ABI graph ID");
    return nodes_[id];
}
Id Graph::child(const Node& node, std::size_t i) const {
    if (i >= node.count) throw std::runtime_error("invalid ABI child index");
    return edges_[node.begin + i];
}
std::vector<Id> Graph::children(Id id) const {
    const Node& n = (*this)[id];
    return std::vector<Id>(edges_.begin() + n.begin, edges_.begin() + n.begin + n.count);
}
std::size_t Graph::storage_bytes() const {
    return strings_.storage_bytes() + nodes_.capacity() * sizeof(Node) +
        (edges_.capacity() + slots_.capacity()) * sizeof(Id) +
        hashes_.capacity() * sizeof(std::uint64_t);
}
Id function_entity(Graph& g, const Function& f) {
    // Fixed metadata followed by three length-delimited child sequences.
    std::vector<Id> data = {f.context, f.local_owner, static_cast<Id>(f.terminal),
        f.conversion, f.literal_suffix, f.result,
        static_cast<Id>(f.arguments.size()), static_cast<Id>(f.parameters.size())};
    data.insert(data.end(), f.arguments.begin(), f.arguments.end());
    data.insert(data.end(), f.parameters.begin(), f.parameters.end());
    data.insert(data.end(), f.tags.begin(), f.tags.end());
    return g.make(Kind::FunctionEntity, f.name, f.qualifiers,
        f.variadic | (f.template_prefix << 1) | (f.c_linkage << 2) | (static_cast<Id>(f.category) << 3), 0, data);
}
Function entity_function(const Graph& g, Id entity) {
    const Node& n = g[entity];
    if (n.kind != Kind::FunctionEntity || n.count < 8)
        throw std::runtime_error("expected ABI function context");
    Function f; f.name = n.a; f.qualifiers = n.b;
    f.variadic = n.c & 1; f.template_prefix = n.c & 2; f.c_linkage = n.c & 4;
    f.category = static_cast<FunctionCategory>(n.c >> 3);
    f.context = g.child(n, 0); f.local_owner = g.child(n, 1);
    f.terminal = static_cast<AbiTerminalKind>(g.child(n, 2));
    f.conversion = g.child(n, 3); f.literal_suffix = g.child(n, 4);
    f.result = g.child(n, 5);
    Id args = g.child(n, 6), params = g.child(n, 7);
    for (Id i = 8; i < n.count; ++i) {
        if (i < 8 + args) f.arguments.push_back(g.child(n, i));
        else if (i < 8 + args + params) f.parameters.push_back(g.child(n, i));
        else f.tags.push_back(g.child(n, i));
    }
    return f;
}
} // namespace abi_mangle
