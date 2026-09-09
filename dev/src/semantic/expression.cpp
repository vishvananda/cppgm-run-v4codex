#include "semantic/analyzer.h"
#include <stdexcept>

namespace cppgm { namespace semantic {
using syntax::Kind;
Expression Analyzer::value_fact(const Expression& source) const
{
    Expression result;
    result.type = source.type; result.entity = source.entity;
    result.category = source.category;
    // Overload sets need target context through parentheses. Cast/builtin forms
    // describe only their original syntax node, never a surrounding comma or
    // parenthesized value (which owns no cast operand or builtin result slot).
    if (source.form == ExpressionForm::Overload) result.form = source.form;
    return result;
}
Expression Analyzer::expression(NodeId n, ScopeId s)
{
    if (expressions[n].ready) { expressions[n].evaluated |= !unevaluated_depth; return expressions[n]; }
    ++expression_work;
    facts[n].scope = s;
    Expression result = resolve_expression(n, s);
    class_result(n,result,s);
    result.ready = true; result.evaluated = !unevaluated_depth;
    expressions[n] = result;
    if (!facts[n].type) facts[n].type = result.type;
    return result;
}
Expression Analyzer::resolve_expression(NodeId n, ScopeId s)
{
    NodeId first = ast[n].first;
    Expression r;
    switch (ast[n].kind) {
    case Kind::New: return placement_new(n, s);
    case Kind::Literal: {
        const syntax::LiteralValue& lit = ast.literals[ast[n].literal];
        if (lit.suffix) return literal_call(n, s);
        r.type = types.fundamental(lit.type);
        if (lit.kind == LiteralKind::string) {
            r.type = types.compound(TypeKind::Array, types.qualify(r.type, 1), lit.elements);
            r.category = ValueCategory::Lvalue;
        }
        return r;
    }
    case Kind::KeywordLiteral:
        if (ast[n].op == KW_THIS) {
            r.type = implicit_object_type(s);
            if (!r.type) throw std::runtime_error("this outside nonstatic member");
            return r;
        }
        r.type = types.fundamental(ast[n].op == KW_NULLPTR ? FT_NULLPTR_T : FT_BOOL); return r;
    case Kind::IdExpression: {
        EntityId e = resolve(ast[n].detail, s);
        if (!e) throw std::runtime_error("unknown expression name");
        if (function_binding(e)) e = explicit_template(ast[n].detail, e, s);
        r.entity = e; facts[n].entity = e;
        if (entities[e].kind == EntityKind::Overload) {
            r.form = ExpressionForm::Overload; r.category = ValueCategory::Lvalue;
            ScopeId naming = naming_class(name_owner(ast[n].detail, s));
            if (naming) { record_object(r, 0, 0, 0); object_uses[r.object_use].naming_scope = naming; }
            return r;
        }
        if (entities[e].kind != EntityKind::Variable && entities[e].kind != EntityKind::Parameter &&
            entities[e].kind != EntityKind::Enumerator && entities[e].kind != EntityKind::Function)
            throw std::runtime_error("expression requires value name");
        r.type = value_type(entities[e].type);
        if (entities[e].constant.valid && scopes[entities[e].owner].kind != ScopeKind::Namespace &&
            scopes[entities[e].owner].kind != ScopeKind::Class) {
            ScopeId use = s;
            while (use && scopes[use].kind != ScopeKind::Function) use = scopes[use].parent;
            if (use && !encloses(use, entities[e].owner)) {
                facts[n].value = constants.size(); constants.push_back(entities[e].constant);
            }
        }
        if (nonstatic_field(e)) {
            TypeId object = implicit_object_type(s);
            if (object) {
                size(types[object].child);
                record_object(r, 0, object, base_steps(types[object].child, scopes[entities[e].owner].entity));
                if (types[entities[e].type].kind != TypeKind::LRef && types[entities[e].type].kind != TypeKind::RRef)
                    r.type = types.qualify(r.type, types[types[object].child].cv & (entities[e].mutable_field ? 2 : 3));
            } else if (!unevaluated_depth && !class_facts[entities[scopes[entities[e].owner].entity].class_info].storage) throw std::runtime_error("field requires object");
        }
        if (entities[e].kind == EntityKind::Enumerator && !entities[types[r.type].entity].complete)
            r.type = entities[e].constant.type;
        if (function_binding(e) && scopes[entities[e].owner].kind == ScopeKind::Class) {
            if (!r.object_use) record_object(r, 0, 0, 0);
            object_uses[r.object_use].naming_scope = naming_class(name_owner(ast[n].detail, s));
        }
        if (entities[e].member_info) facts[n].type = members[entities[e].member_info].call_type;
        if (entities[e].kind != EntityKind::Enumerator) r.category = ValueCategory::Lvalue;
        return r;
    }
    case Kind::BracedInit:
        if (!first) { r.type = types.fundamental(FT_INT); return r; }
        if (first != ast[n].last) throw std::runtime_error("excess scalar braces");
        return value_fact(expression(first, s));
    case Kind::Parenthesized: return value_fact(expression(first, s));
    case Kind::Call: return call_expression(n, s);
    case Kind::Unary: case Kind::Postfix: return unary_expression(n, s);
    case Kind::Binary: case Kind::Assignment: case Kind::Conditional: return binary_expression(n, s);
    case Kind::Cast: return cast_expression(n, s, type_id(first, s), ast[first].next);
    case Kind::TypeTrait: case Kind::Sizeof: {
        ++unevaluated_depth;
        TypeId t = ast[first].kind == Kind::TypeId ? type_id(first, s) : expression(first, s).type;
        --unevaluated_depth;
        if (!t) throw std::runtime_error("sizeof unresolved overload");
        r.type = types.fundamental(FT_UNSIGNED_LONG_INT);
        facts[n].value = constants.size(); constants.push_back(Constant(r.type, size(t, ast[n].op == KW_ALIGNOF)));
        return r;
    }
    case Kind::Subscript: {
        NodeId second = ast[first].next;
        TypeId a = decay(expression(first, s).type), b = decay(expression(second, s).type);
        if ((types[a].kind == TypeKind::Named || types[b].kind == TypeKind::Named) &&
            operator_expression(n, s, OP_LSQUARE, {first, second}, r)) return r;
        TypeId left = a, right = b;
        if (!pointer(a)) std::swap(a, b);
        if (!object_pointer(a) || !integral(b) || scoped_enum(b)) throw std::runtime_error("invalid subscript");
        record_conversion(r, first, conversion(first, pointer(left) ? left : promote(left)));
        record_conversion(r, second, conversion(second, pointer(right) ? right : promote(right)));
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
        NodeId name = ast[ast[first].next].detail;
        NodeId part = ast[name].last;
        bool destructor = ast[part].op == OP_COMPL;
        bool class_type = types[t].kind == TypeKind::Named && entities[types[t].entity].class_info;
        if (destructor) {
            NodeId id = ast[part].first;
            TypeId named = 0;
            if (ast[id].kind == Kind::TypeId) named = type_id(id, s);
            else {
                EntityId found = lookup(class_type ? entities[types[t].entity].scope : s, ast[id].text, Lookup::Ordinary);
                if (found && (entities[found].kind == EntityKind::Type || entities[found].kind == EntityKind::Alias)) named = entities[found].type;
            }
            if (!named || types.unqualified(named) != types.unqualified(t)) throw std::runtime_error("destructor name does not match object type");
            if (!class_type) {
                if (!(arithmetic(t) || pointer(t) || (types[t].kind == TypeKind::Named && entities[types[t].entity].underlying) || fundamental(t, FT_NULLPTR_T))) throw std::runtime_error("pseudo-destructor requires scalar");
                r.type = types.function(types.fundamental(FT_VOID), {}, false);
                r.form = ExpressionForm::PseudoDestructor; return r;
            }
        }
        if (!class_type) throw std::runtime_error("member of non-class");
        size(t); // Establish layout once at the semantic owner before recording field use.
        if (operator_token(name) == OP_ASS) ensure_transfers(t, true);
        EntityId e = destructor ? default_destructor(t, s) : lookup(name_owner(name, entities[types[t].entity].scope), terminal(name), Lookup::Ordinary, true);
        if (!e) throw std::runtime_error("unknown member");
        r.entity = e; facts[n].entity = e;
        ScopeId naming = name_owner(name, entities[types[t].entity].scope);
        if (!function_binding(e)) check_access(e, s, naming, t);
        if (nonstatic_field(e)) record_object(r, first, t, base_steps(t, scopes[entities[e].owner].entity));
        if (!r.object_use) record_object(r, 0, 0, 0);
        object_uses[r.object_use].naming_scope = naming;
        if (entities[e].kind == EntityKind::Overload) r.form = ExpressionForm::Overload;
        else {
            r.type = value_type(entities[e].type);
            if (nonstatic_field(e) && types[entities[e].type].kind != TypeKind::LRef && types[entities[e].type].kind != TypeKind::RRef)
                r.type = types.qualify(r.type, types[t].cv & (entities[e].mutable_field ? 2 : 3));
        }
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
    if (!operand) {
        if (types[to].kind == TypeKind::LRef || types[to].kind == TypeKind::RRef) throw std::runtime_error("value-initialized reference");
        return r;
    }
    Expression x = expression(operand, s);
    Type target = types[to];
    ETokenType op = ast[n].op;
    bool cstyle = op == OP_LPAREN || ast[n].kind == Kind::Call;
    bool reinterpret = op == KW_REINTERPET_CAST || cstyle;
    bool cv_cast = op == KW_CONST_CAST;
    Conversion c; c.target = to; c.rank = 2; c.kind = Conversion::Kind::Explicit;
    if (target.kind == TypeKind::LRef || target.kind == TypeKind::RRef) {
        unsigned added = 0;
        bool compatible = (cv_cast || cstyle) ? similar_type(x.type, target.child) : qualification(x.type, target.child, added);
        if (!cv_cast && op != KW_REINTERPET_CAST && !(types[x.type].cv & ~types[target.child].cv)) {
            if (derived_from(x.type, target.child)) {
                compatible = true; c.derived = true;
                if (!cstyle) check_base_access(x.type, target.child, s);
            } else if (derived_from(target.child, x.type)) {
                compatible = true; c.derived = true;
                if (!cstyle) check_base_access(target.child, x.type, s);
            }
        }
        if (op == KW_REINTERPET_CAST && x.category != ValueCategory::Prvalue &&
            !(types[x.type].cv & ~types[target.child].cv)) compatible = true;
        if (!compatible) throw std::runtime_error("invalid reference cast");
        if (target.kind == TypeKind::LRef && x.category != ValueCategory::Lvalue && !(types[target.child].cv & 1))
            throw std::runtime_error("invalid lvalue cast");
        if (cv_cast && (x.category == ValueCategory::Prvalue || types[x.type].kind == TypeKind::Function))
            throw std::runtime_error("invalid const reference cast");
        r.category = target.kind == TypeKind::LRef ? ValueCategory::Lvalue : ValueCategory::Xvalue;
        c.reference = true; record_conversion(r, operand, c);
        return r;
    }
    if (fundamental(to, FT_VOID) && !cv_cast && op != KW_REINTERPET_CAST) {
        if (x.form == ExpressionForm::Overload) throw std::runtime_error("discarded unresolved overload");
        c.kind = Conversion::Kind::Discarded; record_conversion(r, operand, c); return r;
    }
    if (!cv_cast && op != KW_REINTERPET_CAST) {
        Conversion standard = fundamental(to, FT_BOOL) ? boolean_conversion(operand) : conversion(operand, to);
        if (standard.valid()) {
            if (cstyle && standard.derived) standard.kind = Conversion::Kind::Explicit;
            record_conversion(r, operand, standard); return r;
        }
    }
    TypeId from = decay(x.type);
    if (cv_cast) {
        if (!pointer(from) || !pointer(to) || types[types[from].child].kind == TypeKind::Function || !similar_type(from, to))
            throw std::runtime_error("invalid const cast");
        record_conversion(r, operand, c); return r;
    }
    bool enum_cast = (integral(from) && integral(to)) || (arithmetic(from) && integral(to));
    bool pointer_cast = false;
    if (pointer(from) && pointer(to)) {
        TypeId a = types[from].child, b = types[to].child;
        bool preserves_cv = !(types[a].cv & ~types[b].cv);
        pointer_cast = (cstyle || preserves_cv) && (reinterpret ||
            (fundamental(a, FT_VOID) && types[b].kind != TypeKind::Function) || derived_from(b, a));
        if (pointer_cast && op != KW_REINTERPET_CAST && derived_from(b, a)) {
            c.derived = true;
            if (!cstyle) check_base_access(b, a, s);
        }
    }
    bool integer_pointer = reinterpret && ((pointer(from) && integral(to) && width(to) >= 64) || (integral(from) && pointer(to)));
    if ((enum_cast && op != KW_REINTERPET_CAST) || pointer_cast || integer_pointer ||
        (op == KW_REINTERPET_CAST && integral(from) && from == types.unqualified(to))) {
        record_conversion(r, operand, c); return r;
    }
    throw std::runtime_error("invalid explicit cast");
}
} }
