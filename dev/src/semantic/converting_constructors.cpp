#include "semantic/analyzer.h"
#include <stdexcept>

namespace cppgm { namespace semantic {
Conversion Analyzer::converting_constructor(NodeId n, TypeId target)
{
    return converting_constructor_value(expressions[n],target,n);
}
Conversion Analyzer::converting_constructor_value(Expression source, TypeId target, NodeId n)
{
    Conversion result; result.target = target;
    EntityId cls = types[target].entity;
    complete_class(cls);
    struct Candidate { EntityId entity; Conversion argument; };
    std::vector<Candidate> viable;
    for (EntityId e : candidates(class_facts[entities[cls].class_info].constructor)) {
        if (!e) continue;
        ++candidate_work;
        if (entities[e].template_info) {
            e = deduce_function(e,std::vector<Expression>{source});
            if (!e) continue;
        }
        auto m = members[entities[e].member_info];
        Type f = types[entities[e].type];
        if (m.explicit_constructor || (!f.count && !f.variadic) || (f.count > 1 &&
            (!entities[e].defaults || !default_arguments[entities[e].defaults+1]))) continue;
        Conversion argument = f.count ? (n ? conversion(n, types.parameters[f.offset], false) :
            standard_conversion(source,types.parameters[f.offset])) : ellipsis_conversion_value(source);
        if (argument.valid()) viable.push_back({e, argument});
    }
    if (viable.empty()) return result;
    auto preferred = [&](std::size_t a, std::size_t b) {
        auto x = &viable[a].argument, y = &viable[b].argument;
        if (better(x,y,1)) return true;
        if (better(y,x,1)) return false;
        auto ea = viable[a].entity, eb = viable[b].entity;
        return (!entities[ea].specialization && entities[eb].specialization) || template_more_specialized(ea,eb,1);
    };
    std::size_t best = 0;
    for (std::size_t j = 1; j < viable.size(); ++j)
        if (preferred(j,best)) best = j;
    for (std::size_t j = 0; j < viable.size(); ++j)
        if (j != best && !preferred(best,j)) {
            result.ambiguous = true; return result;
        }
    result.kind = Conversion::Kind::Construction; result.rank = 5;
    result.function = viable[best].entity;
    return result;
}
void Analyzer::materialize_conversion(NodeId n, Conversion& conversion, bool defer, ConversionUse use)
{
    auto recipe = conversion.materialization;
    if (recipe && conversion_objects[recipe].use != ConversionUse::Recipe) return;
    if (use == ConversionUse::Recipe || (use == ConversionUse::Destination && conversion.reference))
        throw std::logic_error("invalid conversion destination");
    EntityId ctor = conversion.function;
    auto m = entities[ctor].member_info;
    if (deleted_transfer(ctor)) throw std::runtime_error("deleted converting constructor");
    check_access(ctor, facts[n].scope, entities[ctor].owner);
    TypeId t = value_type(conversion.target);
    ConversionObject materialized; materialized.constructor = ctor; materialized.use = use;
    if (use == ConversionUse::Temporary) {
        EntityId object = make_entity(EntityKind::Variable, make_scope(ScopeKind::Block, facts[n].scope), 0, n);
        entities[object].type = types.unqualified(t); register_destruction(object);
        materialized.temporary = object;
    } else {
        // The initialized object/subobject owns storage and lifetime. Preserve
        // the same destructor legality and ABI demand without a duplicate object.
        destination_destructor(types.unqualified(t),facts[n].scope);
    }
    NodeId source = n;
    while (ast[source].kind == syntax::Kind::Parenthesized) source = ast[source].first;
    auto value = expressions[source];
    materialized.elided = !conversion.reference && value.category == ValueCategory::Prvalue && types.unqualified(value.type) == types.unqualified(t) &&
        (ast[source].kind == syntax::Kind::Call || value.form == ExpressionForm::Construction || value.form == ExpressionForm::OperatorCall || value.form == ExpressionForm::Cast);
    if (!conversion.reference && ast[source].kind == syntax::Kind::Conditional) {
        materialized.elision_permission = value.category == ValueCategory::Prvalue && types.unqualified(value.type) == types.unqualified(t);
        if (defer || (materialized.elision_permission && trivial_transfer(ctor) && copy_storage_type(t))) {
            // A prvalue conditional already owns these exact branch transfers.
            // Reuse its slice so nested destinations do not rebuild subtrees.
            if (materialized.elision_permission) materialized.branches = value.conversions+1;
            else {
                NodeId b = ast[ast[source].first].next, c = ast[b].next;
                std::vector<NodeId> operands{b,c};
                std::vector<Conversion> selected{this->conversion(b,t),this->conversion(c,t)};
                for (auto& branch : selected) if (!branch.valid()) throw std::runtime_error("invalid conditional class transfer");
                Expression branches; record_call(branches,operands,selected);
                materialized.branches = branches.conversions;
            }
            materialized.elided = true;
        }
    }
    if (!defer && !materialized.elided) demand_member(ctor);
    members[m].complete_entry = true;
    Type f = types[entities[ctor].type];
    std::vector<NodeId> arguments;
    std::vector<Conversion> selected;
    auto call = recipe ? conversion_objects[recipe].call : Expression();
    for (unsigned j = 0; j < f.count; ++j) {
        Conversion c;
        NodeId a = j ? (recipe ? call_argument(call,j) : default_argument(ctor,j,&c)) : n;
        if (recipe && j) { default_argument(ctor,j); expression(a,facts[n].scope); }
        if (recipe) c = copy_conversion_recipe(conversions[call.conversions+j]);
        else if (!j) c = conversion.implicit_move ? transfer_conversion(expressions[a].type,ValueCategory::Xvalue,types.parameters[f.offset+j]) :
            this->conversion(a, types.parameters[f.offset+j],false);
        if (!c.valid()) throw std::runtime_error("invalid converting constructor argument");
        apply_conversion(a, c); arguments.push_back(a); selected.push_back(c);
    }
    if (!f.count && f.variadic) {
        auto c = recipe ? conversions[call.conversions] : ellipsis_conversion(n); apply_conversion(n,c);
        arguments.push_back(n); selected.push_back(c);
    }
    materialized.call.inputs = CallInputs::Concrete;
    materialized.call.arguments = call_arguments.size(); materialized.call.argument_count = arguments.size();
    materialized.call.conversions = conversions.size(); materialized.call.count = selected.size();
    call_arguments.insert(call_arguments.end(), arguments.begin(), arguments.end());
    conversions.insert(conversions.end(), selected.begin(), selected.end());
    conversion.materialization = conversion_objects.size(); conversion_objects.push_back(materialized);
}
} }
