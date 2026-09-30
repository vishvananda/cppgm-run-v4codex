#include "lowering/procedural.h"
#include <stdexcept>
namespace cppgm { namespace lowering {
using namespace lowir_model;
SignatureId Procedural::function_signature(EntityId e, FunctionId owner, bool base)
{
    auto result = signature(sem.call_type(e),owner);
    auto effects = sem.function_effects(e);
    p.signatures[result.index-1].boundary.effects = effects == syntax::FunctionEffects::ReadNone ?
        CFXM_READNONE : effects == syntax::FunctionEffects::ReadOnly ? CFXM_READONLY : CFXM_DEFAULT;
    if (!(base || base_only_entry(e)) || !(sem.constructor_member(e) || sem.destructor_member(e))) return result;
    auto cls = sem.scopes[sem.entities[e].owner].entity;
    auto count = sem.virtual_base_count(cls);
    if (!count) return result;
    auto& sig = p.signatures[result.index-1];
    for (unsigned j = 0; j <= count; ++j) {
        Parameter param; param.type = IRType::Ptr;
        lowir_model::Value value; value.type = IRType::Ptr; value.owner = owner; value.defined = true;
        p.values.push_back(value); param.value = ValueId(p.values.size()); p.parameters.push_back(param);
    }
    sig.parameters.count += count+1;
    return result;
}
Value Procedural::lifecycle_address(EntityId base)
{
    if (auto id = hidden_base_addresses.get(base)) return Value(Operand::value(ValueId(id)),IRType::Ptr);
    if (active_base_entry) throw std::logic_error("missing base-entry virtual address");
    auto cls = sem.scopes[sem.entities[active_function].owner].entity;
    auto object = emit(Opcode::Load,IRType::Ptr,{Operand::slot(this_slot)});
    return emit(Opcode::Index,IRType::I8,{object.operand,Operand::integer(sem.virtual_base_offset(cls,base))});
}
Value Procedural::lifecycle_vtt(unsigned index)
{
    auto value = vtt_argument;
    if (!active_base_entry) {
        auto cls = sem.scopes[sem.entities[active_function].owner].entity;
        value = emit(Opcode::Addr,IRType(),{Operand::symbol(abi_global(cls,abi_mangle::TargetKind::Vtt))});
    }
    if (!index) return value;
    return emit(Opcode::Index,IRType::I8,{value.operand,Operand::integer(index*8)});
}
void Procedural::lifecycle_arguments(EntityId target, std::vector<Operand>& args, unsigned base)
{
    if (!sem.constructor_member(target) && !sem.destructor_member(target)) return;
    auto cls = sem.scopes[sem.entities[target].owner].entity;
    if (!sem.virtual_base_count(cls)) return;
    unsigned index = 0;
    if (base) {
        const auto& fact = sem.lifecycle_bases[base];
        if (fact.type != cls) throw std::logic_error("mismatched lifecycle base identity");
        index = fact.vtt;
    } else if (cls != sem.scopes[sem.entities[active_function].owner].entity)
        throw std::logic_error("missing lifecycle base identity");
    args.push_back(lifecycle_vtt(index).operand);
    for (unsigned j = 0; j < sem.virtual_base_count(cls); ++j)
        args.push_back(lifecycle_address(sem.virtual_base_type(cls,j)).operand);
}
} }
