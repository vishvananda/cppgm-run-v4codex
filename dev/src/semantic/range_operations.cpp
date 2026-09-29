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
    // A checked constructor recipe already owns the selected argument conversions.
    // Applying it creates storage and demand facts, without resolving it again.
    if (!(c.kind == Conversion::Kind::Construction && c.materialization &&
          conversion_objects[c.materialization].use == ConversionUse::Recipe))
        check_fixed_conversion(source,0,c,s);
    if (c.kind == Conversion::Kind::Construction) {
        destination &= !c.reference;
        auto recipe = conversion_objects[c.materialization];
        use_selected_function(c.function,true);
        recipe.use = destination ? ConversionUse::Destination : ConversionUse::Temporary;
        if (!destination) recipe.temporary = range_object(value_type(c.target),s,0);
        recipe.elided = !c.reference && source.category == ValueCategory::Prvalue &&
            types.unqualified(source.type) == types.unqualified(c.target);
        std::vector<Conversion> arguments;
        if (recipe.call.argument_count) arguments.push_back(conversions[recipe.call.conversions]);
        for (unsigned i = 1; i < recipe.call.argument_count; ++i) {
            Conversion argument;
            auto node = default_argument(c.function,i,&argument);
            apply_conversion(node,argument); arguments.push_back(argument);
        }
        recipe.call.conversions = conversions.size();
        conversions.insert(conversions.end(),arguments.begin(),arguments.end());
        members[entities[c.function].member_info].complete_entry = true;
        c.materialization = conversion_objects.size(); conversion_objects.push_back(recipe);
    } else if (c.kind == Conversion::Kind::User) {
        auto record = user_conversions[c.materialization];
        use_selected_function(c.function,!record.virtual_slot);
        auto returned = types[entities[c.function].type].child;
        Expression value; value.type = value_type(returned);
        value.category = types[returned].kind == TypeKind::LRef ? ValueCategory::Lvalue :
            types[returned].kind == TypeKind::RRef ? ValueCategory::Xvalue : ValueCategory::Prvalue;
        bool direct = destination && !c.reference && record.result.kind == Conversion::Kind::Standard &&
            types.unqualified(returned) == types.unqualified(c.target);
        if (class_value(returned) && !direct) record.source_temporary = range_object(returned,s,0);
        record.result = prepare_typed_conversion(value,record.result,s,destination);
        if (c.reference) {
            record.temporary = converted_temporary(record.result);
            if (!record.temporary && record.result.reference && !record.result.temporary)
                record.temporary = record.source_temporary;
        }
        record.prepared = true;
        c.materialization = user_conversions.size(); user_conversions.push_back(record);
    }
    return c;
}
void Analyzer::prepare_range_operation(RangeOperation& op, const std::vector<Expression>& args, ScopeId s, bool evaluated)
{
    if (evaluated) {
        std::vector<Conversion> selected;
        for (unsigned i = 0; i < args.size(); ++i)
            selected.push_back(prepare_typed_conversion(args[i],conversions[op.result.conversions+i],s));
        // Default arguments retain their own source expression facts. The
        // extra builtin increment entry instead records its computation type.
        for (unsigned i = args.size(); i < op.result.count; ++i) {
            if (!op.function) { selected.push_back(conversions[op.result.conversions+i]); continue; }
            Conversion c; default_argument(op.function,i-op.receiver,&c);
            selected.push_back(c);
        }
        op.result.conversions = conversions.size(); op.result.count = selected.size();
        conversions.insert(conversions.end(),selected.begin(),selected.end());
    }
    op.supplied = args.size();
    if (op.function) {
        op.returned = types[entities[op.function].type].child;
        op.receiver = entities[op.function].member_info && !entities[op.function].is_static;
        if (op.receiver) {
            op.adjustment = base_steps(args[0].type,scopes[entities[op.function].owner].entity);
            op.virtual_slot = members[entities[op.function].member_info].virtual_slot;
        }
        if (evaluated) use_selected_function(op.function,!op.virtual_slot);
    } else op.returned = op.result.type;
    if (class_value(op.result.type) && op.result.category == ValueCategory::Prvalue) {
        reject_abstract(op.result.type); default_destructor(op.result.type,s,evaluated);
        if (evaluated) op.temporary = range_object(op.result.type,s,0);
    }
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
    for (unsigned i = 0; i < args.size(); ++i) check_fixed_conversion(args[i],0,selected[i],s);
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
