#include "semantic/analyzer.h"
#include <stdexcept>

namespace cppgm { namespace semantic {
using syntax::Kind;
Expression Analyzer::value_fact(const Expression& source) const
{
    Expression result;
    result.type = source.type; result.storage_type = source.storage_type; result.entity = source.entity;
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
    if (definitions && value.entity && entities[value.entity].kind == EntityKind::Enumerator &&
        (!unevaluated_depth || unevaluated_depth == body_evaluation_depth)) {
        auto constant = entities[value.entity].constant;
        if (constant.valid && constant.bits && (pointer(constant.type) || types[constant.type].kind == TypeKind::LRef)) {
            auto target = constant_storage[constant_addresses[constant.bits].storage].entity;
            if (target && entities[target].kind == EntityKind::Function) use_selected_function(target,true);
            else if (target && !unevaluated_depth) demand_template_storage(target);
        }
    }
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
        prepare_expression_discard(n);
        return expressions[n];
    }
    if (ast.nodes.occurrences[n].context) {
        auto frame = template_type_contexts.get(ast.nodes.occurrences[n].context);
        if (frame && substitution_frames[frame].overlay) s = expansion_scope(frame,s);
    }
    facts.edit(n).scope = s;
    Expression result = resolve_expression(n, s);
    storage_expression(n,result);
    // Reused fixed template facts share type/conversions, but capture storage
    // belongs to this checked occurrence and enclosing closure specialization.
    unsigned capture = 0;
    if (ast[n].kind == Kind::KeywordLiteral && ast[n].op == KW_THIS) capture = capture_object(0);
    if (ast[n].kind == Kind::IdExpression && result.entity) {
        if (nonstatic_field(result.entity)) capture = capture_object(0);
        else if (!entities[result.entity].constant.valid) capture = capture_object(result.entity);
    }
    if (ast[n].kind == Kind::Call && result.object_use) {
        auto use = object_uses[result.object_use];
        if (use.type && !use.node) capture = capture_object(0);
    }
    if (capture) {
        auto use = object_uses[result.object_use]; use.capture = capture;
        result.object_use = object_uses.size(); object_uses.push_back(use);
        if (closure_captures[capture].object) result.type = capture_type(capture);
    }
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
    record_discard_form(n,result);
    class_result(n,result,s);
    result.ready = true; result.evaluated = !unevaluated_depth;
    expressions.set(n,result);
    if (!facts[n].type) facts.edit(n).type = result.type;
    if (ast[n].kind == Kind::Call && atomic_kind(facts[n].entity).form == AtomicForm::Always) {
        auto value = atomic_constant(n,s);
        if (!value.valid) throw std::runtime_error("always_lock_free requires a constant size");
        facts.edit(n).value = constants.size(); constants.push_back(value);
        auto checked = expressions[n]; checked.form = ExpressionForm::ConstantQuery; expressions.set(n,checked);
    }
    prepare_expression_discard(n);
    return expressions[n];
}
Expression Analyzer::resolve_expression(NodeId n, ScopeId s)
{
    Expression r;
    if (ast.nodes.occurrences[n].context && reuse_fixed_expression(n,s,r)) return r;
    ++expression_work;
    NodeId first = ast[n].first;
    switch (ast[n].kind) {
    case Kind::Fold: return fold_expression(n,s);
    case Kind::FunctionName:
        r.entity = predefined_function_name(n,s); facts.edit(n).entity = r.entity;
        r.type = entities[r.entity].type; r.category = ValueCategory::Lvalue; return r;
    case Kind::VaArg: return va_arg_expression(n,s);
    case Kind::StatementExpression: return statement_expression(n,s);
    case Kind::Throw: return throw_expression(n,s);
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
            r.type = implicit_object_type(s);
            if (!r.type) throw std::runtime_error("this outside nonstatic member");
            return r;
        }
        r.type = types.fundamental(ast[n].op == KW_NULLPTR ? FT_NULLPTR_T : FT_BOOL); return r;
    case Kind::IdExpression: {
        auto op = operator_token(ast[n].detail);
        if (op == KW_NEW || op == KW_DELETE) global_allocation(op,array_operator(ast[n].detail));
        EntityId e = resolve(ast[n].detail,s);
        if (!e && ast[ast[n].detail].first == ast[ast[n].detail].last)
            e = builtin_function(terminal(ast[n].detail));
        if (!e) {
            auto name = terminal(ast[n].detail);
            if (!name) throw std::runtime_error("expression requires a value name");
            auto text = ids.spelling(name);
            throw std::runtime_error("unknown expression name: " + std::string(text.data,text.size));
        }
        if (function_binding(e)) e = explicit_template(ast[n].detail, e, s);
        if (placeholder_objects.get(e)) throw std::runtime_error("use before auto type deduction");
        r.entity = e; facts.edit(n).entity = e;
        auto intrinsic = intrinsic_function(e);
        bool atomic_family = intrinsic == Intrinsic::Atomic;
        if (entities[e].kind == EntityKind::Overload || (definitions && entities[e].template_info) || atomic_family || (intrinsic >= Intrinsic::AddOverflow && intrinsic <= Intrinsic::MulOverflow) || (intrinsic >= Intrinsic::Clzg && intrinsic <= Intrinsic::Popcountg)) {
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
            while (use && scopes[use].kind != ScopeKind::Function && !template_object_context_index.get(use))
                use = scopes[use].parent;
            if (use && !encloses(use, entities[e].owner)) {
                facts.edit(n).value = constants.size(); constants.push_back(entities[e].constant);
            }
        }
        if (nonstatic_field(e)) {
            TypeId object = implicit_object_type(s);
            auto owner = entities[scopes[entities[e].owner].entity].type;
            bool related_object = object && (types.unqualified(types[object].child) == types.unqualified(owner) ||
                derived_from(types[object].child,owner));
            if (related_object) {
                size(types[object].child);
                auto naming = name_owner(ast[n].detail,s);
                bool qualified = ast[ast[n].detail].first != ast[ast[n].detail].last;
                if (qualified && base_adjustments[base_path(types[object].child,scopes[entities[e].owner].entity)].ambiguous)
                    record_member_receiver(r,0,types[object].child,e,naming,true,s);
                else record_object(r, 0, object, base_steps(types[object].child, scopes[entities[e].owner].entity));
                if (types[entities[e].type].kind != TypeKind::LRef && types[entities[e].type].kind != TypeKind::RRef)
                    r.type = types.qualify(r.type, types[types[object].child].cv & (entities[e].mutable_field ? 2 : 3));
            } else if (!unevaluated_depth && !class_facts[entities[scopes[entities[e].owner].entity].class_info].storage) throw std::runtime_error("field requires object");
        }
        if (entities[e].kind == EntityKind::Enumerator && types[entities[e].type].kind != TypeKind::LRef && !entities[types[r.type].entity].complete)
            r.type = entities[e].constant.type;
        if (function_binding(e) && scopes[entities[e].owner].kind == ScopeKind::Class) {
            if (!r.object_use) record_object(r, 0, 0, 0);
            object_uses[r.object_use].naming_scope = naming_class(name_owner(ast[n].detail, s));
        }
        if (entities[e].member_info) facts.edit(n).type = members[entities[e].member_info].call_type;
        if (entities[e].kind != EntityKind::Enumerator || types[entities[e].type].kind == TypeKind::LRef) r.category = ValueCategory::Lvalue;
        return r;
    }
    case Kind::BracedInit:
        for (NodeId c = first; c; c = ast[c].next)
            expression(ast[c].kind == Kind::DesignatedInit ? ast[c].first : c,s);
        r.form = ExpressionForm::InitializerList; r.arguments = n; return r;
    case Kind::Parenthesized: return value_fact(expression(first, s));
    case Kind::Call: return call_expression(n, s);
    case Kind::Unary: case Kind::Postfix: return unary_expression(n, s);
    case Kind::Binary: case Kind::Assignment: case Kind::Conditional: return binary_expression(n, s);
    case Kind::Cast: return cast_expression(n, s, type_id(first, s), ast[first].next);
    case Kind::TypeTrait: case Kind::Sizeof: {
        if (ast[n].kind == Kind::TypeTrait && ast[n].flags) {
            auto query = expression_query(n,s);
            if (query_fact(query).state == FactState::Failure) throw std::runtime_error("invalid type operation");
            r = query_fact(query).expression;
            if (type_queries[query].kind == QueryKind::Offsetof)
                for (auto step = ast[first].next; step; step = ast[step].next)
                    if (ast[step].kind == Kind::Subscript) expression(ast[step].first,s);
            auto value = query_value(query);
            r.form = type_queries[query].kind == QueryKind::Offsetof && !constants[value].valid ?
                ExpressionForm::Ordinary : ExpressionForm::ConstantQuery;
            facts.edit(n).value = value; return r;
        }
        if (ast[n].op == KW_TYPEID) return typeid_expression(n,s);
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
        auto bytes = ast[n].op == KW_ALIGNOF && ast[first].kind != Kind::TypeId ? expression_alignment(expressions[first]) : size(t,ast[n].op == KW_ALIGNOF);
        Constant value(r.type,bytes);
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
        TypeId destructor_type = t;
        if (destructor) {
            TypeId named = destructor_target(name,t,s);
            if (!named || (types.unqualified(named) != types.unqualified(t) && !derived_from(t,named)))
                throw std::runtime_error("destructor name does not match object type");
            destructor_type = named;
            if (ast[name].first != part) {
                auto prefix = ast[name].first;
                while (ast[prefix].next != part) prefix = ast[prefix].next;
                auto qualifier = type_name(name,s,prefix);
                if (types.unqualified(qualifier) != types.unqualified(named))
                    throw std::runtime_error("destructor qualifier does not match target");
            }
            if (!class_type) {
                if (!(arithmetic(t) || pointer(t) || (types[t].kind == TypeKind::Named && entities[types[t].entity].underlying) || fundamental(t, FT_NULLPTR_T))) throw std::runtime_error("pseudo-destructor requires scalar");
                r.type = types.function(types.fundamental(FT_VOID), {}, false);
                record_object(r,first,t,0); object_uses[r.object_use].arrow = arrow;
                r.form = ExpressionForm::PseudoDestructor; return r;
            }
        }
        if (!class_type) throw std::runtime_error("member of non-class");
        size(t); // Establish layout once at the semantic owner before recording field use.
        if (operator_token(name) == OP_ASS) ensure_transfers(t, true);
        EntityId e = destructor ? default_destructor(destructor_type, s) : lookup(name_owner(name, entities[types[t].entity].scope), terminal(name), Lookup::Ordinary, true);
        if (ast[part].op == KW_OPERATOR && ast[part].detail)
            e = conversion_lookup(name_owner(name,entities[types[t].entity].scope),type_id(ast[part].detail,s));
        if (!e) throw std::runtime_error("unknown member");
        if (definitions && !destructor && function_binding(e)) e = explicit_template(name,e,s);
        r = member_value(e,types[t].cv,ast[n].op == OP_ARROW ? ValueCategory::Lvalue : object.category);
        facts.edit(n).entity = e;
        ScopeId naming = name_owner(name, entities[types[t].entity].scope);
        if (!function_binding(e)) check_access(e, s, naming, t);
        if (nonstatic_field(e)) {
            bool qualified = ast[name].first != ast[name].last;
            if (qualified && base_adjustments[base_path(t,scopes[entities[e].owner].entity)].ambiguous)
                record_member_receiver(r,first,t,e,naming,true,s);
            else record_object(r, first, t, base_steps(t, scopes[entities[e].owner].entity));
        }
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
    auto storage = to; to = types.signature(to);
    if (ast[n].op == KW_DYNAMIC_CAST) {
        auto result = dynamic_cast_expression(n,s,to,operand);
        if (storage != to) result.storage_type = value_type(storage);
        return result;
    }
    Expression r;
    r.type = value_type(to); r.form = ExpressionForm::Cast;
    if (storage != to) r.storage_type = value_type(storage);
    facts.edit(n).type = to;
    if (ast[n].kind == Kind::Cast && ast[n].op == OP_LPAREN && ast[operand].kind == Kind::BracedInit) {
        if (types[to].kind == TypeKind::LRef || types[to].kind == TypeKind::RRef)
            throw std::runtime_error("compound literal requires an object type");
        expression(operand,s);
        auto c = list_initialization(operand,to,s,true);
        r.type = c.target;
        r.form = class_value(to) || types[to].kind == TypeKind::Array ? ExpressionForm::ListValue : ExpressionForm::Cast;
        if (recipe) {
            check_fixed_conversion(Expression(),operand,c,s);
            store_call(r,{operand},{c}); r.inputs = CallInputs::Source;
        } else {
            record_conversion(r,operand,c);
            if (r.form == ExpressionForm::ListValue) {
                record_object(r,0,0,0);
                object_uses[r.object_use].temporary = converted_temporary(conversions[r.conversions]);
            }
        }
        facts.edit(n).type = c.target; return r;
    }
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
    auto selected = explicit_builtin_conversion(x,to,cstyle ? OP_LPAREN : op,s,operand);
    if (!selected.valid()) throw std::runtime_error("invalid explicit cast");
    if (selected.reference) r.category = target.kind == TypeKind::LRef ? ValueCategory::Lvalue : ValueCategory::Xvalue;
    publish(selected); return r;
}
} }
