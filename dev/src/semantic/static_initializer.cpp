#include "semantic/analyzer.h"
#include <cstring>
namespace cppgm { namespace semantic {
using syntax::Kind;
StaticValue Analyzer::static_value(NodeId n, TypeId target)
{
    ++static_requests;
    std::uint64_t key = (std::uint64_t(n) << 32) | target;
    unsigned index = static_index.get(key);
    if (index) {
        ++static_hits;
        return static_facts[index-1].state == FactState::Success ? static_facts[index-1].value : StaticValue();
    }
    index = static_facts.size()+1;
    static_index.put(key, index);
    StaticFact fact; fact.state = FactState::Active; static_facts.push_back(fact);
    StaticValue result = static_value_impl(n, target);
    // Apply the requested conversion on every path, including wrappers,
    // conditionals and addresses. A floating-to-bool conversion tests zero
    // directly; truncating to an integer first changes fractional values.
    TypeId scalar = target;
    if (types[scalar].kind == TypeKind::LRef || types[scalar].kind == TypeKind::RRef) scalar = types[scalar].child;
    if (fundamental(scalar, FT_BOOL) &&
        (result.kind == StaticValue::Floating || (scalar == target &&
        (result.kind == StaticValue::Address || result.kind == StaticValue::String)))) {
        result.bits = result.kind == StaticValue::Floating ? result.floating != 0 : 1;
        result.kind = StaticValue::Integer;
    } else if (result.kind == StaticValue::Floating && integral(scalar)) {
        result.bits = is_unsigned(scalar) ? std::uint64_t(result.floating) : std::uint64_t(std::int64_t(result.floating));
        result.kind = StaticValue::Integer;
    }
    if (result.kind == StaticValue::Integer && integral(scalar)) {
        Constant c = convert(Constant(types.fundamental(FT_UNSIGNED_LONG_LONG_INT), result.bits), scalar, true);
        result.bits = c.bits;
    }
    static_facts[index-1].value = result;
    static_facts[index-1].state = result.kind == StaticValue::Invalid ? FactState::Failure : FactState::Success;
    return result;
}
StaticValue Analyzer::static_value_impl(NodeId n, TypeId target)
{
    StaticValue r;
    if (!n) { r.kind = StaticValue::Integer; return r; }
    NodeId first = ast[n].first;
    auto kind = ast[n].kind;
    if (kind == Kind::Initializer || kind == Kind::Parenthesized || kind == Kind::ParenInitializer || kind == Kind::BracedInit)
        return static_value(first, target);
    Expression x = expressions[n];
    if (kind == Kind::Literal && ast.literals[ast[n].literal].kind == LiteralKind::string) {
        r.kind = StaticValue::String; r.string = n; return r;
    }
    if (kind == Kind::IdExpression) {
        EntityId e = x.entity;
        if (e) {
            Type t = types[entities[e].type];
            bool ref = types[target].kind == TypeKind::LRef || types[target].kind == TypeKind::RRef;
            if (t.kind == TypeKind::LRef || t.kind == TypeKind::RRef)
                return static_value(entities[e].initializer, target);
            if (ref || t.kind == TypeKind::Array || t.kind == TypeKind::Function) {
                r.kind = StaticValue::Address; r.entity = e; return r;
            }
        }
    }
    if (kind == Kind::Unary && ast[n].op == OP_AMP) return static_value(first, types.compound(TypeKind::LRef, expressions[first].type));
    if (kind == Kind::Unary && ast[n].op == OP_STAR)
        return static_value(first, expressions[first].type);
    if (kind == Kind::Unary && (ast[n].op == OP_PLUS || ast[n].op == OP_MINUS) &&
        types[x.type].kind == TypeKind::Fundamental && types[x.type].fundamental >= FT_FLOAT && types[x.type].fundamental <= FT_LONG_DOUBLE) {
        r = static_value(first, x.type);
        if (r.kind == StaticValue::Floating && ast[n].op == OP_MINUS) r.floating = -r.floating;
        return r;
    }
    if (kind == Kind::Subscript) {
        NodeId base = first;
        NodeId index = ast[first].next;
        if (!pointer(decay(expressions[base].type))) std::swap(base, index);
        r = static_value(base, types.compound(TypeKind::Pointer, x.type));
        Constant c = evaluate(index, facts[index].scope);
        if (!c.valid || r.kind != StaticValue::Address) return StaticValue();
        r.addend += static_cast<std::int64_t>(c.bits) * size(x.type); return r;
    }
    if (kind == Kind::Binary && (ast[n].op == OP_PLUS || ast[n].op == OP_MINUS)) {
        NodeId right = ast[first].next;
        if (ast[n].op == OP_PLUS && pointer(decay(expressions[right].type))) std::swap(first, right);
        TypeId left = decay(expressions[first].type);
        if (pointer(left)) {
            r = static_value(first, left);
            Constant c = evaluate(right, facts[right].scope);
            if (!c.valid || r.kind != StaticValue::Address) return StaticValue();
            std::int64_t delta = static_cast<std::int64_t>(c.bits) * size(types[left].child);
            r.addend += ast[n].op == OP_PLUS ? delta : -delta; return r;
        }
    }
    if (kind == Kind::Conditional) {
        Constant c = evaluate(first, facts[first].scope);
        if (!c.valid) return r;
        NodeId yes = ast[first].next;
        return static_value(c.bits ? yes : ast[yes].next, target);
    }
    if (x.form == ExpressionForm::Cast) {
        NodeId operand = kind == Kind::Cast ? ast[first].next : ast[ast[first].next].first;
        r = static_value(operand, facts[n].type);
    } else if (kind == Kind::KeywordLiteral && ast[n].op == KW_NULLPTR) r.kind = StaticValue::Integer;
    else if (kind == Kind::Literal && ast.literals[ast[n].literal].type >= FT_FLOAT) {
        auto literal = ast.literals[ast[n].literal]; r.kind = StaticValue::Floating;
        if (literal.type == FT_FLOAT) { float f; std::memcpy(&f, literal.scalar.data(), sizeof f); r.floating = f; }
        else if (literal.type == FT_DOUBLE) { double f; std::memcpy(&f, literal.scalar.data(), sizeof f); r.floating = f; }
        else std::memcpy(&r.floating, literal.scalar.data(), sizeof r.floating);
    } else {
        Constant c = evaluate(n, facts[n].scope);
        if (c.valid) { r.kind = StaticValue::Integer; r.bits = c.bits; }
    }
    return r;
}
} }
