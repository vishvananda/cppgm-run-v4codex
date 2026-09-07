#pragma once

// PA9's canonical ABI graph. Nodes and edges live in contiguous, geometrically
// growing pools; references are stable IDs, never owning child pointers.
#include "preprocess/identifier_table.h"
#include "abi/itanium/abi_mangle_terminal.h"
#include "abi/itanium/abi_mangle_type_vocabulary.h"
#include <cstdint>
#include <string>
#include <vector>

namespace abi_mangle {
using Id = std::uint32_t;
enum class Kind : std::uint8_t {
    Name, Standard, Template, Tagged, Builtin, Parameter, Pointer, Reference,
    RvalueReference, Cv, Pack, Vendor, Array, Vector, Transform, FunctionType,
    MemberPointer, Decltype, Local, Lambda, RawContext,
    TypeArgument, Value, DependentValue, ExpressionArgument, ArgumentPack,
    TemplateEntity, MemberTemplateEntity, EntityArgument,
    ExprParameter, ExprFunctionParameter, Unary, Binary, Conditional, Call,
    Conversion, Cast, TemplateId, TypeTrait, SizeofType, Member, ObjectMember,
    ExprPack, EntityExpression, FunctionEntity, VariableEntity, SymbolEntity
};
struct Node {
    Kind kind = Kind::Name;
    Id a = 0, b = 0, c = 0; // kind-specific child IDs, string IDs, or enum values
    std::uint64_t value = 0;
    Id begin = 0, count = 0;
};
struct GraphStats {
    std::uint64_t requests = 0, hits = 0, probes = 0;
    std::uint64_t substitution_lookups = 0, substitution_hits = 0;
    std::uint64_t substitutions = 0, emitted_nodes = 0;
};
class Graph {
public:
    Graph();
    Id string(const std::string& text);
    std::string spelling(Id name) const;
    Id make(Kind kind, Id a = 0, Id b = 0, Id c = 0,
            std::uint64_t value = 0, const std::vector<Id>& children = {});
    Id name(Id parent, const std::string& source);
    Id path(const std::string& qualified);
    Id builtin(AbiBuiltinTypeKind kind);
    Id cv(Id type, unsigned qualifiers);
    const Node& operator[](Id id) const;
    Id child(const Node& node, std::size_t i) const;
    std::vector<Id> children(Id id) const;
    std::size_t size() const { return nodes_.size(); }
    std::size_t storage_bytes() const;
    GraphStats stats;
private:
    cppgm::IdentifierTable strings_;
    std::vector<Node> nodes_;
    std::vector<Id> edges_, slots_;
    std::vector<std::uint64_t> hashes_;
    void grow();
};

// These are per-symbol construction records, not a second semantic tree.
// CV bits are const=1, volatile=2; ref bits lvalue=4, rvalue=8.
enum class FunctionCategory : std::uint8_t { Inferred, Member, Nonmember };
struct Function {
    // Production supplies Member/Nonmember from the resolved declaration.
    // Inferred preserves the compact fact adapter's owner-shape convention.
    FunctionCategory category = FunctionCategory::Inferred;
    Id name = 0;
    Id context = 0, local_owner = 0;
    AbiTerminalKind terminal = ABI_TERMINAL_NONE;
    Id conversion = 0, literal_suffix = 0;
    Id result = 0;
    unsigned qualifiers = 0;
    bool variadic = false, template_prefix = false, c_linkage = false;
    std::vector<Id> arguments, parameters, tags;
};
enum class TargetKind : std::uint8_t {
    Type, Function, Variable, Typeinfo, TypeinfoName, Vtable, Vtt,
    ConstructionVtable, TlsWrapper, Thunk, VirtualThunk
};
struct Target {
    TargetKind kind = TargetKind::Type;
    Id type = 0, base = 0;
    Function function;
    std::int64_t this_adjust = 0, result_adjust = 0, vcall_offset = 0;
    std::int64_t result_vcall_offset = 0;
    bool has_result_adjust = false, virtual_result = false, internal = false;
};
// Store a complete function's immutable shape as one canonical graph node.
Id function_entity(Graph& graph, const Function& function);
Function entity_function(const Graph& graph, Id entity);
std::string mangle(Graph& graph, const Target& target);
} // namespace abi_mangle
