#include "semantic/analyzer.h"
namespace cppgm { namespace semantic {
ArgumentId Analyzer::member_address_template_argument(QueryId query, TypeId target)
{
    auto identity = intern_arguments({query,target,access_override,unsigned(explicit_instantiation_naming)});
    if (auto known = address_template_arguments.get(identity)) return known;
    SubstitutionDependency dependency(incomplete_substitution);
    auto q = type_queries[query];
    if (q.kind == QueryKind::Cast && q.op == TOK_INVALID)
        return convert_argument(value_argument_id(query_edges[q.offset]),target);
    auto fact = query_fact(query);
    if (fact.state == FactState::Failure) return 0;
    auto source = fact.expression;
    Constant value;
    if (source.form != ExpressionForm::Overload) value = constants[query_value(query)];
    auto publish = [&](EntityId member) {
        TypeQuery result; result.type = target; result.value = member;
        auto arg = value_argument_id(intern_query(result,{}));
        if (!fact.incomplete && !incomplete_substitution) address_template_arguments.put(identity,arg);
        return arg;
    };
    // [temp.arg.nontype]/1,5: null member values and nullptr are permitted;
    // integer zero and base-to-derived member conversions are not.
    if (value.valid && !value.bits && fundamental(value.type,FT_NULLPTR_T)) return publish(0);
    // Parentheses around the complete address preserve its value and form.
    // Parentheses around the qualified-id operand still prevent formation.
    while (q.kind == QueryKind::Parenthesized) q = type_queries[query_edges[q.offset]];
    bool canonical = q.kind == QueryKind::Value;
    bool null = value.valid && !value.bits && types[value.type].kind == TypeKind::MemberPointer;
    if (!canonical && !null) {
        if (q.kind != QueryKind::Unary || q.op != OP_AMP || !q.value) return 0;
        q = type_queries[query_edges[q.offset]];
        if (q.kind != QueryKind::Name && q.kind != QueryKind::QualifiedValue) return 0;
        if (q.arguments && source.form == ExpressionForm::Overload) {
            auto pack = argument_packs[q.arguments];
            std::vector<ArgumentId> args(argument_types.begin()+pack.offset,argument_types.begin()+pack.offset+pack.count);
            EntityId family = 0;
            for (auto e : candidates(source.entity)) {
                if (!entities[e].template_info) continue;
                auto instance = specialize(e,args,true);
                if (instance) family = merge_lookup(family,instance);
            }
            if (!family) return 0;
            source.entity = family;
        }
    }
    auto c = standard_conversion(source,target);
    if (!c.valid() || c.derived || c.temporary || c.reference) return 0;
    if (c.function) {
        auto e = c.function;
        if (entities[e].is_static || !entities[e].member_info ||
            scopes[entities[e].owner].entity != types[target].entity || deleted_transfer(e) ||
            !accessible(e,q.context,object_uses[source.object_use].naming_scope,
                entities[scopes[naming_class(object_uses[source.object_use].naming_scope)].entity].type)) return 0;
        value = Constant(target,e);
    } else {
        unsigned added = 0;
        if (types[source.type].kind != TypeKind::MemberPointer || !qualification(source.type,target,added)) return 0;
        value = convert(value,target);
    }
    if (!value.valid) return 0;
    if (value.bits && types[types[target].child].kind == TypeKind::Function) {
        // A member address is potentially evaluated, even when its enclosing
        // template-id appears in an unevaluated operand. Demand just this body.
        struct Evaluated { unsigned& depth; unsigned saved;
            Evaluated(unsigned& d) : depth(d), saved(d) { depth = 0; }
            ~Evaluated() { depth = saved; }
        } evaluated(unevaluated_depth);
        use_selected_function(value.bits,true);
    }
    return publish(value.bits);
}
} }
