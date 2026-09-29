#include "semantic/analyzer.h"
#include <stdexcept>
namespace cppgm { namespace semantic {
using syntax::Kind;
void Analyzer::resolve_range(NodeId n, ScopeId s)
{
    auto control = make_scope(ScopeKind::Control,s); facts.edit(n).scope = control;
    auto declaration = ast[n].first, specs = ast[declaration].first, d = ast[specs].next;
    RangePlan plan; plan.source = ast[ast[declaration].next].first; plan.body = ast[n].last;
    Expression range;
    if (ast[plan.source].kind == Kind::BracedInit) {
        TypeId element = 0; std::uint64_t count = 0;
        expand_expression_list(plan.source,control);
        for (auto c = ast[plan.source].first; c; c = ast[c].next) {
            auto t = types.unqualified(decay(expression(c,control).type));
            if (element && element != t) throw std::runtime_error("conflicting range list types");
            element = t; ++count;
        }
        if (!element) throw std::runtime_error("empty range list cannot deduce element type");
        range.type = types.compound(TypeKind::Array,types.qualify(element,1),count);
    } else range = expression(plan.source,control);
    auto direct = plan.source;
    while (ast[direct].kind == Kind::Parenthesized) direct = ast[direct].first;
    if (ast[direct].kind == Kind::IdExpression && range.entity &&
        !nonstatic_field(range.entity) && range.category == ValueCategory::Lvalue) plan.range = range.entity;
    else {
        // A class/array prvalue is constructed directly in its lifetime-extended
        // storage. Other glvalues bind a hidden reference, evaluated only once.
        auto t = range.category == ValueCategory::Prvalue &&
            (types[range.type].kind != TypeKind::Array || ast[plan.source].kind == Kind::BracedInit) ? range.type :
            types.compound(range.category == ValueCategory::Lvalue ? TypeKind::LRef : TypeKind::RRef,range.type);
        plan.range = make_entity(EntityKind::Variable,control,0,plan.source);
        entities[plan.range].type = t; entities[plan.range].initializer = plan.source;
        initialize(plan.source,t,control); register_destruction(plan.range);
        plan.initialize_range = true;
    }
    range.entity = plan.range; range.category = ValueCategory::Lvalue;
    Expression element;
    auto save = [&](Conversion c) { auto id = conversions.size(); conversions.push_back(c); return id; };
    if (types[range.type].kind == TypeKind::Array) {
        if (!types[range.type].bound) throw std::runtime_error("range requires bounded array");
        plan.array = range.type;
        plan.index_type = types.fundamental(types[range.type].bound <= 0x7fffffffULL ? FT_INT : FT_UNSIGNED_LONG_INT);
        element.type = types[range.type].child; element.category = ValueCategory::Lvalue;
    } else {
        ScopeId naming = 0; EntityId first = 0, last = 0;
        auto begin_name = ids.intern(TextView("begin",5)), end_name = ids.intern(TextView("end",3));
        if (class_value(range.type)) {
            complete_class(types[range.type].entity);
            naming = entities[types[range.type].entity].scope;
            first = lookup(naming,begin_name,Lookup::Ordinary,true);
            last = lookup(naming,end_name,Lookup::Ordinary,true);
            // C++11 range lookup selects member syntax if either name occurs.
            if (!first && !last) naming = 0;
        }
        plan.first = range_endpoint(range,begin_name,first,naming,control);
        plan.last = range_endpoint(range,end_name,last,naming,control);
        auto iterator_type = types.unqualified(decay(plan.first.result.type));
        if (iterator_type != types.unqualified(decay(plan.last.result.type)))
            throw std::runtime_error("range begin and end deduce different types");
        plan.begin = range_object(iterator_type,control,n); plan.end = range_object(iterator_type,control,n);
        plan.begin_conversion = save(prepare_typed_conversion(plan.first.result,conversion_value(plan.first.result,iterator_type),control,true));
        plan.end_conversion = save(prepare_typed_conversion(plan.last.result,conversion_value(plan.last.result,iterator_type),control,true));
        Expression iterator; iterator.type = iterator_type; iterator.category = ValueCategory::Lvalue; iterator.entity = plan.begin;
        auto finish = iterator; finish.entity = plan.end;
        plan.test = range_operator(OP_NE,{iterator,finish},control);
        plan.condition_conversion = save(prepare_typed_conversion(plan.test.result,boolean_conversion_value(plan.test.result),control));
        plan.next = range_operator(OP_INC,{iterator},control);
        plan.element = range_operator(OP_STAR,{iterator},control); element = plan.element.result;
    }
    TypeId t;
    if (spec_has(specs,KW_AUTO)) {
        unsigned cv = (spec_has(specs,KW_CONST) ? 1 : 0) | (spec_has(specs,KW_VOLATILE) ? 2 : 0);
        auto saved = deducing_placeholder; deducing_placeholder = true;
        TypeId pattern;
        try { pattern = declarator(d,types.qualify(placeholder_type(),cv),control); }
        catch (...) { deducing_placeholder = saved; throw; }
        deducing_placeholder = saved; TypeId deduced = 0;
        t = deduce_placeholder(pattern,element,deduced);
    } else t = declarator(d,specifiers(specs,control),control);
    auto c = prepare_typed_conversion(element,conversion_value(element,t),control,true);
    plan.element_conversion = save(c);
    auto id = terminal(decl_name(d));
    plan.variable = make_entity(EntityKind::Variable,control,id,d);
    entities[plan.variable].type = t;
    bind(control,id,plan.variable); record(control,plan.variable,d,t,EntityKind::Variable);
    facts.edit(d).type = t; facts.edit(d).entity = plan.variable;
    register_destruction(plan.variable);
    if ((types[t].kind == TypeKind::LRef || types[t].kind == TypeKind::RRef) && plan.element.temporary &&
        c.kind == Conversion::Kind::Standard && !c.temporary) {
        reference_temporaries.put(plan.variable,plan.element.temporary);
        object_destructors.put(plan.variable,object_destructor(plan.element.temporary));
    }
    auto index = ranges.size(); ranges.push_back(plan); range_index.put(n,index);
    ++loop_depth;
    resolve_statement(plan.body,ast[plan.body].kind == Kind::Compound ? control : make_scope(ScopeKind::Block,control));
    --loop_depth;
}
void Analyzer::bind_template_range(NodeId n, ScopeId s)
{
    auto control = make_scope(ScopeKind::Control,s,0,0,false);
    template_pattern_scopes.put(control,1); facts.edit(n).scope = control;
    auto declaration = ast[n].first, specs = ast[declaration].first, d = ast[specs].next;
    auto source = ast[ast[declaration].next].first;
    bind_template_expression(source,control);
    bool placeholder = spec_has(specs,KW_AUTO);
    auto t = placeholder ? 0 : bind_template_type(specs,d,control);
    auto e = pattern_declaration(EntityKind::Variable,control,terminal(decl_name(d)),d,!t || dependent_type(t));
    entities[e].type = t; facts.edit(d).type = t; facts.edit(d).entity = e;
    ++loop_depth;
    bind_template_statement(ast[n].last,control);
    --loop_depth;
}
} }
