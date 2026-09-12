#include "semantic/analyzer.h"
#include <algorithm>
#include <stdexcept>
namespace cppgm { namespace semantic {
using syntax::Kind;
void Analyzer::finish_allocations()
{
    for (auto& use : placements) {
        if (!use.array) continue;
        if (use.bound && !constant_fact(use.bound).valid) {
            // Retain the source-width O0 extent arithmetic only with a
            // completed, constant-return bound proof. Otherwise widen before
            // multiplying, so valid size_t extents cannot wrap at 32 bits.
            TypeId t = expression_fact(use.bound).type;
            EntityId callee = ast[use.bound].kind == Kind::Call ? facts[use.bound].entity : 0;
            NodeId body = entities[callee].body, ret = ast[body].first;
            Constant bound = ast[ret].kind == Kind::Return && !ast[ret].next ? constant_fact(ast[ret].first) : Constant();
            if (!bound.valid && ast[ret].kind == Kind::Return && !ast[ret].next &&
                ast[ast[ret].first].kind == Kind::Literal && !ast.literals[ast[ast[ret].first].literal].suffix)
                bound = evaluate(ast[ret].first,facts[ast[ret].first].scope);
            if (width(t) == 32 && bound.valid && (is_unsigned(bound.type) || static_cast<std::int64_t>(bound.bits) >= 0)) {
                std::uint64_t max = is_unsigned(t) ? 4294967295ull : 2147483647ull;
                use.narrow_extent = use.cookie <= max && bound.bits <= (max-use.cookie)/use.stride;
            }
        }
        if (!use.constructor) continue;
        auto e = use.constructor; auto& m = members[entities[e].member_info];
        // An empty no-argument body with no subobject actions has no work per
        // element. Keep this proof on the allocation record, after demand.
        bool empty = m.body && ast[m.body].kind == Kind::Compound && !ast[m.body].first &&
            !m.action_count && !m.inherited_constructor && !types[entities[e].type].count;
        use.construct = constructor_needed(e) && !empty;
        if (use.construct) m.complete_entry = true;
    }
}
EntityId Analyzer::global_allocation(ETokenType op, bool array)
{
    TypeId v = types.fundamental(FT_VOID), ptr = types.compound(TypeKind::Pointer,v);
    TypeId size_type = types.fundamental(FT_UNSIGNED_LONG_INT);
    TypeId type = types.function(op == KW_NEW ? ptr : v,{op == KW_NEW ? size_type : ptr},false);
    EntityId e = declare_function(global,operator_name(op,array),0,type);
    entities[e].key = op; entities[e].array_allocation = array;
    entities[e].allocation_runtime = (op == KW_NEW ? 1 : 3) + array;
    if (op == KW_DELETE && !(entities[e].exception_spec & 3)) entities[e].exception_spec = 129;
    return e;
}
EntityId Analyzer::select_deallocation(TypeId t, bool array, bool force_global, ScopeId s)
{
    EntityId family = 0;
    IdentifierId name = operator_name(KW_DELETE,array);
    if (!force_global && class_value(t)) family = lookup(entities[types[t].entity].scope,name,Lookup::Ordinary,true);
    if (!family) {
        global_allocation(KW_DELETE,array); family = lookup(global,name);
    }
    EntityId unsized = 0, sized = 0;
    TypeId ptr = types.compound(TypeKind::Pointer,types.fundamental(FT_VOID));
    for (EntityId e : candidates(family)) {
        ++candidate_work; Type f = types[entities[e].type];
        if (entities[e].template_info || f.variadic || !f.count || types.parameters[f.offset] != ptr) continue;
        if (f.count == 1) { if (unsized) throw std::runtime_error("ambiguous deallocation"); unsized = e; }
        if (f.count == 2 && fundamental(types.parameters[f.offset+1],FT_UNSIGNED_LONG_INT)) {
            if (sized) throw std::runtime_error("ambiguous sized deallocation"); sized = e;
        }
    }
    EntityId selected = unsized ? unsized : sized;
    if (!selected || deleted_transfer(selected)) throw std::runtime_error("no usable deallocation function");
    check_access(selected,s,entities[selected].owner); demand_member(selected);
    return selected;
}
Expression Analyzer::delete_expression(NodeId n, ScopeId s)
{
    DeleteExpression use; use.array = child(n,Kind::ArrayDelete); use.operand = ast[n].last;
    Expression x = expression(use.operand,s);
    TypeId pointer_type = decay(x.type);
    if (class_value(x.type)) {
        pointer_type = 0;
        for (EntityId e : conversion_candidates(x.type)) {
            TypeId t = decay(types[entities[e].type].child);
            if (members[entities[e].member_info].explicit_constructor || !object_pointer(t) || !object_conversion(e,x.type,x.category).valid()) continue;
            if (pointer_type && pointer_type != t) throw std::runtime_error("ambiguous delete pointer conversion");
            pointer_type = t;
        }
    }
    if (!pointer_type || !object_pointer(pointer_type)) throw std::runtime_error("delete requires object pointer");
    use.type = types[pointer_type].child; size(use.type);
    use.leaf = use.type;
    while (types[use.leaf].kind == TypeKind::Array) use.leaf = types[use.leaf].child;
    if (use.array && class_value(use.leaf)) use.cookie = std::max<std::uint64_t>(8,size(use.type,true));
    use.conversion = conversion(use.operand,pointer_type); apply_conversion(use.operand,use.conversion);
    use.destructor = default_destructor(use.type,s);
    if (!use.array && use.destructor && members[entities[use.destructor].member_info].virtual_member)
        use.virtual_slot = members[entities[use.destructor].member_info].virtual_slot + 1;
    use.deallocation = select_deallocation(use.leaf,use.array,child(n,Kind::Global),s);
    use.sized = types[entities[use.deallocation].type].count == 2;
    delete_index.put(n,deletions.size()); deletions.push_back(use);
    Expression result; result.type = types.fundamental(FT_VOID); return result;
}
} }
