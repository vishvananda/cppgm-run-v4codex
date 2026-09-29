#include "semantic/analyzer.h"
#include <stdexcept>
namespace cppgm { namespace semantic {
EntityId Analyzer::range_object(TypeId type, ScopeId scope, NodeId source)
{
    auto object = make_entity(EntityKind::Variable,scope,0,source);
    entities[object].type = type;
    register_destruction(object);
    return object;
}
Conversion Analyzer::prepare_typed_conversion(Expression source, Conversion c, ScopeId s, bool destination)
{
    check_fixed_conversion(source,0,c,s);
    if (c.kind == Conversion::Kind::Construction) {
        auto recipe = conversion_objects[c.materialization];
        use_selected_function(c.function,true);
        recipe.use = destination ? ConversionUse::Destination : ConversionUse::Temporary;
        if (!destination) recipe.temporary = range_object(value_type(c.target),s,0);
        recipe.elided = !c.reference && source.category == ValueCategory::Prvalue &&
            types.unqualified(source.type) == types.unqualified(c.target);
        for (unsigned i = 1; i < recipe.call.argument_count; ++i) default_argument(c.function,i);
        c.materialization = conversion_objects.size(); conversion_objects.push_back(recipe);
    } else if (c.kind == Conversion::Kind::User) {
        auto record = user_conversions[c.materialization];
        use_selected_function(c.function,!record.virtual_slot);
        auto returned = types[entities[c.function].type].child;
        Expression value; value.type = value_type(returned);
        value.category = types[returned].kind == TypeKind::LRef ? ValueCategory::Lvalue :
            types[returned].kind == TypeKind::RRef ? ValueCategory::Xvalue : ValueCategory::Prvalue;
        if (class_value(returned)) record.source_temporary = range_object(returned,s,0);
        record.result = prepare_typed_conversion(value,record.result,s,destination);
        record.prepared = true;
        c.materialization = user_conversions.size(); user_conversions.push_back(record);
    }
    return c;
}
void Analyzer::prepare_range_operation(RangeOperation& op, const std::vector<Expression>& args, ScopeId s)
{
    std::vector<Conversion> selected;
    for (unsigned i = 0; i < args.size(); ++i)
        selected.push_back(prepare_typed_conversion(args[i],conversions[op.result.conversions+i],s));
    // Default arguments are source expressions with their own recorded facts.
    for (unsigned i = args.size(); i < op.result.count; ++i) {
        if (!op.function) { selected.push_back(conversions[op.result.conversions+i]); continue; }
        auto parameter = i-op.receiver;
        Conversion c; default_argument(op.function,parameter,&c);
        selected.push_back(c);
    }
    op.supplied = args.size();
    op.result.conversions = conversions.size(); op.result.count = selected.size();
    conversions.insert(conversions.end(),selected.begin(),selected.end());
    if (op.function) {
        op.returned = types[entities[op.function].type].child;
        op.receiver = entities[op.function].member_info && !entities[op.function].is_static;
        if (op.receiver) {
            op.adjustment = base_steps(args[0].type,scopes[entities[op.function].owner].entity);
            op.virtual_slot = members[entities[op.function].member_info].virtual_slot;
        }
        use_selected_function(op.function,!op.virtual_slot);
    } else op.returned = op.result.type;
    if (class_value(op.result.type) && op.result.category == ValueCategory::Prvalue)
        op.temporary = range_object(op.result.type,s,0);
}
RangeOperation Analyzer::range_endpoint(Expression object, IdentifierId name, EntityId family, ScopeId naming, ScopeId s)
{
    RangeOperation op;
    std::vector<Expression> args;
    if (!naming) { family = associated_type_lookup(name,{object.type}); args.push_back(object); }
    std::vector<Conversion> selected;
    auto choice = select_call(family,args,0,naming ? object.type : 0,ValueCategory::Lvalue,naming,0,selected);
    if (choice.failure != CallFailure::None) throw std::runtime_error("invalid range endpoint");
    op.function = choice.entity;
    if (deleted_transfer(op.function)) throw std::runtime_error("deleted range endpoint");
    check_access(op.function,s,naming,object.type);
    require_deduced_return(op.function);
    op.receiver = naming && entities[op.function].member_info && !entities[op.function].is_static;
    if (op.receiver) args.insert(args.begin(),object);
    else if (naming) selected.erase(selected.begin()); // Static member has no receiver argument.
    auto f = types[entities[op.function].type];
    for (unsigned i = args.size()-op.receiver; i < f.count; ++i) {
        Conversion c; default_argument(op.function,i,&c,DefaultReason::Recipe); selected.push_back(c);
    }
    op.result.type = value_type(f.child);
    op.result.category = types[f.child].kind == TypeKind::LRef ? ValueCategory::Lvalue :
        types[f.child].kind == TypeKind::RRef ? ValueCategory::Xvalue : ValueCategory::Prvalue;
    op.result.conversions = conversions.size(); op.result.count = selected.size();
    conversions.insert(conversions.end(),selected.begin(),selected.end());
    prepare_range_operation(op,args,s); return op;
}
RangeOperation Analyzer::range_operator(ETokenType token, const std::vector<Expression>& args, ScopeId s)
{
    TypeQuery query; query.op = token; query.name = operator_name(token); query.context = s;
    auto ordinary = lookup(s,query.name);
    if (function_binding(ordinary)) query.entity = ordinary;
    std::vector<TypeQueryFact> children;
    for (auto value : args) { TypeQueryFact child; child.expression = value; children.push_back(child); }
    auto fact = query_operator(query,children);
    if (fact.failure != TypeQueryFact::Failure::None || !fact.expression.type)
        throw std::runtime_error("invalid range iterator operation");
    RangeOperation op; op.op = token; op.function = fact.selected; op.result = fact.expression;
    op.receiver = op.function && entities[op.function].member_info && !entities[op.function].is_static;
    prepare_range_operation(op,args,s); return op;
}
} }
