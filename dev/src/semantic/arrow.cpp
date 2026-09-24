#include "semantic/analyzer.h"
#include <stdexcept>
namespace cppgm { namespace semantic {
std::uint32_t Analyzer::prepare_arrow(NodeId n, ScopeId s)
{
    return prepare_arrow(expressions[n],s,n,true);
}
std::uint32_t Analyzer::prepare_arrow(Expression object, ScopeId s, NodeId n, bool demand)
{
    if (!class_value(object.type)) return 0;
    std::vector<ArrowStep> steps; Index seen;
    while (class_value(object.type)) {
        auto cls = types[object.type].entity;
        if (demand) size(object.type);
        else {
            complete_class(cls);
            if (!entities[cls].complete) { incomplete_query(object.type); return 0; }
        }
        auto naming = entities[cls].scope;
        auto family = lookup(naming,operator_name(OP_ARROW),Lookup::Ordinary,true);
        std::vector<Conversion> chosen;
        auto selection = select_call(family,{},nullptr,object.type,object.category,naming,0,chosen);
        if (selection.failure != CallFailure::None) {
            if (!demand) return 0;
            throw std::runtime_error("no unique operator->");
        }
        auto e = selection.entity;
        if (seen.get(e)) {
            if (!demand) return 0;
            throw std::runtime_error("recursive operator-> result type");
        }
        seen.put(e,1);
        if (deleted_transfer(e) || !accessible(e,s,naming,object.type)) {
            if (!demand) return 0;
            throw std::runtime_error("deleted or inaccessible operator->");
        }
        if (demand) { demand_member(e); demand_specialization(e); }
        ArrowStep step; step.function = e; step.result = types[entities[e].type].child;
        step.adjustment = demand ? base_steps(object.type,scopes[entities[e].owner].entity) :
            base_path(object.type,scopes[entities[e].owner].entity);
        step.virtual_slot = members[entities[e].member_info].virtual_slot;
        object.type = value_type(step.result);
        object.category = types[step.result].kind == TypeKind::LRef ? ValueCategory::Lvalue :
            types[step.result].kind == TypeKind::RRef ? ValueCategory::Xvalue : ValueCategory::Prvalue;
        if (class_value(object.type) && object.category == ValueCategory::Prvalue) {
            if (demand) {
                step.temporary = make_entity(EntityKind::Variable,make_scope(ScopeKind::Block,s),0,n);
                entities[step.temporary].type = object.type; register_destruction(step.temporary);
            } else {
                auto destructor = destructor_declaration(object.type);
                if (!destructor) { incomplete_query(object.type); return 0; }
                if (deleted_transfer(destructor) || !accessible(destructor,s,entities[destructor].owner) ||
                    !default_destructor_valid(destructor)) return 0;
            }
        }
        steps.push_back(step);
    }
    if (!pointer(object.type)) {
        if (!demand) return 0;
        throw std::runtime_error("operator-> must end in a pointer");
    }
    ArrowChain chain; chain.first = arrow_steps.size(); chain.count = steps.size(); chain.type = object.type;
    auto id = arrow_chains.size(); arrow_chains.push_back(chain);
    arrow_steps.insert(arrow_steps.end(),steps.begin(),steps.end()); return id;
}
std::uint32_t Analyzer::constant_arrow(NodeId n, std::uint32_t id)
{
    return constant_arrow_value(constant_node_object(n),id);
}
std::uint32_t Analyzer::constant_arrow_value(std::uint32_t object, std::uint32_t id)
{
    auto chain = arrow_chains[id];
    for (unsigned i = 0; object && i < chain.count; ++i) {
        auto step = arrow_steps[chain.first+i];
        if (step.virtual_slot) return 0;
        object = constant_base_address(object,entities[scopes[entities[step.function].owner].entity].type);
        auto value = execute_constant(step.function,{},object);
        if (!value.valid) return 0;
        auto kind = types[value.type].kind;
        object = kind == TypeKind::Pointer || kind == TypeKind::LRef || kind == TypeKind::RRef ? value.bits :
            constant_storage_address(value.type,value);
    }
    return object;
}
} }
