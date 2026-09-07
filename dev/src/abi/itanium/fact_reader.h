#pragma once
#include "abi/itanium/abi_mangle.h"

namespace abi_mangle {
using Words = std::vector<std::string>;
enum class BindingKind { None, Type, Argument, Expression, Context, Entity };
struct Binding { BindingKind kind = BindingKind::None; Id id = 0; };
class FactReader {
public:
    explicit FactReader(Graph& graph) : g(graph) {}
    void record(const Words& words);
    Target finish();
    bool has_target() const { return target_seen; }
private:
    Graph& g;
    std::vector<Binding> bindings;
    Target target;
    bool target_seen = false;
    unsigned depth = 0;
    Binding lookup(const std::string& name);
    Id reference(const std::string& name, BindingKind kind);
    void bind(const std::string& name, BindingKind kind, Id id);
    Id type(const Words& words, std::size_t& pos);
    Id compact(const std::string& word);
    Id argument(const Words& words, std::size_t& pos);
    Id expression(const Words& words, std::size_t& pos);
    Id entity(const Words& words, std::size_t& pos);
    Id context(const Words& words, std::size_t& pos);
    Function function(const Words& words, std::size_t& pos);
    void function_record(const Words& words);
    void target_record(const Words& words);
    std::vector<Id> refs(const Words& words, std::size_t& pos, BindingKind kind);
};
Words split_words(const std::string& line);
const std::string& take(const Words& words, std::size_t& pos);
void exhausted(const Words& words, std::size_t pos);
std::uint64_t index_value(const std::string& word);
std::uint64_t integral_value(const std::string& word);
bool boolean(const std::string& word);
unsigned qualifier(const std::string& word);
} // namespace abi_mangle
