#include "semantic/analyzer.h"
#include <stdexcept>
namespace cppgm { namespace semantic {
using syntax::Kind;
RttiExpression Analyzer::rtti_expression(NodeId n) const
{
    auto id = rtti_expression_index.get(n);
    if (!id) id = rtti_expression_index.get(template_fixed_expressions.get(ast.nodes.occurrences[n].source));
    if (!id) throw std::logic_error("missing checked RTTI expression");
    return rtti_expressions[id];
}
void Analyzer::demand_rtti(const RttiExpression& use)
{
    if (unevaluated_depth) return;
    for (auto t : {use.type,use.source}) {
        record_rtti_type(t);
        if (class_value(t) && !entities[types[t].entity].specialization)
            demand_vtable(types[t].entity,VtableReason::Rtti);
    }
}
Expression Analyzer::rtti_operand(QueryId id)
{
    auto fact = query_fact(id);
    auto operand = fact.expression;
    if (operand.type || fact.dependent || fact.state == FactState::Failure) return operand;
    // An explicit function template-id can name a unique signature without a
    // target pointer type. Complete only those signatures, never their bodies.
    while (type_queries[id].kind == QueryKind::Parenthesized) id = query_edges[type_queries[id].offset];
    auto q = type_queries[id]; auto entity = operand.entity;
    if (q.arguments && function_binding(entity)) {
        auto pack = argument_packs[q.arguments];
        std::vector<TypeId> args(argument_types.begin()+pack.offset,argument_types.begin()+pack.offset+pack.count);
        EntityId selected = 0;
        for (auto candidate : candidates(entity)) {
            if (!entities[candidate].template_info) continue;
            auto instance = specialize(candidate,args,true);
            if (instance && (entities[instance].kind != EntityKind::Function || entities[instance].template_info || selected)) return operand;
            if (instance) selected = instance;
        }
        entity = selected;
    }
    if (entity && entities[entity].kind == EntityKind::Function && !entities[entity].template_info) {
        if (deleted_transfer(entity)) throw std::runtime_error("typeid refers to a deleted function");
        if (entities[entity].member_info && !entities[entity].is_static) return operand;
        require_deduced_return(entity);
        operand.type = entities[entity].type; operand.entity = entity;
    }
    return operand;
}
TypeId Analyzer::typeinfo_result_type()
{
    // [expr.typeid] requires the library declaration in namespace std. Resolve
    // that declaration normally; no unqualified or fabricated substitute.
    auto ns = lookup(global,ids.intern(TextView("std",3)),Lookup::Ordinary,true);
    auto e = ns && entities[ns].kind == EntityKind::Namespace ?
        lookup(entities[ns].scope,ids.intern(TextView("type_info",9)),Lookup::Ordinary,true) : 0;
    if (!e || !class_value(entities[e].type)) throw std::runtime_error("typeid requires std::type_info");
    return types.qualify(entities[e].type,1);
}
Expression Analyzer::typeid_expression(NodeId n, ScopeId s)
{
    auto first = ast.first(n);
    RttiExpression use;
    if (ast.kind(first) == Kind::TypeId) use.type = value_type(type_id(first,s));
    else {
        // Formation queries check the operand without materialization or body
        // demand. Only a polymorphic glvalue promotes it to evaluated work.
        auto query = expression_query(first,s);
        auto operand = rtti_operand(query);
        use.type = operand.type;
        if (class_value(use.type)) size(use.type);
        use.dynamic = operand.category != ValueCategory::Prvalue && class_value(use.type) && polymorphic(types[use.type].entity);
        if (use.dynamic && default_inquiry_queries.get(ast.nodes.occurrences[first].source) && evaluated_prototype_parameter(query))
            throw std::runtime_error("parameter used in evaluated default typeid operand");
        if (use.dynamic) expression(first,s);
    }
    if (!use.type) throw std::runtime_error("typeid requires a resolved type");
    if (class_value(use.type)) size(use.type);
    demand_rtti(use);
    use.type = types.unqualified(use.type);
    Expression result; result.type = typeinfo_result_type();
    result.category = ValueCategory::Lvalue; result.form = ExpressionForm::Typeid;
    rtti_expression_index.put(n,rtti_expressions.size()); rtti_expressions.push_back(use);
    return result;
}
Conversion Analyzer::dynamic_cast_conversion(Expression x, TypeId to, ScopeId s, RttiExpression& use)
{
    Conversion c; c.target = to;
    auto target = types[to];
    use.reference = target.kind == TypeKind::LRef || target.kind == TypeKind::RRef;
    if (!use.reference && target.kind != TypeKind::Pointer) return c;
    auto from = use.reference ? x.type : decay(x.type);
    if (!use.reference && !pointer(from)) return c;
    use.source = use.reference ? from : types[from].child;
    use.type = target.child;
    bool to_void = !use.reference && fundamental(use.type,FT_VOID);
    if ((!to_void && !class_value(use.type)) || !class_value(use.source)) return c;
    if (!to_void) size(use.type);
    size(use.source);
    if (types[use.source].cv & ~types[use.type].cv) return c;
    if (use.reference && (x.category == ValueCategory::Prvalue ||
        (target.kind == TypeKind::LRef && x.category != ValueCategory::Lvalue))) return c;
    if (types.unqualified(use.type) == types.unqualified(use.source) || derived_from(use.source,use.type))
        return explicit_builtin_conversion(x,to,KW_STATIC_CAST,s);
    if (!polymorphic(types[use.source].entity)) return c;
    use.dynamic = true;
    if (to_void) use.hint = -2;
    else if (derived_from(use.type,use.source)) {
        auto path = base_path(use.type,types[use.source].entity);
        bool accessible = base_accessible(types[use.type].entity,types[use.source].entity,global);
        if (!accessible) use.hint = -2;
        else if (base_adjustments[path].ambiguous)
            use.hint = virtual_base_count(types[use.type].entity) ? -1 : -3;
        else {
            auto projection = base_adjustments[base_steps(use.type,types[use.source].entity)];
            // A positive src2dst hint promises a unique *nonvirtual* base.
            // Shared bases (including their nonvirtual descendants) have a
            // most-derived-dependent offset and must use the unknown hint.
            use.hint = projection.virtual_row ? -1 : std::int64_t(projection.total);
        }
    } else use.hint = -2;
    c.rank = 0; c.reference = use.reference;
    return c;
}
Expression Analyzer::dynamic_cast_expression(NodeId n, ScopeId s, TypeId to, NodeId operand)
{
    auto x = expression(operand,s);
    RttiExpression use;
    auto c = dynamic_cast_conversion(x,to,s,use);
    if (!c.valid()) throw std::runtime_error("invalid dynamic_cast");
    Expression result; result.type = value_type(to);
    result.category = types[to].kind == TypeKind::LRef ? ValueCategory::Lvalue :
        types[to].kind == TypeKind::RRef ? ValueCategory::Xvalue : ValueCategory::Prvalue;
    facts.edit(n).type = to;
    if (!use.dynamic) { result.form = ExpressionForm::Cast; record_conversion(result,operand,c); return result; }
    result.form = ExpressionForm::DynamicCast;
    demand_rtti(use);
    rtti_expression_index.put(n,rtti_expressions.size()); rtti_expressions.push_back(use);
    return result;
}
bool Analyzer::typeinfo_comparison(EntityId e, ETokenType op)
{
    if ((op != OP_EQ && op != OP_NE) || entities[e].body || !entities[e].member_info) return false;
    if (entities[e].name != operator_name(op)) return false;
    auto owner = entities[e].owner;
    if (scopes[owner].name != ids.intern(TextView("type_info",9))) return false;
    auto ns = scopes[owner].parent;
    if (scopes[ns].kind != ScopeKind::Namespace || scopes[ns].parent != global || scopes[ns].name != ids.intern(TextView("std",3))) return false;
    auto f = types[entities[e].type];
    return fundamental(f.child,FT_BOOL) && f.count == 1 && f.cv == 1 &&
        types[types.parameters[f.offset]].kind == TypeKind::LRef &&
        types[types.parameters[f.offset]].child == typeinfo_result_type();
}
} }
