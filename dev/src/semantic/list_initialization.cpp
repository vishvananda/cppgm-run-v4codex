#include "semantic/analyzer.h"
#include <stdexcept>
namespace cppgm { namespace semantic {
using syntax::Kind;
Conversion Analyzer::list_element(NodeId& cursor, TypeId to, ScopeId s)
{
    if (!cursor) return list_initialization(0,to,s);
    if (ast.kind(cursor) == Kind::DesignatedInit) return Conversion();
    NodeId source = cursor;
    expression(source,s);
    Conversion c = conversion(source,to);
    if (string_initialization(source,to)) {
        ListPlan plan; plan.source = source; plan.target = to; plan.scope = s;
        plan.aggregate = true; plan.literal = ast.literal(source); plan.state = FactState::Success;
        if (types[to].bound && ast.literals[plan.literal].elements <= types[to].bound) plan.rank = 0;
        c.target = to; c.kind = Conversion::Kind::ListPlan; c.rank = plan.rank;
        c.materialization = list_plans.size(); list_plans.push_back(plan);
    }
    if (c.valid()) { cursor = ast.next(source); return c; }
    if (ast.kind(source) != Kind::BracedInit && aggregate_type(to)) {
        auto id = list_aggregate(cursor,to,s);
        c.target = to; c.kind = Conversion::Kind::ListPlan; c.materialization = id;
        c.rank = list_plans[id].rank;
    }
    return c;
}
std::uint32_t Analyzer::list_aggregate(NodeId& cursor, TypeId to, ScopeId s)
{
    ListPlan plan; plan.source = cursor; plan.target = to; plan.scope = s; plan.aggregate = true;
    plan.zero = !cursor;
    std::vector<NodeId> args;
    std::vector<Conversion> selected;
    std::vector<ListField> fields;
    bool designated_operand = false;
    auto add = [&](TypeId t, EntityId field, std::uint64_t index, std::uint64_t count) {
        NodeId source = cursor;
        Conversion c;
        if (ast.kind(cursor) == Kind::DesignatedInit && designated_storage(field,ast.text(cursor))) {
            c.kind = Conversion::Kind::ListPlan; c.target = t;
            c.materialization = list_aggregate(cursor,t,s); c.rank = list_plans[c.materialization].rank;
        } else if (designated_operand && cursor && ast.kind(cursor) != Kind::BracedInit &&
                   aggregate_type(t) && !string_initialization(cursor,t)) {
            expression(cursor,s); c = conversion(cursor,t);
            if (c.valid()) cursor = ast.next(cursor);
        } else c = list_element(cursor,t,s);
        if (!c.valid()) return false;
        if (source || c.kind != Conversion::Kind::ListPlan) plan.zero = false;
        else {
            auto nested = list_plans[c.materialization];
            plan.zero &= !nested.constructor && !nested.backing_element &&
                (nested.aggregate ? nested.zero : !nested.call.argument_count) && zero_value(t);
        }
        args.push_back(source); selected.push_back(c);
        ListField item; item.field = field; item.type = t; item.index = index; item.count = count;
        fields.push_back(item); return true;
    };
    bool valid = true; Type target = types[to];
    unsigned rank = class_value(to) ? 5 : 0;
    if (vector_kind(target.kind)) target.bound = vector_elements(to);
    if (complex_type(to)) { target.child = complex_component(to); target.bound = 2; }
    if (target.kind == TypeKind::Array || vector_kind(target.kind) || complex_type(to)) {
        std::uint64_t index = 0;
        while (valid && cursor && (target.unknown_bound || index < target.bound)) {
            auto before = cursor;
            valid = add(target.child,0,index++,1) && cursor != before;
        }
        if (target.unknown_bound) {
            if (!index) valid = false;
            else plan.target = types.compound(TypeKind::Array,target.child,index);
        }
        if (valid && index < target.bound) valid = add(target.child,0,index,target.bound-index);
        for (auto c : selected) if (rank < c.rank) rank = c.rank;
    } else {
        size(to);
        for (auto d = scopes[entities[target.entity].scope].first_decl; valid && d; d = declarations[d].next) {
            EntityId field = declarations[d].entity;
            if (!nonstatic_field(field)) continue;
            auto designation = ast.kind(cursor) == Kind::DesignatedInit ? cursor : 0;
            if (designation && designated_storage(field,ast.text(designation))) designation = 0;
            if (designation) {
                bool match = ast.text(designation) == entities[field].name;
                if (!match && entities[target.entity].key == KW_UNION) continue;
                cursor = match ? ast.first(designation) : 0;
            }
            designated_operand = designation && cursor;
            valid = add(initialized_field_type(to,field),field,0,1);
            if (designation) {
                valid &= !cursor;
                cursor = ast.text(designation) == entities[field].name ? ast.next(designation) : designation;
            }
            if (entities[target.entity].key == KW_UNION) break;
        }
    }
    store_call(plan.call,args,selected);
    plan.explicit_count = args.size(); plan.fields = list_fields.size();
    list_fields.insert(list_fields.end(),fields.begin(),fields.end());
    plan.state = FactState::Success; if (valid) plan.rank = rank;
    auto id = list_plans.size(); list_plans.push_back(plan); return id;
}
Conversion Analyzer::list_initialization(NodeId n, TypeId to, ScopeId s, bool direct)
{
    if (!s) s = facts[n].scope;
    auto k = n ? key(n,to) : key(to,s);
    Index& cache = !n ? (direct ? empty_direct_list_index : empty_list_index) : direct ? direct_list_index : list_index;
    auto id = cache.get(k);
    if (!id) {
        id = list_plans.size(); list_plans.emplace_back(); list_plans[id].state = FactState::Active; cache.put(k,id);
        try {
        ListPlan plan; plan.source = n; plan.target = to; plan.scope = s; plan.direct = direct;
        Type target = types[to];
        bool ref = target.kind == TypeKind::LRef || target.kind == TypeKind::RRef;
        TypeId t = value_type(to);
        NodeId first = n ? ast.first(n) : 0;
        if (first && first == ast.last(n) && ref && ast.kind(first) != Kind::BracedInit && ast.kind(first) != Kind::DesignatedInit) {
            expression(first,s);
            Conversion c = standard_conversion(expressions[first],to,first);
            if (c.valid() && c.reference && !c.temporary) {
                plan.direct_binding = true; plan.rank = c.rank;
                store_call(plan.call,{first},{c});
            }
        }
        TypeId qualified = t;
        while (types[qualified].kind == TypeKind::Array) qualified = types[qualified].child;
        bool can_bind = !ref || target.kind == TypeKind::RRef || (types[qualified].cv & 3) == 1;
        if (!plan.direct_binding && can_bind) {
            if (class_value(t)) {
                complete_class(types[t].entity);
                if (!entities[types[t].entity].complete)
                    throw UnavailableSemanticFact(SemanticFact::ClassDefinition,types[t].entity,n);
            }
            bool designated = false;
            for (auto a = first; a; a = ast.next(a)) designated |= ast.kind(a) == Kind::DesignatedInit;
            if (designated && !aggregate_type(t)) {
                // In overload formation this is an invalid candidate, not an
                // expression error that can prevent another aggregate match.
            } else if (first && first == ast.last(n) && string_initialization(first,t)) {
                plan.source = first; plan.literal = ast.literal(first); plan.aggregate = true;
                if (!ref && types[t].unknown_bound) t = to = types.compound(TypeKind::Array,types[t].child,ast.literals[plan.literal].elements);
                plan.target = to;
                if (types[t].bound && ast.literals[plan.literal].elements <= types[t].bound) plan.rank = 0;
            } else if (auto element = initializer_list_element(t)) {
                auto info = initializer_list_type(t); plan.backing_begin = info.begin; plan.backing_size = info.size;
                plan.backing_element = types.qualify(element,1); plan.rank = 0;
                std::vector<NodeId> args; std::vector<Conversion> selected;
                for (auto a = first; a; a = ast.next(a)) {
                    expression(a,s); auto c = conversion(a,plan.backing_element);
                    if (c.rank > plan.rank) plan.rank = c.rank;
                    args.push_back(a); selected.push_back(c);
                }
                store_call(plan.call,args,selected); plan.explicit_count = args.size();
            } else if (aggregate_type(t) || vector_kind(types[t].kind) || (complex_type(t) && first != ast.last(n))) {
                NodeId cursor = first;
                auto group = list_aggregate(cursor,t,s);
                if (!ref) t = to = list_plans[group].target;
                plan = list_plans[group]; plan.source = n; plan.target = to; plan.direct = direct;
                if (cursor) plan.rank = 255;
            } else if (class_value(t)) {
                std::vector<NodeId> args;
                for (NodeId a = first; a; a = ast.next(a)) { expression(a,s); args.push_back(a); }
                if (args.empty()) plan.constructor = choose_constructor(types.unqualified(t),{},&plan.call,s,true,true);
                if (!plan.constructor && plan.call.form != ExpressionForm::Overload)
                    plan.constructor = choose_constructor(types.unqualified(t),{n},&plan.call,s,true,true,0,false,true);
                if (!plan.constructor && plan.call.form != ExpressionForm::Overload)
                    plan.constructor = choose_constructor(types.unqualified(t),args,&plan.call,s,true,true);
                plan.explicit_count = plan.call.argument_count;
                if (plan.constructor || plan.call.form == ExpressionForm::Overload) plan.rank = 5;
                if (plan.constructor) {
                    auto m = members[entities[plan.constructor].member_info];
                    plan.zero = args.empty() && m.synthetic && !m.defaulted_late;
                }
            } else if (!fundamental(t,FT_VOID) && types[t].kind != TypeKind::Function) {
                if (!first) plan.rank = 0;
                else if (first == ast.last(n)) {
                    expression(first,s);
                    Conversion c = conversion(first,t);
                    plan.rank = c.rank; store_call(plan.call,{first},{c});
                }
            }
        }
        plan.state = FactState::Success; list_plans[id] = plan;
        } catch (...) { list_plans[id].state = FactState::Failure; throw; }
    }
    auto plan = list_plans[id];
    to = plan.target;
    if (plan.state == FactState::Failure) throw FailedSemanticFact(SemanticFact::ListInitialization,0,n);
    Conversion c; c.target = to; c.kind = Conversion::Kind::ListPlan; c.materialization = id;
    if (plan.state != FactState::Success) return c;
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
void Analyzer::validate_list_plan(std::uint32_t id)
{
    auto plan = list_plans[id];
    if (plan.validation == FactState::Success) return;
    if (plan.validation == FactState::Failure)
        throw FailedSemanticFact(SemanticFact::ListConversion,plan.constructor,plan.source);
    if (plan.validation == FactState::Active) throw std::logic_error("recursive list validation");
    list_plans[id].validation = FactState::Active;
    try {
        if (plan.state != FactState::Success || plan.rank == 255) throw std::runtime_error("invalid list initialization");
        TypeId t = value_type(plan.target);
        if (plan.constructor) {
            auto ctor = plan.constructor;
            if (deleted_transfer(ctor)) throw std::runtime_error("deleted list constructor");
            if (!plan.direct && members[entities[ctor].member_info].explicit_constructor)
                throw std::runtime_error("explicit constructor in copy-list initialization");
            check_access(ctor,plan.scope,entities[ctor].owner);
            auto signature = types[entities[ctor].type];
            if (plan.call.argument_count < signature.count) {
                // Candidate ranking only inspected supplied arguments. Once
                // selected, complete this immutable call recipe's defaults.
                std::vector<NodeId> args; std::vector<Conversion> chosen;
                for (unsigned i = 0; i < signature.count; ++i) {
                    Conversion c;
                    auto n = i < plan.call.argument_count ? call_argument(plan.call,i) :
                        default_argument(ctor,i,&c,DefaultReason::Recipe);
                    if (i < plan.call.argument_count) c = conversions[plan.call.conversions+i];
                    args.push_back(n); chosen.push_back(c);
                }
                store_call(plan.call,args,chosen); list_plans[id].call = plan.call;
            }
        } else if (class_value(t) && !plan.aggregate && !plan.direct_binding && !plan.backing_element)
            throw std::runtime_error("ambiguous list constructor");
        if (!plan.direct_binding) default_destructor(t,plan.scope,false);
        for (unsigned i = 0; i < plan.call.argument_count; ++i) {
            auto n = call_argument(plan.call,i);
            auto c = conversions[plan.call.conversions+i];
            if (n && ast.kind(n) != Kind::BracedInit && i < plan.explicit_count)
                list_conversion_from(n,expressions[n].type,value_type(c.target),&c);
            if (!i && !plan.aggregate && !plan.constructor && n && ast.kind(n) != Kind::BracedInit)
                list_conversion_from(n,expressions[n].type,value_type(plan.target),&c);
            // Selected constructor defaults were checked in their own
            // declaration environment, not this list's calling scope.
            if (!plan.constructor || i < plan.explicit_count)
                check_fixed_conversion(expressions[n],n,c,plan.scope);
            conversions[plan.call.conversions+i] = c;
        }
        list_plans[id].validation = FactState::Success;
    } catch (...) {
        list_plans[id].validation = FactState::Failure; throw;
    }
}
void Analyzer::prepare_list(NodeId n, Conversion& c)
{
    validate_list_plan(c.materialization);
    auto plan = list_plans[c.materialization];
    TypeId t = value_type(c.target);
    if (plan.zero) prepare_zero_initialization(t);
    if (plan.constructor) {
        demand_member(plan.constructor); members[entities[plan.constructor].member_info].complete_entry = true;
        for (unsigned i = plan.explicit_count; i < plan.call.argument_count; ++i) default_argument(plan.constructor,i);
    }
    ListObject object; object.plan = c.materialization;
    if (plan.backing_element && plan.call.argument_count) {
        object.backing = make_entity(EntityKind::Variable,make_scope(ScopeKind::Block,plan.scope),0,n);
        entities[object.backing].type = types.compound(TypeKind::Array,plan.backing_element,plan.call.argument_count);
        register_destruction(object.backing);
    }
    if (!plan.direct_binding && (c.reference || class_value(t) || types[t].kind == TypeKind::Array || vector_kind(types[t].kind) || (complex_type(t) && plan.aggregate))) {
        object.temporary = make_entity(EntityKind::Variable,make_scope(ScopeKind::Block,n ? facts[n].scope : plan.scope),0,n);
        entities[object.temporary].type = t; register_destruction(object.temporary);
    }
    std::vector<NodeId> args;
    for (unsigned i = 0; i < plan.call.argument_count; ++i) {
        auto a = call_argument(plan.call,i);
        // Project source recipes once. A concrete pack lane already carries
        // its own substitution frame and must not be rebound to the outer list.
        if (n && ast.nodes.occurrences[n].context && !ast.nodes.occurrences[a].context) {
            auto projected = ast.projected(a,ast.nodes.occurrences[n].context);
            if (projected) a = projected;
        }
        if (a && ast.kind(a) != Kind::DesignatedInit) expression(a,n ? facts[n].scope : plan.scope);
        args.push_back(a);
    }
    std::vector<Conversion> selected;
    for (unsigned i = 0; i < plan.call.count; ++i)
        selected.push_back(copy_conversion_recipe(conversions[plan.call.conversions+i]));
    record_call(object.call,args,selected);
    if (plan.literal) {
        InitAction root; root.kind = InitKind::String; root.type = t; root.source = plan.source;
        object.initializer = initializers.size(); initializers.push_back(root);
    } else if (plan.aggregate || plan.backing_element) {
        InitAction root; root.kind = InitKind::Group; root.type = plan.backing_element ? entities[object.backing].type : t; root.source = plan.source;
        std::uint32_t tail = 0;
        for (unsigned j = 0; j < args.size(); ++j) {
            ListField field;
            if (plan.backing_element) { field.type = plan.backing_element; field.index = j; }
            else field = list_fields[plan.fields+j];
            InitAction item; item.kind = InitKind::Converted; item.type = field.type; item.field = field.field;
            item.index = field.index; item.count = field.count; item.source = args[j]; item.conversion = object.call.conversions+j;
            auto selected = conversions[item.conversion];
            if (selected.kind == Conversion::Kind::List) {
                auto nested = list_objects[selected.materialization];
                item.helper_safe = nested.initializer ? initializers[nested.initializer].helper_safe :
                    !nested.call.argument_count && !class_value(item.type);
                auto constructor = list_plans[nested.plan].constructor;
                // This transports a representation, not an extra language
                // copy/move. Prove that early construction cannot observe the
                // destination or escape its temporary address, and that no
                // destructor or transfer side effects are introduced.
                if (!plan.backing_element && class_value(item.type) && nested.temporary && !(types[item.type].cv & 2) &&
                    constructor && independent_constructor(constructor) &&
                    copy_storage_type(item.type) && trivial_destructor(item.type)) {
                    bool safe = true;
                    for (unsigned i = 0; safe && i < nested.call.argument_count; ++i) {
                        auto c = conversions[nested.call.conversions+i];
                        safe = c.kind == Conversion::Kind::Standard && !c.function &&
                            independent_initializer(call_argument(nested.call,i));
                    }
                    if (safe) {
                        item.helper_safe = item.helper_copy = true;
                        prepare_value_boundary(item.type);
                    }
                }
            }
            else item.helper_safe = (selected.kind == Conversion::Kind::Standard || selected.kind == Conversion::Kind::Explicit) &&
                !selected.function && independent_initializer(item.source);
            // An omitted class member is initialized in place by the helper,
            // after its preceding fields. No argument evaluation is hoisted.
            if (!item.source && class_value(item.type) && selected.kind == Conversion::Kind::List)
                item.helper_safe = true;
            // Backing elements are constructed in their final array slots.
            // Only aggregate helpers transport a class argument across an
            // extra ABI boundary and need its otherwise elided transfer.
            if (!plan.backing_element && class_value(item.type) && selected.kind == Conversion::Kind::Construction && conversion_objects[selected.materialization].elided) {
                item.helper_transfer = selected.function;
                item.helper_parameter = conversion_objects[selected.materialization].temporary;
                demand_member(item.helper_transfer);
                prepare_value_boundary(item.type);
                item.helper_safe = independent_materialization(item.source);
                item.helper_commutes = function_nonthrowing(item.helper_transfer) &&
                    conversion_objects[selected.materialization].call.argument_count == 1 &&
                    independent_constructor(item.helper_transfer,true);
            }
            root.helper_safe &= item.helper_safe;
            auto next = initializers.size(); initializers.push_back(item);
            if (tail) initializers[tail].next = next; else root.first = next;
            tail = next;
        }
        object.initializer = initializers.size(); initializers.push_back(root);
    }
    c.materialization = list_objects.size(); c.kind = Conversion::Kind::List; list_objects.push_back(object);
}
} }
