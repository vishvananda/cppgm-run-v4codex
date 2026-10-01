#include "semantic/analyzer.h"
#include <stdexcept>
namespace cppgm { namespace semantic {
using syntax::Kind;
Expression Analyzer::range_initializer(NodeId source, ScopeId s, bool pattern)
{
    if (ast[source].kind != Kind::BracedInit)
        return pattern ? template_statement_value(source,s) : expression(source,s);
    TypeId element = 0; std::uint64_t count = 0;
    if (!pattern) expand_expression_list(source,s);
    for (auto c = ast[source].first; c; c = ast[c].next) {
        if (pattern && ast[c].kind == Kind::PackExpression) return Expression();
        auto value = pattern ? template_statement_value(c,s) : expression(c,s);
        if (!value.type || dependent_type(value.type)) return Expression();
        auto t = types.unqualified(decay(value.type));
        if (element && element != t) throw std::runtime_error("conflicting range list types");
        element = t; ++count;
    }
    if (!element) throw std::runtime_error("empty range list cannot deduce element type");
    Expression result; result.type = types.compound(TypeKind::Array,types.qualify(element,1),count);
    return result;
}
RangePlan Analyzer::range_shape(Expression range, ScopeId s)
{
    RangePlan plan;
    auto save = [&](Expression from, Conversion c) {
        check_fixed_conversion(from,0,c,s);
        auto id = conversions.size(); conversions.push_back(c); return id;
    };
    if (types[range.type].kind == TypeKind::Array) {
        if (!types[range.type].bound) throw std::runtime_error("range requires bounded array");
        plan.array = range.type;
        plan.index_type = types.fundamental(types[range.type].bound <= 0x7fffffffULL ? FT_INT : FT_UNSIGNED_LONG_INT);
        plan.element.result.type = types[range.type].child; plan.element.result.category = ValueCategory::Lvalue;
    } else if (auto element = initializer_list_element(range.type)) {
        auto info = initializer_list_type(range.type);
        plan.list_element = element; plan.list_begin = info.begin; plan.list_size = info.size;
        plan.index_type = types.fundamental(FT_INT);
        plan.element.result.type = types.qualify(element,1); plan.element.result.category = ValueCategory::Lvalue;
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
        plan.first = range_endpoint(range,begin_name,first,naming,s);
        plan.last = range_endpoint(range,end_name,last,naming,s);
        auto iterator_type = types.unqualified(decay(plan.first.result.type));
        if (iterator_type != types.unqualified(decay(plan.last.result.type)))
            throw std::runtime_error("range begin and end deduce different types");
        plan.begin_conversion = save(plan.first.result,conversion_value(plan.first.result,iterator_type));
        plan.end_conversion = save(plan.last.result,conversion_value(plan.last.result,iterator_type));
        Expression iterator; iterator.type = iterator_type; iterator.category = ValueCategory::Lvalue;
        auto finish = iterator;
        plan.test = range_operator(OP_NE,{iterator,finish},s);
        plan.condition_conversion = save(plan.test.result,boolean_conversion_value(plan.test.result));
        plan.next = range_operator(OP_INC,{iterator},s);
        plan.element = range_operator(OP_STAR,{iterator},s);
    }
    return plan;
}
void Analyzer::resolve_range(NodeId n, ScopeId s)
{
    auto control = make_scope(ScopeKind::Control,s); facts.edit(n).scope = control;
    auto declaration = ast[n].first, specs = ast[declaration].first, d = ast[specs].next;
    RangePlan plan; plan.source = ast[ast[declaration].next].first; plan.body = ast[n].last;
    Expression range;
    range = range_initializer(plan.source,control);
    auto direct = plan.source;
    while (ast[direct].kind == Kind::Parenthesized) direct = ast[direct].first;
    bool direct_object = ast[direct].kind == Kind::IdExpression && range.entity &&
        !nonstatic_field(range.entity) && range.category == ValueCategory::Lvalue;
    if (direct_object) observe_scalar(direct); // Iteration uses the object's storage, even when const.
    // A captured declaration keeps its original entity identity, but its
    // storage is reached through this closure's environment. Bind a hidden
    // reference through the checked source expression instead of reusing the
    // enclosing function's declaration as a local range object.
    if (direct_object && !object_uses[expressions[direct].object_use].capture) plan.range = range.entity;
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
    auto source_id = ast.nodes.occurrences[n].source;
    auto retained = ast.nodes.occurrences[n].context ? range_pattern_index.get(source_id) : 0;
    auto shape = retained ? range_patterns[retained] : range_shape(range,control);
    if (retained) ++range_pattern_uses;
    plan.array = shape.array; plan.index_type = shape.index_type;
    plan.list_element = shape.list_element; plan.list_begin = shape.list_begin; plan.list_size = shape.list_size;
    plan.first = shape.first; plan.last = shape.last; plan.test = shape.test;
    plan.next = shape.next; plan.element = shape.element;
    if (!plan.array && !plan.list_element) {
        auto endpoint = [&](RangeOperation& op) {
            prepare_range_operation(op,op.supplied ? std::vector<Expression>{range} : std::vector<Expression>(),control,true);
        };
        endpoint(plan.first); endpoint(plan.last);
        auto iterator_type = types.unqualified(decay(plan.first.result.type));
        plan.begin = range_object(iterator_type,control,n); plan.end = range_object(iterator_type,control,n);
        plan.begin_conversion = save(prepare_typed_conversion(plan.first.result,conversions[shape.begin_conversion],control,true));
        plan.end_conversion = save(prepare_typed_conversion(plan.last.result,conversions[shape.end_conversion],control,true));
        Expression iterator; iterator.type = iterator_type; iterator.category = ValueCategory::Lvalue; iterator.entity = plan.begin;
        auto finish = iterator; finish.entity = plan.end;
        prepare_range_operation(plan.test,{iterator,finish},control,true);
        prepare_range_operation(plan.next,{iterator},control,true);
        prepare_range_operation(plan.element,{iterator},control,true);
        plan.condition_conversion = save(prepare_typed_conversion(plan.test.result,conversions[shape.condition_conversion],control));
    }
    element = plan.element.result;
    TypeId t;
    bool decomposition = child(d,Kind::BindingNames);
    if (decomposition) t = binding_object_type(specs,d,element);
    else if (spec_has(specs,KW_AUTO)) {
        unsigned cv = (spec_has(specs,KW_CONST) ? 1 : 0) | (spec_has(specs,KW_VOLATILE) ? 2 : 0);
        auto saved = deducing_placeholder; deducing_placeholder = true;
        TypeId pattern;
        try { pattern = declarator(d,types.qualify(placeholder_type(),cv),control); }
        catch (...) { deducing_placeholder = saved; throw; }
        deducing_placeholder = saved; TypeId deduced = 0;
        t = deduce_placeholder(pattern,element,deduced);
    } else t = declarator(d,specifiers(specs,control),control);
    auto selected = retained && shape.element_conversion ? conversions[shape.element_conversion] : conversion_value(element,t);
    auto c = prepare_typed_conversion(element,selected,control,true);
    plan.element_conversion = save(c);
    auto id = terminal(decl_name(d));
    plan.variable = make_entity(EntityKind::Variable,control,id,d);
    entities[plan.variable].type = t;
    if (!decomposition) bind(control,id,plan.variable);
    record(control,plan.variable,d,t,EntityKind::Variable);
    facts.edit(d).type = t; facts.edit(d).entity = plan.variable;
    register_destruction(plan.variable);
    if (decomposition) declare_bindings(d,control,plan.variable,false);
    if (types[t].kind == TypeKind::LRef || types[t].kind == TypeKind::RRef) {
        auto temporary = converted_temporary(c);
        if (!temporary && c.kind == Conversion::Kind::Standard && !c.temporary) temporary = plan.element.temporary;
        if (temporary) {
            reference_temporaries.put(plan.variable,temporary);
            object_destructors.put(plan.variable,object_destructor(temporary));
        }
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
    auto value = range_initializer(source,control,true);
    RangePlan shape; bool fixed = value.type && !dependent_type(value.type) && !pattern_class_type(value.type);
    if (fixed) {
        value.category = ValueCategory::Lvalue;
        shape = range_shape(value,control);
    }
    bool placeholder = spec_has(specs,KW_AUTO);
    auto t = placeholder ? 0 : bind_template_type(specs,d,control);
    bool decomposition = child(d,Kind::BindingNames);
    if (decomposition) t = binding_object_type(specs,d,fixed ? shape.element.result : Expression());
    else if (placeholder && fixed) {
        unsigned cv = (spec_has(specs,KW_CONST) ? 1 : 0) | (spec_has(specs,KW_VOLATILE) ? 2 : 0);
        auto saved = deducing_placeholder; deducing_placeholder = true;
        try {
            auto pattern = declarator(d,types.qualify(placeholder_type(),cv),control);
            TypeId deduced = 0; t = deduce_placeholder(pattern,shape.element.result,deduced);
        } catch (...) { deducing_placeholder = saved; throw; }
        deducing_placeholder = saved;
    }
    if (fixed && t && !dependent_type(t)) {
        auto c = conversion_value(shape.element.result,t);
        check_fixed_conversion(shape.element.result,0,c,control);
        shape.element_conversion = conversions.size(); conversions.push_back(c);
    }
    if (fixed) {
        range_pattern_index.put(ast.nodes.occurrences[n].source,range_patterns.size());
        range_patterns.push_back(shape);
    }
    auto e = pattern_declaration(EntityKind::Variable,control,terminal(decl_name(d)),d,!t || dependent_type(t));
    entities[e].type = t; facts.edit(d).type = t; facts.edit(d).entity = e;
    if (decomposition) declare_bindings(d,control,e,true);
    ++loop_depth;
    bind_template_statement(ast[n].last,control);
    --loop_depth;
}
} }
