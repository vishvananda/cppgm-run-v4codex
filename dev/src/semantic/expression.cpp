#include "semantic/analyzer.h"
#include <stdexcept>

namespace cppgm { namespace semantic {
using syntax::Kind;
Expression Analyzer::expression(NodeId n, ScopeId s)
{
    if (expressions[n].ready) return expressions[n];
    ++expression_work;
    facts[n].scope = s;
    Expression result = resolve_expression(n, s);
    result.ready = true;
    expressions[n] = result;
    if (!facts[n].type) facts[n].type = result.type;
    return result;
}
Expression Analyzer::resolve_expression(NodeId n, ScopeId s)
{
    NodeId first = ast[n].first;
    Expression r;
    switch (ast[n].kind) {
    case Kind::Literal: {
        const syntax::LiteralValue& lit = ast.literals[ast[n].literal];
        r.type = types.fundamental(lit.type);
        if (lit.kind == LiteralKind::string) {
            r.type = types.compound(TypeKind::Array, types.qualify(r.type, 1), lit.elements);
            r.category = ValueCategory::Lvalue;
        }
        return r;
    }
    case Kind::KeywordLiteral:
        r.type = types.fundamental(ast[n].op == KW_NULLPTR ? FT_NULLPTR_T : FT_BOOL); return r;
    case Kind::IdExpression: {
        EntityId e = resolve(ast[n].detail, s);
        if (!e) throw std::runtime_error("unknown expression name");
        r.entity = e; facts[n].entity = e;
        if (entities[e].kind == EntityKind::Overload) { r.form = ExpressionForm::Overload; r.category = ValueCategory::Lvalue; return r; }
        if (entities[e].kind != EntityKind::Variable && entities[e].kind != EntityKind::Parameter &&
            entities[e].kind != EntityKind::Enumerator && entities[e].kind != EntityKind::Function)
            throw std::runtime_error("expression requires value name");
        r.type = value_type(entities[e].type);
        if (entities[e].kind != EntityKind::Enumerator) r.category = ValueCategory::Lvalue;
        return r;
    }
    case Kind::Parenthesized: return expression(first, s);
    case Kind::Call: return call_expression(n, s);
    case Kind::Unary: case Kind::Postfix: return unary_expression(n, s);
    case Kind::Binary: case Kind::Assignment: case Kind::Conditional: return binary_expression(n, s);
    case Kind::Cast: return cast_expression(n, s, type_id(first, s), ast[first].next);
    case Kind::Sizeof: {
        TypeId t = ast[first].kind == Kind::TypeId ? type_id(first, s) : expression(first, s).type;
        r.type = types.fundamental(FT_UNSIGNED_LONG_INT);
        facts[n].value = constants.size(); constants.push_back(Constant(r.type, size(t)));
        return r;
    }
    case Kind::Subscript: {
        NodeId second = ast[first].next;
        TypeId a = decay(expression(first, s).type), b = decay(expression(second, s).type);
        if (!pointer(a)) std::swap(a, b);
        if (!pointer(a) || !integral(b) || scoped_enum(b) || fundamental(types[a].child, FT_VOID) ||
            types[types[a].child].kind == TypeKind::Function) throw std::runtime_error("invalid subscript");
        r.type = types[a].child; r.category = ValueCategory::Lvalue; return r;
    }
    case Kind::Member: {
        Expression object = expression(first, s);
        TypeId t = object.type;
        if (ast[n].op == OP_ARROW) {
            t = decay(t);
            if (!pointer(t)) throw std::runtime_error("arrow requires pointer");
            t = types[t].child;
        }
        if (types[t].kind != TypeKind::Named || !entities[types[t].entity].class_info) throw std::runtime_error("member of non-class");
        NodeId name = ast[ast[first].next].detail;
        EntityId e = resolve(name, entities[types[t].entity].scope);
        if (!e) throw std::runtime_error("unknown member");
        r.entity = e; facts[n].entity = e;
        r.type = types.qualify(value_type(entities[e].type), types[t].cv);
        r.category = ast[n].op == OP_ARROW || object.category == ValueCategory::Lvalue ? ValueCategory::Lvalue : ValueCategory::Xvalue;
        return r;
    }
    default: throw std::runtime_error("unsupported expression");
    }
}
Expression Analyzer::cast_expression(NodeId n, ScopeId s, TypeId to, NodeId operand)
{
    Expression r;
    r.type = value_type(to); r.form = ExpressionForm::Cast;
    facts[n].type = to;
    if (!operand) return r;
    Expression x = expression(operand, s);
    Type target = types[to];
    if (target.kind == TypeKind::LRef || target.kind == TypeKind::RRef) {
        unsigned added = 0;
        if (!qualification(x.type, target.child, added)) throw std::runtime_error("invalid reference cast");
        if (target.kind == TypeKind::LRef && x.category != ValueCategory::Lvalue && !(types[target.child].cv & 1))
            throw std::runtime_error("invalid lvalue cast");
        r.category = target.kind == TypeKind::LRef ? ValueCategory::Lvalue : ValueCategory::Xvalue;
        return r;
    }
    if (fundamental(to, FT_VOID)) return r;
    Conversion c = conversion(operand, to);
    if (c.valid()) { if (c.function) select_function(operand, c.function); return r; }
    TypeId from = decay(x.type);
    bool enum_cast = (integral(from) && integral(to)) || (arithmetic(from) && integral(to));
    bool pointer_cast = pointer(from) && pointer(to);
    bool reinterpret = ast[n].op == KW_REINTERPET_CAST || ast[n].op == OP_LPAREN;
    if (enum_cast || pointer_cast || (reinterpret && ((pointer(from) && integral(to)) || (integral(from) && pointer(to))))) return r;
    throw std::runtime_error("invalid explicit cast");
}
} }
