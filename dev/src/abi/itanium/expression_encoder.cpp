#include "abi/itanium/encoder.h"
#include "abi/itanium/operations.h"
#include <stdexcept>

namespace abi_mangle {
namespace {
const char* const operations[] = {
    "ad", "de", "ps", "ng", "co", "nt", "pl", "mi", "ml", "dv", "rm",
    "an", "or", "eo", "ls", "rs", "eq", "ne", "lt", "gt", "le", "ge",
    "aa", "oo", "cm", "pm", "pt", "ix", "sc", "dc", "cc", "rc", "dt"
};
}
Id operation(const std::string& code) {
    for (Id i = 0; i < sizeof(operations) / sizeof(*operations); ++i)
        if (code == operations[i]) return i + 1;
    throw std::runtime_error("unknown dependent ABI operation");
}
const char* operation_code(Id op) {
    if (!op || op > sizeof(operations) / sizeof(*operations))
        throw std::runtime_error("invalid dependent ABI operation");
    return operations[op - 1];
}
void Encoder::literal(const Node& n) {
    output += 'L'; type(n.a);
    std::uint64_t bits = n.value;
    bool negative = static_cast<std::int64_t>(bits) < 0;
    const Node& t = g[n.a];
    if (t.kind == Kind::Builtin) {
        switch (static_cast<AbiBuiltinTypeKind>(t.a)) {
        case ABI_BUILTIN_TYPE_BOOL: bits = bits != 0; negative = false; break;
        case ABI_BUILTIN_TYPE_UNSIGNED_CHAR: bits &= 255; negative = false; break;
        case ABI_BUILTIN_TYPE_UNSIGNED_SHORT: bits &= 65535; negative = false; break;
        case ABI_BUILTIN_TYPE_UNSIGNED_INT: bits &= 0xffffffffull; negative = false; break;
        case ABI_BUILTIN_TYPE_UNSIGNED_LONG: case ABI_BUILTIN_TYPE_UNSIGNED_LONG_LONG:
        case ABI_BUILTIN_TYPE_UINT128: negative = false; break;
        default: break;
        }
    }
    integer(bits, negative); output += 'E';
}
void Encoder::argument(Id id) {
    const Node n = g[id];
    if (++depth > 1024) throw std::runtime_error("ABI nesting limit exceeded");
    ++g.stats.emitted_nodes;
    switch (n.kind) {
    case Kind::TypeArgument: type(n.a); break;
    case Kind::Value: literal(n); break;
    case Kind::DependentValue:
        output += "Tn"; type(n.a); literal(g[n.b]); break;
    case Kind::ExpressionArgument: output += 'X'; expression(n.a); output += 'E'; break;
    case Kind::ArgumentPack:
        output += 'J';
        for (Id i = 0; i < n.count; ++i) argument(g.child(n, i));
        output += 'E'; break;
    case Kind::TemplateEntity: prefix(n.a); break;
    case Kind::MemberTemplateEntity:
        // A qualified template-template argument retains its owner's template
        // argument list. Only its template prefix and member component reuse
        // substitutions, as required by the unresolved template-name grammar.
        output += 'N';
        if (g[n.a].kind == Kind::Template) {
            const Node owner = g[n.a]; prefix(owner.a); args(owner);
        } else prefix(n.a);
        if (!use(id)) { source(n.b); enter(id); }
        output += 'E'; break;
    case Kind::EntityArgument:
        if (n.b) output += "Xad";
        output += 'L'; external(n.a); output += 'E';
        if (n.b) output += 'E';
        break;
    default: throw std::runtime_error("fact is not an ABI argument");
    }
    --depth;
}
void Encoder::expression(Id id) {
    const Node n = g[id];
    if (++depth > 1024) throw std::runtime_error("ABI nesting limit exceeded");
    ++g.stats.emitted_nodes;
    switch (n.kind) {
    case Kind::ExprParameter: parameter(n.a); break;
    case Kind::ExprFunctionParameter:
        output += "fp"; if (n.a) output += std::to_string(n.a - 1); output += '_'; break;
    case Kind::Value: literal(n); break;
    case Kind::Unary: output += operation_code(n.b); expression(n.a); break;
    case Kind::Binary:
        output += operation_code(n.c); expression(n.a); expression(n.b); break;
    case Kind::Conditional:
        output += "qu"; expression(n.a); expression(n.b); expression(n.c); break;
    case Kind::ExprPack: output += "sp"; expression(n.a); break;
    case Kind::Call:
        output += "cl"; expression(n.a);
        for (Id i = 0; i < n.count; ++i) expression(g.child(n, i));
        output += 'E'; break;
    case Kind::Conversion:
        output += "cv"; type(n.a); output += '_';
        for (Id i = 0; i < n.count; ++i) expression(g.child(n, i));
        output += 'E'; break;
    case Kind::Cast: output += operation_code(n.c); type(n.a); expression(n.b); break;
    case Kind::TemplateId: source(n.a); args(n); break;
    case Kind::TypeTrait:
        output += 'u'; source(n.a);
        for (Id i = 0; i < n.count; ++i) type(g.child(n, i));
        output += 'E'; break;
    case Kind::SizeofType: output += "st"; type(n.a); break;
    case Kind::Member:
        output += "sr"; type(n.a); if (n.c) output += 'E'; source(n.b);
        if (n.count) args(n);
        break;
    case Kind::ObjectMember:
        output += operation_code(n.c); expression(n.a); source(n.b);
        if (n.count) args(n);
        break;
    case Kind::EntityExpression:
        if (n.b) output += "ad";
        output += 'L'; external(n.a); output += 'E'; break;
    default: throw std::runtime_error("fact is not an ABI expression");
    }
    --depth;
}
} // namespace abi_mangle
