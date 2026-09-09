#include "semantic/analyzer.h"
#include <stdexcept>
namespace cppgm { namespace semantic {
using syntax::Kind;
bool Analyzer::class_value(TypeId t) const
{ return types[t].kind == TypeKind::Named && entities[types[t].entity].class_info; }
bool Analyzer::indirect_value(TypeId t) const
{ return class_value(t) && class_facts[entities[types[t].entity].class_info].value_abi == 2; }
bool Analyzer::indirect_parameter(TypeId t) const
{ return class_value(t) && class_facts[entities[types[t].entity].class_info].parameter_abi == 2; }
bool Analyzer::empty_class(TypeId t) const
{ return class_value(t) && class_facts[entities[types[t].entity].class_info].empty; }
bool Analyzer::parameter_cleanup(EntityId e) const
{ return entities[e].kind == EntityKind::Parameter && class_value(entities[e].type) && class_facts[entities[types[entities[e].type].entity].class_info].trivial_destructor_state == 2; }
const ValueInitialization& Analyzer::class_initialization(NodeId n, TypeId t) const
{ return value_initializations[class_initializer_index.get(key(n,t))]; }
const ValueReturn& Analyzer::class_return(NodeId n) const
{ return value_returns[class_return_index.get(n)]; }
EntityId Analyzer::return_object(EntityId e) const
{ return function_returns[function_return_index.get(e)].object; }
void Analyzer::prepare_value_boundary(TypeId t)
{
    if (!class_value(t) || !entities[types[t].entity].complete) return;
    auto info = entities[types[t].entity].class_info;
    if (class_facts[info].value_abi) return;
    class_facts[info].value_abi = class_facts[info].parameter_abi = 2;
    bool simple_destruction = trivial_destructor(t);
    bool direct = false;
    if (simple_destruction) {
        direct = copy_storage_type(t);
        EntityId move = select_transfer(t, types.unqualified(t), ValueCategory::Xvalue, false);
        direct |= move && !deleted_transfer(move) && trivial_transfer(move);
    }
    class_facts[info].parameter_abi = direct ? 1 : 2;
    class_facts[info].value_abi = direct && size(t) <= 16 ? 1 : 2;
}
void Analyzer::prepare_function_boundaries()
{
    // One declaration traversal; references do not trigger class-value demand.
    for (EntityId e = 1; e < entities.size(); ++e) {
        if (entities[e].kind != EntityKind::Function || entities[e].template_info) continue;
        Type f = types[entities[e].type];
        prepare_value_boundary(f.child);
        for (unsigned j = 0; j < f.count; ++j) prepare_value_boundary(types.parameters[f.offset+j]);
    }
}
void Analyzer::class_result(NodeId n, Expression& result, ScopeId s)
{
    if (result.category != ValueCategory::Prvalue || !class_value(result.type)) return;
    if (ast[n].kind != Kind::Call && ast[n].kind != Kind::Conditional && result.form != ExpressionForm::OperatorCall && result.form != ExpressionForm::LiteralCall && result.form != ExpressionForm::Cast) return;
    if (!result.object_use) record_object(result,0,0,0);
    if (object_uses[result.object_use].temporary) return;
    if (result.form == ExpressionForm::Cast && result.count) {
        auto c = conversions[result.conversions];
        if (c.kind == Conversion::Kind::User && user_conversions[c.materialization].temporary) {
            object_uses[result.object_use].temporary = user_conversions[c.materialization].temporary; return;
        }
    }
    EntityId temporary = make_entity(EntityKind::Variable, make_scope(ScopeKind::Block,s),0,n);
    entities[temporary].type = types.unqualified(result.type);
    register_destruction(temporary);
    object_uses[result.object_use].temporary = temporary;
}
bool Analyzer::record_class_initialization(NodeId n, TypeId target, NodeId source, const Conversion* selected)
{
    Conversion c = selected ? *selected : conversion(source,target);
    if (!c.valid()) throw std::runtime_error("invalid class value initialization");
    apply_conversion(source,c);
    ValueInitialization init; init.source = source; init.conversion = conversions.size(); conversions.push_back(c);
    class_initializer_index.put(key(n,target),value_initializations.size()); value_initializations.push_back(init);
    if (n != source) facts[n].type = target;
    return true;
}
void Analyzer::record_class_return(NodeId n, ScopeId s)
{
    NodeId source = ast[n].first;
    if (!source) throw std::runtime_error("missing class return value");
    Expression x = expression(source,s);
    NodeId id = source;
    while (ast[id].kind == Kind::Parenthesized) id = ast[id].first;
    EntityId local = ast[id].kind == Kind::IdExpression ? expressions[id].entity : 0;
    bool eligible = local && (entities[local].kind == EntityKind::Variable || entities[local].kind == EntityKind::Parameter) &&
        !entities[local].is_static && !entities[local].external_decl && class_value(entities[local].type) &&
        encloses(entities[current_function].scope,entities[local].owner);
    Conversion c = conversion(source,return_type);
    if (eligible && types.unqualified(x.type) == types.unqualified(return_type)) {
        EntityId ctor = select_transfer(return_type,x.type,ValueCategory::Xvalue,false);
        if (ctor) { c.kind = Conversion::Kind::Construction; c.function = ctor; c.rank = 0; c.implicit_move = true; }
    }
    if (!c.valid()) throw std::runtime_error("invalid class return conversion");
    if (c.kind == Conversion::Kind::Construction) materialize_conversion(source,c,true);
    else apply_conversion(source,c);
    ValueReturn record; record.source = source; record.conversion = conversions.size(); conversions.push_back(c);
    if (eligible && entities[local].kind == EntityKind::Variable && entities[local].owner == facts[entities[current_function].body].scope &&
        types.unqualified(x.type) == types.unqualified(return_type)) record.local = local;
    auto owner = function_return_index.get(current_function);
    if (!owner) { owner = function_returns.size(); function_returns.emplace_back(); function_return_index.put(current_function,owner); }
    auto r = value_returns.size(); value_returns.push_back(record); class_return_index.put(n,r);
    if (function_returns[owner].last) value_returns[function_returns[owner].last].next = r;
    else function_returns[owner].first = r;
    function_returns[owner].last = r;
}
void Analyzer::finish_class_returns(EntityId e)
{
    auto f = function_return_index.get(e);
    if (!f) return;
    TypeId type = types[entities[e].type].child; prepare_value_boundary(type);
    EntityId local = value_returns[function_returns[f].first].local;
    for (auto r = function_returns[f].first; r; r = value_returns[r].next) if (value_returns[r].local != local) local = 0;
    if (!indirect_value(type) || !trivial_destructor(type)) local = 0;
    function_returns[f].object = local;
    for (auto r = function_returns[f].first; r; r = value_returns[r].next) {
        auto c = conversions[value_returns[r].conversion];
        if (c.kind != Conversion::Kind::Construction || (local && value_returns[r].local == local) || conversion_objects[c.materialization].elided) continue;
        demand_member(c.function);
    }
}
} }
