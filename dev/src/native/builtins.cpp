#include "native/encoding.h"
namespace native {
// Canonical builtin identity enters through typed symbol metadata. This is a
// target runtime definition, constructed as MIR and consumed by the same encoder.
Function builtin_strlen(const lowir_model::Program& p, const lowir_model::Function& source)
{
    Function f; f.symbol = source.symbol; f.result = Type::I64;
    const auto& signature = p.signatures[source.signature.index-1];
    lowir_model::require(signature.parameters.count == 1 && signature.result == Type::I64 &&
        p.parameters[signature.parameters.begin].type == Type::Ptr,"invalid strlen boundary");
    auto value = p.parameters[signature.parameters.begin].value;
    f.params.push_back({p.values[value.index-1].name,Type::Ptr,Operand::r(XR_RDI)});
    auto block = [&](unsigned id) { Block b; b.id = id; b.name = 0; b.instructions.begin = f.instructions.size(); f.blocks.push_back(b); };
    auto emit = [&](Op op,Type type,std::initializer_list<Operand> args) -> Instruction& {
        Instruction i(op,type); i.count = args.size(); std::copy(args.begin(),args.end(),i.args.begin());
        f.instructions.push_back(i); ++f.blocks.back().instructions.count; return f.instructions.back();
    };
    unsigned entry = p.blocks.size()+1, loop = entry+1, next = entry+2, done = entry+3;
    block(entry); emit(Op::Mov,Type::I64,{Operand::r(XR_RAX),Operand::imm(0)});
    block(loop);
    auto address = Operand::mem(XR_RDI); address.index = XR_RAX;
    emit(Op::Load,Type::U8,{Operand::r(XR_RDX),address});
    emit(Op::Compare,Type::U8,{Operand::r(XR_RDX),Operand::imm(0)});
    emit(Op::Jcc,Type(),{Operand::label(done)}).condition = XC_E;
    block(next);
    emit(Op::Add,Type::I64,{Operand::r(XR_RAX),Operand::imm(1)});
    emit(Op::Jump,Type(),{Operand::label(loop)});
    block(done); emit(Op::Return,Type::I64,{Operand::r(XR_RAX)});
    return f;
}
} // namespace native

namespace native {
Function builtin_fill(const lowir_model::Program& p, const lowir_model::Function& source)
{
    const auto& sig = p.signatures[source.signature.index-1];
    lowir_model::require(sig.parameters.count == 3 && sig.result == Type::Void &&
        p.parameters[sig.parameters.begin].type == Type::Ptr &&
        p.parameters[sig.parameters.begin+1].type == Type::I32 &&
        p.parameters[sig.parameters.begin+2].type == Type::I64,"invalid fill boundary");
    Function f; f.symbol = source.symbol; f.result = Type::Void;
    f.frame_pointer = false; f.shared_epilogue = false;
    int regs[] = {XR_RDI,XR_RSI,XR_RDX};
    for (unsigned n = 0; n < 3; ++n) {
        auto a = p.parameters[sig.parameters.begin+n];
        f.params.push_back({p.values[a.value.index-1].name,a.type,Operand::r(regs[n])});
    }
    auto block = [&](unsigned id) { Block b; b.id = id; b.name = 0; b.instructions.begin = f.instructions.size(); f.blocks.push_back(b); };
    auto emit = [&](Op op, Type type, std::initializer_list<Operand> args) -> Instruction& {
        Instruction i(op,type); i.count = args.size(); std::copy(args.begin(),args.end(),i.args.begin());
        f.instructions.push_back(i); ++f.blocks.back().instructions.count; return f.instructions.back();
    };
    block(p.blocks.size()+1);
    emit(Op::Mov,Type::I64,{Operand::r(XR_RAX),Operand::r(XR_RSI)});
    emit(Op::Mov,Type::I64,{Operand::r(XR_RCX),Operand::r(XR_RDX)});
    emit(Op::FillBytes,Type(),{Operand::r(XR_RDI),Operand::r(XR_RAX),Operand::r(XR_RCX)});
    emit(Op::Return,Type(),{});
    return f;
}
}
