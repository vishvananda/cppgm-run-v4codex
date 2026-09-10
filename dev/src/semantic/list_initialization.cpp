#include "semantic/analyzer.h"
#include <stdexcept>
namespace cppgm { namespace semantic {
using syntax::Kind;
Conversion Analyzer::list_element(NodeId& cursor, TypeId to, ScopeId s)
{
    if (!cursor) return list_initialization(0,to,s);
    NodeId source = cursor;
    expression(source,s);
    Conversion c = conversion(source,to);
    if (c.valid()) { cursor = ast[source].next; return c; }
    if (ast[source].kind != Kind::BracedInit && aggregate_type(to)) {
        auto id = list_aggregate(cursor,to,s);
        c.target = to; c.kind = Conversion::Kind::ListPlan; c.materialization = id;
        c.rank = list_plans[id].rank;
    }
    return c;
}
std::uint32_t Analyzer::list_aggregate(NodeId& cursor, TypeId to, ScopeId s)
{
    ListPlan plan; plan.source = cursor; plan.target = to; plan.scope = s; plan.aggregate = true;
    std::vector<NodeId> args;
    std::vector<Conversion> selected;
    std::vector<ListField> fields;
    auto add = [&](TypeId t, EntityId field, std::uint64_t index, std::uint64_t count) {
        NodeId source = cursor;
        Conversion c = list_element(cursor,t,s);
        if (!c.valid()) return false;
        args.push_back(source); selected.push_back(c);
        ListField item; item.field = field; item.type = t; item.index = index; item.count = count;
        fields.push_back(item); return true;
    };
    bool valid = true; Type target = types[to];
    unsigned rank = class_value(to) ? 5 : 0;
    if (target.kind == TypeKind::Array) {
        std::uint64_t index = 0;
        if (!target.bound) valid = false;
        while (valid && cursor && index < target.bound) valid = add(target.child,0,index++,1);
        if (valid && index < target.bound) valid = add(target.child,0,index,target.bound-index);
        for (auto c : selected) if (rank < c.rank) rank = c.rank;
    } else {
        size(to);
        for (auto d = scopes[entities[target.entity].scope].first_decl; valid && d; d = declarations[d].next) {
            EntityId field = declarations[d].entity;
            if (!nonstatic_field(field)) continue;
            valid = add(initialized_field_type(to,field),field,0,1);
            if (entities[target.entity].key == KW_UNION) break;
        }
    }
    store_call(plan.call,args,selected);
    plan.explicit_count = args.size(); plan.fields = list_fields.size();
    list_fields.insert(list_fields.end(),fields.begin(),fields.end());
    plan.state = 2; if (valid) plan.rank = rank;
    auto id = list_plans.size(); list_plans.push_back(plan); return id;
}
Conversion Analyzer::list_initialization(NodeId n, TypeId to, ScopeId s, bool direct)
{
    if (!s) s = facts[n].scope;
    auto k = n ? key(n,to) : key(to,s);
    Index& cache = !n ? empty_list_index : direct ? direct_list_index : list_index;
    auto id = cache.get(k);
    if (!id) {
        id = list_plans.size(); list_plans.emplace_back(); list_plans[id].state = 1; cache.put(k,id);
        ListPlan plan; plan.source = n; plan.target = to; plan.scope = s; plan.direct = direct;
        Type target = types[to];
        bool ref = target.kind == TypeKind::LRef || target.kind == TypeKind::RRef;
        TypeId t = value_type(to);
        NodeId first = n ? ast[n].first : 0;
        if (first && first == ast[n].last && ref && ast[first].kind != Kind::BracedInit) {
            expression(first,s);
            Conversion c = standard_conversion(expressions[first],to,first);
            if (c.valid() && c.reference && !c.temporary) {
                plan.direct_binding = true; plan.rank = c.rank;
                store_call(plan.call,{first},{c});
            }
        }
        TypeId qualified = t;
        while (types[qualified].kind == TypeKind::Array) qualified = types[qualified].child;
        bool can_bind = !ref || target.kind == TypeKind::RRef || types[qualified].cv == 1;
        if (!plan.direct_binding && can_bind) {
            if (aggregate_type(t)) {
                NodeId cursor = first;
                auto group = list_aggregate(cursor,t,s);
                plan = list_plans[group]; plan.source = n; plan.target = to; plan.direct = direct;
                if (cursor) plan.rank = 255;
            } else if (class_value(t)) {
                std::vector<NodeId> args;
                for (NodeId a = first; a; a = ast[a].next) { expression(a,s); args.push_back(a); }
                plan.constructor = choose_constructor(types.unqualified(t),args,&plan.call,s,true,true);
                plan.explicit_count = args.size();
                if (plan.constructor || plan.call.form == ExpressionForm::Overload) plan.rank = 5;
                if (plan.constructor) {
                    auto m = members[entities[plan.constructor].member_info];
                    plan.zero = args.empty() && m.synthetic && !m.defaulted_late;
                }
            } else if (!fundamental(t,FT_VOID) && types[t].kind != TypeKind::Function) {
                if (!first) plan.rank = 0;
                else if (first == ast[n].last) {
                    expression(first,s);
                    Conversion c = conversion(first,t);
                    plan.rank = c.rank; store_call(plan.call,{first},{c});
                }
            }
        }
        plan.state = 2; list_plans[id] = plan;
    }
    auto plan = list_plans[id];
    Conversion c; c.target = to; c.kind = Conversion::Kind::ListPlan; c.materialization = id;
    if (plan.state != 2) return c;
    c.rank = plan.rank; c.function = plan.constructor;
    c.reference = types[to].kind == TypeKind::LRef || types[to].kind == TypeKind::RRef;
    c.temporary = c.reference && !plan.direct_binding;
    if (c.reference) {
        c.qualification = types[value_type(to)].cv;
        c.preference = types[to].kind == TypeKind::LRef;
        if (plan.direct_binding) c.preference = conversions[plan.call.conversions].preference;
    }
    return c;
}
void Analyzer::prepare_list(NodeId n, Conversion& c)
{
    auto plan = list_plans[c.materialization];
    if (!c.valid()) throw std::runtime_error("invalid list initialization");
    TypeId t = value_type(c.target);
    if (plan.zero) prepare_zero_initialization(t);
    if (plan.constructor) {
        auto ctor = plan.constructor;
        if (deleted_transfer(ctor)) throw std::runtime_error("deleted list constructor");
        if (!plan.direct && members[entities[ctor].member_info].explicit_constructor)
            throw std::runtime_error("explicit constructor in copy-list initialization");
        check_access(ctor,plan.scope,entities[ctor].owner);
        demand_member(ctor); members[entities[ctor].member_info].complete_entry = true;
    } else if (class_value(t) && !plan.aggregate && !plan.direct_binding)
        throw std::runtime_error("ambiguous list constructor");
    ListObject object; object.plan = c.materialization;
    if (!plan.direct_binding && (c.reference || class_value(t) || types[t].kind == TypeKind::Array)) {
        object.temporary = make_entity(EntityKind::Variable,make_scope(ScopeKind::Block,plan.scope),0,n);
        entities[object.temporary].type = t; register_destruction(object.temporary);
    }
    std::vector<NodeId> args(call_arguments.begin()+plan.call.arguments,call_arguments.begin()+plan.call.arguments+plan.call.argument_count);
    std::vector<Conversion> selected(conversions.begin()+plan.call.conversions,conversions.begin()+plan.call.conversions+plan.call.count);
    for (unsigned j = 0; j < args.size(); ++j)
        if (args[j] && ast[args[j]].kind != Kind::BracedInit && j < plan.explicit_count)
            list_conversion(args[j],value_type(selected[j].target));
    if (!plan.aggregate && !plan.constructor && !args.empty() && ast[args[0]].kind != Kind::BracedInit)
        list_conversion(args[0],value_type(c.target));
    record_call(object.call,args,selected);
    if (plan.aggregate) {
        InitAction root; root.kind = InitKind::Group; root.type = t; root.source = plan.source;
        std::uint32_t tail = 0;
        for (unsigned j = 0; j < args.size(); ++j) {
            auto field = list_fields[plan.fields+j];
            InitAction item; item.kind = InitKind::Converted; item.type = field.type; item.field = field.field;
            item.index = field.index; item.count = field.count; item.source = args[j]; item.conversion = object.call.conversions+j;
            auto selected = conversions[item.conversion];
            if (class_value(item.type) && selected.kind == Conversion::Kind::Construction && conversion_objects[selected.materialization].elided) {
                item.helper_transfer = selected.function;
                item.helper_parameter = conversion_objects[selected.materialization].temporary;
                demand_member(item.helper_transfer);
                prepare_value_boundary(item.type);
            }
            auto next = initializers.size(); initializers.push_back(item);
            if (tail) initializers[tail].next = next; else root.first = next;
            tail = next;
        }
        object.initializer = initializers.size(); initializers.push_back(root);
    }
    c.materialization = list_objects.size(); c.kind = Conversion::Kind::List; list_objects.push_back(object);
}
} }
