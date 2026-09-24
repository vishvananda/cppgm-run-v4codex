#include "semantic/analyzer.h"
namespace cppgm { namespace semantic {
ArgumentId Analyzer::address_template_argument(QueryId query, TypeId target)
{
    // [temp.arg.nontype]/1,5. A source query keeps the permitted syntactic
    // form; a Value query is an already-validated substituted parameter.
    auto q = type_queries[query];
    if (q.kind == QueryKind::Cast && q.op == TOK_INVALID)
        return convert_argument(value_argument_id(query_edges[q.offset]),target);
    auto fact = query_fact(query); auto source = fact.expression;
    bool reference = types[target].kind == TypeKind::LRef;
    bool canonical = q.kind == QueryKind::Value;
    Constant value;
    if (canonical) value = constants[query_value(query)];
    else if (!reference && source.form != ExpressionForm::Overload)
        value = constants[query_value(query)];
    auto publish = [&](Constant v) {
        TypeQuery result; result.type = target; result.value = v.bits;
        return value_argument_id(intern_query(result,{}));
    };
    // Null expressions need no id-expression form, but integral zero is not
    // a permitted pointer NTTP conversion. Casted pointer zero is permitted.
    if (!reference && value.valid && !value.bits &&
        (pointer(value.type) || fundamental(value.type,FT_NULLPTR_T))) {
        unsigned added = 0;
        if (!fundamental(value.type,FT_NULLPTR_T) && (!pointer(target) ||
            !qualification(types[value.type].child,types[target].child,added))) return 0;
        return publish(Constant(target,0));
    }
    if (fundamental(target,FT_NULLPTR_T)) return 0;
    if (!canonical) {
        while (q.kind == QueryKind::Parenthesized) q = type_queries[query_edges[q.offset]];
        bool address = q.kind == QueryKind::Unary && q.op == OP_AMP;
        if (address) {
            if (reference) return 0;
            q = type_queries[query_edges[q.offset]];
            while (q.kind == QueryKind::Parenthesized) q = type_queries[query_edges[q.offset]];
        }
        if (q.kind != QueryKind::Name && q.kind != QueryKind::QualifiedValue) return 0;
        if (!reference && !address && source.form != ExpressionForm::Overload &&
            types[source.type].kind != TypeKind::Array && types[source.type].kind != TypeKind::Function) return 0;
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
    if (!c.valid() || c.derived || c.temporary || (reference && source.category != ValueCategory::Lvalue)) return 0;
    // Only qualification, array/function decay, or direct reference binding
    // applies. In particular void/base pointer conversions are not admitted.
    if (!c.function) {
        unsigned added = 0;
        auto from = reference ? source.type : decay(source.type);
        if (!reference && !pointer(from)) return 0;
        if (!qualification(reference ? from : types[from].child,types[target].child,added)) return 0;
        if (reference) {
            auto to = types[target].child;
            while (types[from].kind == TypeKind::Array && types[to].kind == TypeKind::Array) {
                from = types[from].child; to = types[to].child;
            }
            if (types.unqualified(from) != types.unqualified(to)) return 0;
        }
    }
    if (c.function) {
        if (deleted_transfer(c.function)) return 0;
        check_access(c.function,q.context,object_uses[source.object_use].naming_scope);
        value = Constant(target,constant_entity_address(c.function));
    } else if (canonical) value = convert(value,target);
    else value = constant_query_conversion(query,c);
    if (!value.valid || !value.bits) return 0;
    auto address = constant_addresses[value.bits];
    auto storage = constant_storage[address.storage];
    auto e = storage.entity;
    if (!e || storage.literal || !storage.live || nonstatic_field(e) || entities[e].thread_local_storage) return 0;
    if (scopes[entities[e].owner].kind != ScopeKind::Namespace &&
        !(scopes[entities[e].owner].kind == ScopeKind::Class && entities[e].is_static)) return 0;
    if (address.parent && !(types[storage.type].kind == TypeKind::Array && !reference &&
        !constant_addresses[address.parent].parent && address.selector == 0)) return 0;
    if (entities[e].kind == EntityKind::Function) {
        // A template argument is potentially evaluated even when the type
        // containing it is mentioned by an unevaluated operand or candidate.
        // Queue the ordinary definition owner; body errors are not SFINAE.
        struct Evaluated { unsigned& depth; unsigned saved;
            Evaluated(unsigned& d) : depth(d), saved(d) { depth = 0; }
            ~Evaluated() { depth = saved; }
        } evaluated(unevaluated_depth);
        use_selected_function(e,true);
    }
    return publish(value);
}
} }
