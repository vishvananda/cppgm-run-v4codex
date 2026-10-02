#include "semantic/analyzer.h"
namespace cppgm { namespace semantic {
void Analyzer::storage_value(Expression& value)
{
    if (!value.entity || !value.type) return;
    auto raw = declared_storage_types.get(value.entity);
    if (!raw) return;
    raw = value_type(raw);
    // Member access may add the receiver's cv-qualification. Retain only the
    // storage decoration belonging to this already selected value type.
    raw = types.qualify(raw,types[value.type].cv);
    if (types.signature(raw) == value.type) value.storage_type = raw;
}
void Analyzer::storage_operation(Expression& value, ETokenType op, const Expression& a, const Expression& b)
{
    if (!value.type || (!a.storage_type && !b.storage_type)) return;
    auto raw = a.storage_type ? a.storage_type : a.type;
    auto other = b.storage_type ? b.storage_type : b.type;
    TypeId result = 0;
    if (!b.type) {
        if (op == OP_LPAREN) {
            if (address_value(raw)) raw = types[raw].child;
            if (types[raw].kind == TypeKind::Function) result = value_type(types[raw].child);
        } else if (op == OP_AMP && types[value.type].kind == TypeKind::Pointer)
            result = types.compound(TypeKind::Pointer,raw);
        else if (op == OP_STAR) {
            auto kind = types[raw].kind;
            if (kind == TypeKind::Pointer || kind == TypeKind::Array) result = types[raw].child;
        } else if (op == OP_INC || op == OP_DEC) result = raw;
    } else if (op == OP_LSQUARE) {
        if (types[raw].kind != TypeKind::Pointer && types[raw].kind != TypeKind::Array) raw = other;
        result = types[raw].child;
    } else if (op == OP_COMMA) result = other;
    else if (op == OP_ASS || op == OP_PLUS || op == OP_MINUS) {
        if (types[value.type].kind == TypeKind::Pointer) {
            if (types[raw].kind != TypeKind::Pointer && types[raw].kind != TypeKind::Array) raw = other;
            result = types[raw].kind == TypeKind::Array ? types.compound(TypeKind::Pointer,types[raw].child) : raw;
        } else if (op == OP_ASS) result = raw;
    }
    if (result && types.signature(result) == value.type) value.storage_type = result;
}
std::uint64_t Analyzer::expression_alignment(const Expression& value)
{
    if (value.entity && entities[value.entity].kind == EntityKind::Variable) return storage_alignment(value.entity);
    return size(value.storage_type ? value.storage_type : value.type,true);
}
void Analyzer::storage_expression(NodeId n, Expression& result)
{
    using syntax::Kind;
    storage_value(result);
    if (ast.kind(n) == Kind::Call && result.form == ExpressionForm::Ordinary) {
        auto callee = facts[n].entity;
        Expression function = expressions[ast.first(n)];
        if (callee) { function.type = entities[callee].type; function.storage_type = declared_storage_types.get(callee); }
        storage_operation(result,OP_LPAREN,function);
    }
    if (result.form == ExpressionForm::Ordinary && !facts[n].entity) {
        auto first = ast.first(n);
        auto kind = ast.kind(n);
        if (kind == Kind::Unary || kind == Kind::Postfix || kind == Kind::Binary || kind == Kind::Assignment || kind == Kind::Subscript)
            storage_operation(result,kind == Kind::Subscript ? OP_LSQUARE : ast.op(n),expressions[first],expressions[ast.next(first)]);
    }
}
void Analyzer::storage_query(const TypeQuery& q, const std::vector<TypeQueryFact>& children, TypeQueryFact& r)
{
    if (q.kind == QueryKind::Cast || q.kind == QueryKind::Name || q.kind == QueryKind::Parameter) {
        auto raw = types.qualify(value_type(q.type),types[r.expression.type].cv);
        if (raw != r.expression.type && types.signature(raw) == r.expression.type) r.expression.storage_type = raw;
    } else storage_value(r.expression);
    if (!r.selected && (q.kind == QueryKind::Unary || q.kind == QueryKind::Binary))
        storage_operation(r.expression,q.op,children[0].expression,children.size() > 1 ? children[1].expression : Expression());
    if (q.kind == QueryKind::Call && r.expression.form == ExpressionForm::Ordinary && !children.empty()) {
        Expression function = children[0].expression;
        if (r.selected) { function.type = entities[r.selected].type; function.storage_type = declared_storage_types.get(r.selected); }
        storage_operation(r.expression,OP_LPAREN,function);
    }
}
} }
