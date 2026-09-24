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
    if (source.form == ExpressionForm::BoundMember) { result.form = source.form; result.object_use = source.object_use; }
    return result;
}
void Analyzer::demand_function_expression(const Expression& value)
{
    // A uniquely named free function is odr-used in every evaluated context,
    // including a discarded expression. Overload selection and nonstatic
    // member use retain their own call/address demand decisions.
    if (!definitions || (unevaluated_depth && unevaluated_depth != body_evaluation_depth) || !value.entity || types[value.type].kind != TypeKind::Function ||
        value.form == ExpressionForm::Overload || entities[value.entity].member_info) return;
    use_selected_function(value.entity,true);
}
Expression Analyzer::expression(NodeId n, ScopeId s)
{
    if (expressions[n].ready) {
        if (!unevaluated_depth) expressions.evaluated(n,true);
        if ((!unevaluated_depth || active_default_fact) && definitions) demand_template_storage(expressions[n].entity);
        demand_function_expression(expressions[n]);
        return expressions[n];
    }
    if (ast.nodes.occurrences[n].context) {
        auto frame = template_type_contexts.get(ast.nodes.occurrences[n].context);
        if (frame && substitution_frames[frame].overlay) s = expansion_scope(frame,s);
    }
    facts.edit(n).scope = s;
    Expression result = resolve_expression(n, s);
    demand_function_expression(result);
    bool storage = (!unevaluated_depth || active_default_fact) && definitions;
    bool substituted = ast.nodes.occurrences[n].context != 0;
    if (storage && substituted) demand_template_storage(result.entity);
    // A substituted use establishes its value at instantiation. Ordinary O0
    // reads retain the value already published at that source use, before its
    // storage demand can attach an out-of-class initializer. Both paths record
    // the decision here; lowering never consults a later declaration value.
    if ((ast[n].kind == Kind::IdExpression || ast[n].kind == Kind::Member) && result.entity && ((entities[result.entity].is_static && scopes[entities[result.entity].owner].kind == ScopeKind::Class) ||
        (entities[result.entity].kind == EntityKind::Variable && entities[result.entity].specialization)) &&
        entities[result.entity].constant.valid) {
        facts.edit(n).value = constants.size(); constants.push_back(entities[result.entity].constant);
    }
    if (storage && !substituted) demand_template_storage(result.entity);
    class_result(n,result,s);
    result.ready = true; result.evaluated = !unevaluated_depth;
    expressions.set(n,result);
    if (!facts[n].type) facts.edit(n).type = result.type;
    return expressions[n];
}
Expression Analyzer::resolve_expression(NodeId n, ScopeId s)
{
    Expression r;
    if (ast.nodes.occurrences[n].context && reuse_fixed_expression(n,s,r)) return r;
    ++expression_work;
    NodeId first = ast[n].first;
    switch (ast[n].kind) {
    case Kind::Lambda: return lambda_expression(n,s);
    case Kind::SizeofPack: {
        auto query = expression_query(n,s);
        r.type = types.fundamental(FT_UNSIGNED_LONG_INT);
        if (!query_fact(query).dependent) facts.edit(n).value = query_value(query);
        return r;
    }
    case Kind::New: return placement_new(n, s);
    case Kind::Delete: return delete_expression(n,s);
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
            if (closure_functions.get(current_function) && unevaluated_depth == body_evaluation_depth)
                throw std::runtime_error("this requires lambda capture");
            r.type = implicit_object_type(s);
            if (!r.type) throw std::runtime_error("this outside nonstatic member");
            return r;
        }
        r.type = types.fundamental(ast[n].op == KW_NULLPTR ? FT_NULLPTR_T : FT_BOOL); return r;
    case Kind::IdExpression: {
        auto op = operator_token(ast[n].detail);
        if (op == KW_NEW || op == KW_DELETE) global_allocation(op,array_operator(ast[n].detail));
        EntityId e = resolve(ast[n].detail, s);
        if (!e) {
            auto text = ids.spelling(terminal(ast[n].detail));
            throw std::runtime_error("unknown expression name: " + std::string(text.data,text.size));
        }
        if (function_binding(e)) e = explicit_template(ast[n].detail, e, s);
        if (placeholder_objects.get(e)) throw std::runtime_error("use before auto type deduction");
        r.entity = e; facts.edit(n).entity = e;
        if (entities[e].kind == EntityKind::Overload || (definitions && entities[e].template_info)) {
            r.form = ExpressionForm::Overload; r.category = ValueCategory::Lvalue;
            ScopeId naming = naming_class(name_owner(ast[n].detail, s));
            if (naming) { record_object(r, 0, 0, 0); object_uses[r.object_use].naming_scope = naming; }
            return r;
        }
        if (entities[e].kind != EntityKind::Variable && entities[e].kind != EntityKind::Parameter &&
            entities[e].kind != EntityKind::Enumerator && entities[e].kind != EntityKind::Function)
            throw std::runtime_error("expression requires value name");
        r.type = value_type(entities[e].type);
        if (closure_functions.get(current_function) && unevaluated_depth == body_evaluation_depth &&
            (entities[e].kind == EntityKind::Variable || entities[e].kind == EntityKind::Parameter) &&
            !entities[e].is_static && !entities[e].external_decl && !entities[e].constant.valid &&
            scopes[entities[e].owner].kind != ScopeKind::Namespace && scopes[entities[e].owner].kind != ScopeKind::Class &&
            !encloses(entities[current_function].scope,entities[e].owner))
            throw std::runtime_error("automatic variable requires lambda capture");
        if (entities[e].constant.valid && scopes[entities[e].owner].kind != ScopeKind::Namespace &&
            scopes[entities[e].owner].kind != ScopeKind::Class) {
            ScopeId use = s;
            while (use && scopes[use].kind != ScopeKind::Function) use = scopes[use].parent;
            if (use && !encloses(use, entities[e].owner)) {
                facts.edit(n).value = constants.size(); constants.push_back(entities[e].constant);
            }
        }
        if (nonstatic_field(e)) {
            if (closure_functions.get(current_function) && unevaluated_depth == body_evaluation_depth)
                throw std::runtime_error("member use requires lambda capture");
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
        if (entities[e].member_info) facts.edit(n).type = members[entities[e].member_info].call_type;
        if (entities[e].kind != EntityKind::Enumerator) r.category = ValueCategory::Lvalue;
        return r;
    }
    case Kind::BracedInit:
        for (NodeId c = first; c; c = ast[c].next) expression(c,s);
        r.form = ExpressionForm::InitializerList; return r;
    case Kind::Parenthesized: return value_fact(expression(first, s));
    case Kind::Call: return call_expression(n, s);
    case Kind::Unary: case Kind::Postfix: return unary_expression(n, s);
    case Kind::Binary: case Kind::Assignment: case Kind::Conditional: return binary_expression(n, s);
    case Kind::Cast: return cast_expression(n, s, type_id(first, s), ast[first].next);
    case Kind::TypeTrait: case Kind::Sizeof: {
        ++unevaluated_depth;
        TypeId t = ast[first].kind == Kind::TypeId ? type_id(first, s) : expression(first, s).type;
        --unevaluated_depth;
        if (ast[n].op == KW_NOEXCEPT) {
            r.type = types.fundamental(FT_BOOL);
            Constant value(r.type,expression_nonthrowing(first));
            facts.edit(n).value = constants.size(); constants.push_back(value);
            return r;
        }
        if (!t) throw std::runtime_error("sizeof unresolved overload");
        r.type = types.fundamental(FT_UNSIGNED_LONG_INT);
        // Layout can instantiate a class whose bounds/enumerators publish
        // other constants. Reserve this query's identity only after that
        // dependency completes, so it cannot point at a nested query's value.
        Constant value(r.type, size(t, ast[n].op == KW_ALIGNOF));
        facts.edit(n).value = constants.size(); constants.push_back(value);
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
        size(types[a].child);
        record_conversion(r, first, conversion(first, pointer(left) ? left : promote(left)));
        record_conversion(r, second, conversion(second, pointer(right) ? right : promote(right)));
        r.type = types[a].child; r.category = ValueCategory::Lvalue; return r;
    }
    case Kind::Member: {
        Expression object = expression(first, s);
        TypeId t = object.type;
        std::uint32_t arrow = 0;
        if (ast[n].op == OP_ARROW) {
            arrow = prepare_arrow(first,s);
            if (arrow) t = arrow_chains[arrow].type;
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
                EntityId found = class_type ? lookup(entities[types[t].entity].scope,ast[id].text,Lookup::Ordinary,true) : 0;
                if (!found) found = lookup(s,ast[id].text,Lookup::Ordinary);
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
        if (ast[part].op == KW_OPERATOR && ast[part].detail)
            e = conversion_lookup(name_owner(name,entities[types[t].entity].scope),type_id(ast[part].detail,s));
        if (!e) throw std::runtime_error("unknown member");
        if (definitions && function_binding(e)) e = explicit_template(name,e,s);
        r = member_value(e,types[t].cv,ast[n].op == OP_ARROW ? ValueCategory::Lvalue : object.category);
        facts.edit(n).entity = e;
        ScopeId naming = name_owner(name, entities[types[t].entity].scope);
        if (!function_binding(e)) check_access(e, s, naming, t);
        if (nonstatic_field(e)) record_object(r, first, t, base_steps(t, scopes[entities[e].owner].entity));
        if (!r.object_use) record_object(r, 0, 0, 0);
        object_uses[r.object_use].naming_scope = naming;
        object_uses[r.object_use].arrow = arrow;
        return r;
    }
    default: throw std::runtime_error("unsupported expression");
    }
}
Expression Analyzer::cast_expression(NodeId n, ScopeId s, TypeId to, NodeId operand, bool recipe)
{
    Expression r;
    r.type = value_type(to); r.form = ExpressionForm::Cast;
    facts.edit(n).type = to;
    if (!operand) {
        if (types[to].kind == TypeKind::LRef || types[to].kind == TypeKind::RRef) throw std::runtime_error("value-initialized reference");
        return r;
    }
    Expression x = expression(operand, s);
    auto publish = [&](Conversion c) {
        if (!recipe) { record_conversion(r,operand,c); return; }
        check_fixed_conversion(x,operand,c,s);
        store_call(r,{operand},{c}); r.inputs = CallInputs::Source;
    };
    Type target = types[to];
    ETokenType op = ast[n].op;
    bool cstyle = op == OP_LPAREN || ast[n].kind == Kind::Call;
    bool cv_cast = op == KW_CONST_CAST;
    if (!cv_cast && op != KW_REINTERPET_CAST && class_value(to)) {
        if (recipe) throw std::logic_error("class cast needs a constructor recipe");
        EntityId ctor = choose_constructor(to,{operand},&r,s);
        if (converting_transfer(ctor,r)) {
            auto conversion = result_conversion(ctor,r,to);
            r = Expression(); r.type = to; r.form = ExpressionForm::Cast;
            publish(conversion); return r;
        }
        r.type = to; r.form = ExpressionForm::Construction;
        facts.edit(n).entity = ctor; members[entities[ctor].member_info].complete_entry = true;
        record_object(r,0,0,0);
        return r;
    }
    if (!cv_cast && op != KW_REINTERPET_CAST && !fundamental(to,FT_VOID)) {
        Conversion selected = standard_conversion(x,to,operand);
        if (!selected.valid() && class_value(x.type)) selected = conversion_function(operand,to,true);
        bool ref = target.kind == TypeKind::LRef || target.kind == TypeKind::RRef;
        bool related = ref && (types.unqualified(x.type) == types.unqualified(target.child) || derived_from(x.type,target.child) || derived_from(target.child,x.type));
        if (ref && !related && !selected.valid()) selected = conversion(operand,to);
        if (selected.valid() && (selected.kind == Conversion::Kind::User || (ref && !related))) {
            publish(selected);
            if (ref) r.category = target.kind == TypeKind::LRef ? ValueCategory::Lvalue : ValueCategory::Xvalue;
            return r;
        }
    }
    auto selected = explicit_builtin_conversion(x,to,cstyle ? OP_LPAREN : op,s,operand);
    if (selected.reference) r.category = target.kind == TypeKind::LRef ? ValueCategory::Lvalue : ValueCategory::Xvalue;
    publish(selected); return r;
}
} }
