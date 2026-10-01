#include "lowering/procedural.h"
namespace cppgm { namespace lowering {
void Procedural::assembly_statement(NodeId n)
{
    using namespace syntax;
    // Locations, register inputs and matching inputs are captured before any
    // instruction. Register outputs are committed after the whole statement.
    // A memory operand instead observes its location at each instruction.
    struct OperandUse { Value location, value; unsigned flags, match; };
    std::vector<OperandUse> operands;
    for (auto c = ast[n].first; c; c = ast[c].next) {
        OperandUse use; use.flags = ast[c].flags; use.match = ast[c].literal;
        auto value = expression(ast[c].first);
        if (ended) return;
        if (use.flags & (AsmOutput|AsmMemory)) use.location = value;
        if ((use.flags & AsmRead) && !(use.flags & AsmMemory)) use.value = load(value);
        operands.push_back(use);
    }
    for (const auto& use : operands) if (use.match) operands[use.match-1].value = use.value;
    const auto& plan = ast.assemblies[ast[n].literal];
    // Conservatively retain every assembly statement and a compiler barrier.
    // This is stronger than a nonvolatile recipe but avoids invented effects.
    emit(Opcode::AtomicSignalFence,IRType(),{Operand::integer(5)});
    auto read = [&](unsigned i) { return operands[i].flags & AsmMemory ? load(operands[i].location) : operands[i].value; };
    auto write = [&](unsigned i, Value value) {
        if (operands[i].flags & AsmMemory) store(value,operands[i].location);
        else operands[i].value = value;
    };
    for (unsigned i = 0; i < plan.count; ++i) {
        const auto instruction = ast.assembly_instructions[plan.begin+i]; auto op = instruction.op;
        if (op == AsmOp::Nop || op == AsmOp::Pause) {
            emit(op == AsmOp::Nop ? Opcode::Nop : Opcode::Pause,IRType(),{}); continue;
        }
        if (op == AsmOp::Fence) { emit(Opcode::AtomicThreadFence,IRType(),{Operand::integer(5)}); continue; }
        unsigned d = instruction.destination; auto target = operands[d].location;
        auto ir = type(target.type);
        bool binary = (op >= AsmOp::Move && op <= AsmOp::Xor) || op >= AsmOp::Exchange;
        Value source;
        if (binary) source = instruction.immediate ? Value(Operand::integer(instruction.value),ir) : coerce(read(instruction.source),ir);
        bool atomic = instruction.locked || (op == AsmOp::Exchange && (operands[d].flags & AsmMemory));
        auto compute = [&](Value prior) {
            if (op == AsmOp::Move || op == AsmOp::Exchange) return source;
            if (op == AsmOp::Bswap || op == AsmOp::Not || op == AsmOp::Neg)
                return emit(Opcode::Unary,ir,{prior.operand},op == AsmOp::Bswap ? Operation::Bswap : op == AsmOp::Not ? Operation::Bitnot : Operation::Neg);
            auto rhs = op == AsmOp::Inc || op == AsmOp::Dec ? Operand::integer(1) : source.operand;
            auto operation = op == AsmOp::Sub || op == AsmOp::Dec ? Operation::Sub : op == AsmOp::And ? Operation::And : op == AsmOp::Or ? Operation::Or : op == AsmOp::Xor ? Operation::Xor : Operation::Add;
            return emit(Opcode::Binary,ir,{prior.operand,rhs},operation);
        };
        Value prior, result;
        if (atomic) {
            // x86 locked operations on these 1/2/4/8-byte memory operands do
            // not require C++ atomic type identity or natural alignment.
            auto pointer = address(target).operand;
            if (op == AsmOp::Exchange) prior = emit(Opcode::AtomicExchange,ir,{pointer,source.operand,Operand::integer(5)});
            else if (op == AsmOp::Inc || op == AsmOp::Dec || op == AsmOp::Add || op == AsmOp::Sub || op == AsmOp::Xadd) {
                auto delta = op == AsmOp::Inc || op == AsmOp::Dec ? Value(Operand::integer(1),ir) : source;
                if (op == AsmOp::Dec || op == AsmOp::Sub) delta = emit(Opcode::Unary,ir,{delta.operand},Operation::Neg);
                auto updated = emit(Opcode::AtomicAddFetch,ir,{pointer,delta.operand,Operand::integer(5)});
                if (op == AsmOp::Xadd) prior = emit(Opcode::Binary,ir,{updated.operand,delta.operand},Operation::Sub);
            }
            else {
                auto expected = Operand::slot(builder->add_slot(0,ir));
                prior = emit(Opcode::AtomicLoad,ir,{pointer,Operand::integer(5)});
                emit(Opcode::Store,ir,{prior.operand,expected});
                auto expected_pointer = address(Value(expected,ir,0,true)).operand;
                auto retry = block(), done = block(); jump(retry); start(retry);
                prior = emit(Opcode::Load,ir,{expected}); result = compute(prior);
                auto success = emit(Opcode::AtomicCompareExchange,ir,{pointer,expected_pointer,result.operand,Operand::integer(5),Operand::integer(5)});
                emit(Opcode::Branch,IRType(),{success.operand,Operand::label(done),Operand::label(retry)}); start(done);
            }
        } else {
            if (op != AsmOp::Move) prior = read(d);
            result = compute(prior); write(d,result);
        }
        if (op == AsmOp::Exchange || (op == AsmOp::Xadd && instruction.source != d)) write(instruction.source,prior);
    }
    for (const auto& use : operands)
        if ((use.flags & AsmOutput) && !(use.flags & AsmMemory)) store(use.value,use.location);
    emit(Opcode::AtomicSignalFence,IRType(),{Operand::integer(5)});
}
} }
