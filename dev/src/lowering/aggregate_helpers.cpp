#include "lowering/procedural.h"
namespace cppgm { namespace lowering {
using semantic::InitKind;
SymbolId Procedural::aggregate_helper(std::uint32_t plan)
{
    auto action = sem.initializers[plan];
    TypeId target = action.type;
    if (auto known = aggregate_helpers.get(target)) return p.functions[known-1].symbol;
    AggregateHelper helper; helper.type = target; helper.actions = aggregate_actions.size(); helper.count = 0;
    std::vector<TypeId> parameters(1, sem.types.compound(TypeKind::Pointer, target));
    for (auto child = action.first; child; child = sem.initializers[child].next) {
        auto item = sem.initializers[child];
        aggregate_actions.push_back(child); ++helper.count; parameters.push_back(item.type);
    }
    helper.function = FunctionId(p.functions.size()+1);
    lowir_model::Function f; f.symbol = fresh_symbol("@__aggregate_" + std::to_string(target));
    f.signature = signature(sem.types.function(sem.types.fundamental(FT_VOID), parameters, false), helper.function);
    p.signatures[f.signature.index-1].boundary.unwind = ir_model::CUM_NO;
    p.functions.push_back(f);
    auto& symbol = p.symbols[f.symbol.index-1]; symbol.kind = lowir_model::Symbol::FunctionSymbol;
    symbol.entity = helper.function.index; symbol.metadata.binding = ir_model::SBM_INTERNAL;
    aggregate_helpers.put(target, helper.function.index); aggregate_definitions.push_back(helper);
    return f.symbol;
}
bool Procedural::call_aggregate_helper(std::uint32_t plan, Value location)
{
    auto action = sem.initializers[plan];
    if (action.kind != InitKind::Group || sem.types[action.type].kind != TypeKind::Named) return false;
    for (auto child = action.first; child; child = sem.initializers[child].next) {
        auto item = sem.initializers[child];
        if (!item.field || !type(item.type).scalar() || item.kind != InitKind::Scalar) return false;
    }
    SymbolId callee = aggregate_helper(plan);
    std::size_t begin = call_work.size();
    call_work.push_back(Operand::symbol(callee)); call_work.push_back(address(location).operand);
    for (auto child = action.first; child; child = sem.initializers[child].next) {
        auto item = sem.initializers[child];
        call_work.push_back(initialization_value(item.source, item.type).operand);
    }
    guarded_call(Instruction(Opcode::Call, IRType::Void), call_work.data()+begin, call_work.size()-begin);
    call_work.resize(begin); return true;
}
void Procedural::emit_aggregate_helpers()
{
    for (auto helper : aggregate_definitions) {
        function = helper.function; reset_lifetime(0); initialized_units = semantic::Index();
        builder.reset(new lowir_model::FunctionBuilder(p, function)); start(block());
        auto signature = p.signatures[p.functions[function.index-1].signature.index-1];
        std::vector<SlotId> slots;
        for (unsigned j = 0; j < signature.parameters.count; ++j) {
            auto parameter = p.parameters[signature.parameters.begin+j];
            auto slot = builder->add_slot(0, parameter.type); slots.push_back(slot);
            emit(Opcode::Store, parameter.type, {Operand::value(parameter.value), Operand::slot(slot)});
        }
        for (unsigned j = 0; j < helper.count; ++j) {
            auto action = sem.initializers[aggregate_actions[helper.actions+j]];
            EntityId field = action.field; TypeId t = action.type;
            Value value = emit(Opcode::Load, type(t), {Operand::slot(slots[j+1])}); value.type = t;
            Value base = emit(Opcode::Load, IRType::Ptr, {Operand::slot(slots[0])});
            Instruction index(Opcode::Index, IRType::I8); index.projection = ir_model::IPK_FIELD;
            Value at = emit(index, {base.operand, Operand::integer(sem.entities[field].member_offset)});
            at.type = t; at.address = true; at.init_offset = sem.entities[field].member_offset; at.initializing = true;
            if (sem.field_fact(field).bit_field) at.bit_field = field;
            store(value, at);
        }
        emit(Opcode::Return, IRType(), {}); builder.reset();
    }
}
} }
