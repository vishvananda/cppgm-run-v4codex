#include "semantic/analyzer.h"
#include <cstring>
#include <limits>
#include <stdexcept>

namespace cppgm { namespace semantic {
using syntax::Kind;
namespace {
std::uint64_t layout_add(std::uint64_t a, std::uint64_t b)
{
    if (b > std::numeric_limits<std::uint64_t>::max() - a)
        throw std::runtime_error("class size overflow");
    return a + b;
}
std::uint64_t layout_align(std::uint64_t bytes, std::uint64_t alignment)
{
    std::uint64_t remainder = bytes % alignment;
    return remainder ? layout_add(bytes, alignment - remainder) : bytes;
}
}
bool Analyzer::integral(TypeId id) const
{
    const Type& t = types[id];
    if (t.kind == TypeKind::LRef || t.kind == TypeKind::RRef) return integral(t.child);
    if (t.kind == TypeKind::Named) return entities[t.entity].key == KW_ENUM;
    return t.kind == TypeKind::Fundamental && t.fundamental <= FT_BOOL;
}
bool Analyzer::scoped_enum(TypeId t) const { return types[t].kind == TypeKind::Named && entities[types[t].entity].scoped; }
bool Analyzer::is_unsigned(TypeId id) const
{
    const Type& t = types[id];
    if (t.kind == TypeKind::Named) return is_unsigned(entities[t.entity].underlying);
    return (t.fundamental >= FT_UNSIGNED_CHAR && t.fundamental <= FT_UNSIGNED_LONG_LONG_INT) ||
        t.fundamental == FT_CHAR16_T || t.fundamental == FT_CHAR32_T || t.fundamental == FT_BOOL;
}
unsigned Analyzer::width(TypeId id) const
{
    const Type& t = types[id];
    if (t.kind == TypeKind::Named) return width(entities[t.entity].underlying);
    return fundamental_width(t.fundamental) * 8;
}
Constant Analyzer::convert(Constant v, TypeId to, bool explicit_cast)
{
    if (!v.valid) return v;
    if (types[to].kind == TypeKind::LRef || types[to].kind == TypeKind::RRef) to = types[to].child;
    if (!integral(to)) return Constant();
    if (!explicit_cast && (scoped_enum(v.type) || scoped_enum(to)) && types.unqualified(to) != types.unqualified(v.type))
        throw std::runtime_error("implicit scoped enum conversion");
    unsigned bits = width(to);
    if (types[to].kind == TypeKind::Fundamental && types[to].fundamental == FT_BOOL) v.bits = !!v.bits;
    else if (bits < 64) {
        std::uint64_t mask = (std::uint64_t(1) << bits) - 1;
        v.bits &= mask;
        if (!is_unsigned(to) && (v.bits & (std::uint64_t(1) << (bits - 1)))) v.bits |= ~mask;
    }
    v.type = to; return v;
}
std::uint64_t Analyzer::size(TypeId id, bool alignment)
{
    Type t = types[id];
    if (t.kind == TypeKind::LRef || t.kind == TypeKind::RRef) return size(t.child, alignment);
    if (t.kind == TypeKind::Pointer) return 8;
    if (t.kind == TypeKind::Array) {
        if (!t.bound) throw std::runtime_error("sizeof incomplete array");
        if (alignment) return size(t.child, true);
        std::uint64_t element = size(t.child);
        if (t.bound > std::numeric_limits<std::uint64_t>::max() / element)
            throw std::runtime_error("array size overflow");
        return t.bound * element;
    }
    if (t.kind == TypeKind::Fundamental && t.fundamental != FT_VOID) return fundamental_width(t.fundamental);
    if (t.kind == TypeKind::Named && entities[t.entity].key == KW_ENUM) return size(entities[t.entity].underlying, alignment);
    if (t.kind == TypeKind::Named && entities[t.entity].complete) {
        EntityId e = t.entity;
        std::uint32_t layout = entities[e].class_info;
        if (class_facts[layout].layout_state == 1) throw std::runtime_error("recursive class layout");
        if (class_facts[layout].layout_state != 2) {
            class_facts[layout].layout_state = 1;
            std::uint64_t bytes = 0, align = 1;
            for (std::uint32_t d = scopes[entities[e].scope].first_decl; d; d = declarations[d].next) {
                const Entity member = entities[declarations[d].entity];
                if (member.kind != EntityKind::Variable || member.is_static) continue;
                Type field = types[member.type];
                bool reference = field.kind == TypeKind::LRef || field.kind == TypeKind::RRef;
                std::uint64_t field_align = reference ? 8 : size(member.type, true);
                std::uint64_t field_size = reference ? 8 : size(member.type);
                align = std::max(align, field_align);
                if (entities[e].key == KW_UNION) bytes = std::max(bytes, field_size);
                else bytes = layout_add(layout_align(bytes, field_align), field_size);
            }
            class_facts[layout].alignment = align;
            class_facts[layout].size = bytes ? layout_align(bytes, align) : 1;
            class_facts[layout].layout_state = 2;
        }
        return alignment ? class_facts[layout].alignment : class_facts[layout].size;
    }
    throw std::runtime_error("sizeof unsupported or incomplete type");
}
TypeId Analyzer::expression_type(NodeId n, ScopeId s, bool decltype_form)
{
    if (ast[n].kind == Kind::Literal) return types.fundamental(ast.literals[ast[n].literal].type);
    if (ast[n].kind == Kind::KeywordLiteral && ast[n].op == KW_NULLPTR) return types.fundamental(FT_NULLPTR_T);
    if (ast[n].kind == Kind::Parenthesized) {
        TypeId t = expression_type(ast[n].first, s);
        NodeId inner = ast[n].first;
        while (ast[inner].kind == Kind::Parenthesized) inner = ast[inner].first;
        if (decltype_form && ast[inner].kind == Kind::IdExpression) {
            EntityId e = resolve(ast[inner].detail, s);
            if (e && entities[e].kind != EntityKind::Enumerator) return types.compound(TypeKind::LRef, t);
        }
        return t;
    }
    if (ast[n].kind == Kind::IdExpression) {
        EntityId e = resolve(ast[n].detail, s);
        if (!e) throw std::runtime_error("unknown decltype/sizeof name");
        facts[n].entity = e; facts[n].type = entities[e].type;
        return entities[e].type;
    }
    Constant v = evaluate(n, s);
    if (!v.valid) throw std::runtime_error("unsupported type-forming expression");
    return v.type;
}
Constant Analyzer::evaluate(NodeId n, ScopeId s)
{
    if (!n) return Constant();
    if (facts[n].value) return constants[facts[n].value];
    Constant result = evaluate_value(n, s);
    facts[n].scope = s;
    if (result.valid) {
        facts[n].value = constants.size();
        facts[n].type = result.type;
        constants.push_back(result);
    } else facts[n].value = 1; // Expected non-constant, owned by this parsed region.
    return result;
}
Constant Analyzer::evaluate_value(NodeId n, ScopeId s)
{
    ++constant_work;
    NodeId first = ast[n].first;
    switch (ast[n].kind) {
    case Kind::Initializer: case Kind::Parenthesized: case Kind::BracedInit: case Kind::ParenInitializer:
        return evaluate(first, s);
    case Kind::Literal: {
        const syntax::LiteralValue& literal = ast.literals[ast[n].literal];
        TypeId t = types.fundamental(literal.type);
        if (!integral(t) || literal.kind == LiteralKind::string) return Constant();
        std::uint64_t bits = 0;
        std::memcpy(&bits, literal.scalar.data(), fundamental_width(literal.type));
        return convert(Constant(t, bits), t);
    }
    case Kind::KeywordLiteral:
        if (ast[n].op == KW_TRUE || ast[n].op == KW_FALSE)
            return Constant(types.fundamental(FT_BOOL), ast[n].op == KW_TRUE);
        return Constant();
    case Kind::IdExpression: {
        EntityId e = resolve(ast[n].detail, s);
        if (!e) return Constant();
        facts[n].entity = e; facts[n].type = entities[e].type;
        return entities[e].constant;
    }
    case Kind::Sizeof: case Kind::TypeTrait: {
        TypeId t = ast[first].kind == Kind::TypeId ? type_id(first, s) : expression_type(first, s);
        return Constant(types.fundamental(FT_UNSIGNED_LONG_INT), size(t, ast[n].op == KW_ALIGNOF));
    }
    case Kind::Cast: return convert(evaluate(ast[first].next, s), type_id(first, s), true);
    case Kind::Conditional: {
        Constant cond = evaluate(first, s);
        if (!cond.valid || scoped_enum(cond.type)) return Constant();
        NodeId yes = ast[first].next;
        return evaluate(cond.bits ? yes : ast[yes].next, s);
    }
    case Kind::Unary: {
        Constant v = evaluate(first, s);
        if (!v.valid || scoped_enum(v.type)) return Constant();
        if (width(v.type) < 32) v = convert(v, types.fundamental(FT_INT));
        if (ast[n].op == OP_LNOT) return Constant(types.fundamental(FT_BOOL), !v.bits);
        if (ast[n].op == OP_PLUS) return v;
        if (ast[n].op == OP_COMPL) return convert(Constant(v.type, ~v.bits), v.type);
        if (ast[n].op == OP_MINUS) return binary(OP_MINUS, Constant(v.type, 0), v);
        return Constant();
    }
    case Kind::Binary: {
        Constant a = evaluate(first, s);
        if (!a.valid) return Constant();
        ETokenType op = ast[n].op;
        if ((op == OP_LAND || op == OP_LOR) && scoped_enum(a.type)) return Constant();
        if (op == OP_LAND && !a.bits) return Constant(types.fundamental(FT_BOOL), 0);
        if (op == OP_LOR && a.bits) return Constant(types.fundamental(FT_BOOL), 1);
        return binary(op, a, evaluate(ast[first].next, s));
    }
    default: return Constant();
    }
}
Constant Analyzer::binary(ETokenType op, Constant a, Constant b)
{
    if (!a.valid || !b.valid || !integral(a.type) || !integral(b.type)) return Constant();
    bool compare = op == OP_EQ || op == OP_NE || op == OP_LT || op == OP_GT || op == OP_LE || op == OP_GE;
    if (scoped_enum(a.type) || scoped_enum(b.type)) {
        if (!compare || types.unqualified(a.type) != types.unqualified(b.type)) throw std::runtime_error("invalid scoped enum operation");
    }
    TypeId at = types[a.type].kind == TypeKind::Named ? entities[types[a.type].entity].underlying : a.type;
    TypeId bt = types[b.type].kind == TypeKind::Named ? entities[types[b.type].entity].underlying : b.type;
    if (width(at) < 32) at = types.fundamental(FT_INT);
    if (width(bt) < 32) bt = types.fundamental(FT_INT);
    TypeId common = width(at) > width(bt) ? at : width(bt) > width(at) ? bt : is_unsigned(bt) ? bt : at;
    if (op == OP_LSHIFT || op == OP_RSHIFT) common = at;
    a = convert(a, common, true); b = convert(b, (op == OP_LSHIFT || op == OP_RSHIFT) ? bt : common, true);
    bool unsign = is_unsigned(common);
    __int128 x = unsign ? __int128(a.bits) : __int128(static_cast<std::int64_t>(a.bits));
    __int128 y = is_unsigned(b.type) ? __int128(b.bits) : __int128(static_cast<std::int64_t>(b.bits));
    __int128 result = 0;
    bool boolean = compare || op == OP_LAND || op == OP_LOR;
    switch (op) {
    case OP_PLUS: result = x + y; break;
    case OP_MINUS: result = x - y; break;
    case OP_STAR: {
        // Unsigned multiplication is modulo the result width, including uint64.
        if (unsign) return convert(Constant(common, a.bits * b.bits), common);
        result = x * y; break;
    }
    case OP_DIV: case OP_MOD:
        if (!y) throw std::runtime_error("division by zero in constant");
        result = op == OP_DIV ? x / y : x % y; break;
    case OP_AMP: result = a.bits & b.bits; break;
    case OP_BOR: result = a.bits | b.bits; break;
    case OP_XOR: result = a.bits ^ b.bits; break;
    case OP_EQ: result = x == y; break;
    case OP_NE: result = x != y; break;
    case OP_LT: result = x < y; break;
    case OP_GT: result = x > y; break;
    case OP_LE: result = x <= y; break;
    case OP_GE: result = x >= y; break;
    case OP_LAND: result = x && y; break;
    case OP_LOR: result = x || y; break;
    case OP_LSHIFT: case OP_RSHIFT:
        if (y < 0 || y >= width(common) || (op == OP_LSHIFT && x < 0)) throw std::runtime_error("invalid constant shift");
        result = op == OP_LSHIFT ? x * (__int128(1) << unsigned(y)) : x >> unsigned(y); break;
    default: return Constant();
    }
    if (boolean) return Constant(types.fundamental(FT_BOOL), result);
    bool arithmetic = op == OP_PLUS || op == OP_MINUS || op == OP_STAR || op == OP_DIV || op == OP_MOD;
    if (!unsign && arithmetic) {
        __int128 limit = __int128(1) << (width(common) - 1);
        if (result < -limit || result >= limit) throw std::runtime_error("signed constant overflow");
    }
    return convert(Constant(common, std::uint64_t(result)), common);
}
} }
