#include "native/selection.h"
namespace native {
using namespace lowir_model;
void Selector::call(const lowir_model::Instruction& i)
{
    static const int registers[] = {XR_RDI,XR_RSI,XR_RDX,XR_RCX,XR_R8,XR_R9};
    SignatureId signature_id = i.signature;
    auto target_input = arg(i,0);
    if (!signature_id && target_input.kind == lowir_model::Operand::Symbol) {
        const auto& symbol = p.symbols[target_input.ref-1];
        signature_id = p.functions[symbol.entity-1].signature;
    }
    require(bool(signature_id), "call has no ABI signature");
    const auto& signature = p.signatures[signature_id.index-1];
    require(signature.result == Type() || scalar_integer(signature.result) || signature.result.floating(), "native call result class not implemented");
    Operand target = value(target_input,Type::Ptr);
    if (target_input.kind == lowir_model::Operand::Temporary) {
        const auto& origin = state(root(target_input.ref));
        if (origin.definition) {
            const auto& def = p.instructions[origin.definition-1];
            if (def.opcode == Opcode::Addr && arg(def,0).kind == lowir_model::Operand::Symbol &&
                p.symbols[arg(def,0).ref-1].kind == lowir_model::Symbol::GlobalSymbol && target.kind != Operand::Symbol) {
                target = in_register(target,Type::Ptr,XR_R11);
                move(Operand::r(XR_R11),Operand::mem(target.reg),Type::Ptr); target = Operand::r(XR_R11);
            }
        }
    }
    if (target.kind == Operand::Symbol && p.symbols[target.id-1].kind == lowir_model::Symbol::GlobalSymbol) {
        target.address = false;
        move(Operand::r(XR_R11),target,Type::Ptr); target = Operand::r(XR_R11);
    } else if (target.kind != Operand::Symbol) {
        move(Operand::r(XR_R11),target,Type::Ptr); target = Operand::r(XR_R11);
    }
    unsigned count = i.operands.count-1;
    struct Assignment { Operand to, from; Type type; bool done = false; };
    std::vector<Assignment> moves;
    std::vector<Assignment> stack_moves;
    unsigned gp = 0, fp = 0;
    std::uint64_t stack = 0;
    for (unsigned n = 0; n < count; ++n) {
        const auto& a = arg(i,n+1);
        bool fixed = n < signature.parameters.count;
        Type t = fixed ? p.parameters[signature.parameters.begin+n].type : value_type(a,Type::I64);
        if (!fixed && a.kind == lowir_model::Operand::Floating) t = Type::F64;
        require(scalar_integer(t) || t.floating(), "native argument ABI class not implemented");
        Operand from = value(a,t);
        Type actual = value_type(a,t);
        if (fixed && p.parameters[signature.parameters.begin+n].passing != PPM_DIRECT) {
            if (a.kind == lowir_model::Operand::Slot) from.address = true;
            else if (actual != Type::Ptr) {
                Operand storage = home(a.kind == lowir_model::Operand::Temporary ? p.values[a.ref-1].name : 0,actual,true);
                move(storage,from,actual); from = storage; from.address = true;
            }
            actual = Type::Ptr;
        }
        if (actual != t && (actual.floating() || t.floating())) {
            auto storage = home(0,t,true); convert_to(storage,from,actual,t); from = storage; actual = t;
        }
        Assignment m; m.from = from; m.type = actual;
        if ((t == Type::F32 || t == Type::F64) && fp < 8) m.to = Operand::r(xmm(fp++));
        else if (scalar_integer(t) && gp < 6) m.to = Operand::r(registers[gp++]);
        else {
            unsigned alignment = std::max(8u,t.alignment());
            stack = (stack+alignment-1)&~std::uint64_t(alignment-1);
            m.to = Operand::mem(XR_RSP,stack); stack += std::max(8u,t.bytes());
            stack_moves.push_back(m); continue;
        }
        moves.push_back(m);
    }
    stack = (stack+15)&~std::uint64_t(15);
    if (stack) emit(Op::Sub,Type::I64,{Operand::r(XR_RSP),Operand::imm(stack)});
    for (const auto& m : stack_moves) move(m.to,m.from,m.type);
    // At most fourteen scalar carriers: bounded parallel move scheduling, with one
    // reserved scratch to break cycles. Stack arguments are captured first.
    unsigned pending = moves.size();
    while (pending) {
        bool progress = false;
        for (auto& m : moves) {
            if (m.done) continue;
            bool needed = false;
            for (const auto& other : moves)
                if (!other.done && &m != &other && other.from.kind == Operand::Reg && other.from.reg == m.to.reg) needed = true;
            if (needed) continue;
            move(m.to,m.from,m.type); m.done = true; --pending; progress = true;
        }
        if (progress) continue;
        for (auto& m : moves) if (!m.done) {
            auto scratch = Operand::r(m.to.reg >= 16 ? xmm(15) : XR_R10);
            move(scratch,m.to,m.type);
            for (auto& other : moves) if (!other.done && other.from.kind == Operand::Reg && other.from.reg == m.to.reg)
                other.from = scratch;
            break;
        }
    }
    if (signature.boundary.arity == CAM_VARIADIC)
        emit(Op::Mov,Type::I64,{Operand::r(XR_RAX),Operand::imm(fp)});
    auto& site = emit(Op::Call,i.type,{target});
    site.bytes = stack; site.boundary = signature.boundary;
    for (const auto& m : moves) site.arg_registers |= 1u << m.to.reg;
    if (stack) emit(Op::Add,Type::I64,{Operand::r(XR_RSP),Operand::imm(stack)});
    if (i.destination && state(i.destination.index).uses) {
        auto dest = allocate(i.destination.index,i.type);
        if (i.type == Type::F80) { f.scratch_bytes = 48; emit(Op::Fpop,i.type,{dest}); }
        else {
            normalize_register(Operand::r(XR_RAX),i.type);
            move(dest,Operand::r(i.type.floating() ? xmm(0) : XR_RAX),i.type);
        }
    } else if (i.type == Type::F80) {
        f.scratch_bytes = 48; emit(Op::Fpop,i.type,{home(0,i.type,true)});
    }
}
void Selector::bulk(const lowir_model::Instruction& i)
{
    auto dst = memory(arg(i,i.opcode == Opcode::CopyObject ? 1 : 0),XR_R10);
    if (i.opcode == Opcode::CopyObject) {
        auto src = memory(arg(i,0),XR_R11);
        auto& instruction = emit(Op::CopyBytes,Type(),{dst,src});
        instruction.bytes = i.bytes; instruction.alignment = i.alignment;
    } else {
        auto& instruction = emit(Op::ZeroBytes,Type(),{dst});
        instruction.bytes = i.bytes; instruction.alignment = i.alignment;
    }
}
void Selector::atomic(const lowir_model::Instruction& i)
{
    if (i.opcode == Opcode::AtomicSignalFence) return;
    if (i.opcode == Opcode::AtomicThreadFence) {
        if (arg(i,0).data.integer == 5) emit(Op::Fence,Type(),{});
        return;
    }
    require(scalar_integer(i.type), "native atomic class not implemented");
    Operand address = memory(arg(i,i.opcode == Opcode::AtomicStore ? 1 : 0));
    if (i.opcode == Opcode::AtomicLoad) {
        auto dest = allocate(i.destination.index,i.type);
        auto reg = dest.kind == Operand::Reg ? dest : Operand::r(XR_R10);
        emit(Op::Load,i.type,{reg,address}); move(dest,reg,i.type); return;
    }
    if (i.opcode == Opcode::AtomicStore) {
        auto from = value(arg(i,0),i.type);
        if (arg(i,2).data.integer == 5) {
            move(Operand::r(XR_R10),from,value_type(arg(i,0),i.type));
            emit(Op::Exchange,i.type,{address,Operand::r(XR_R10)});
        } else move(address,from,i.type);
        return;
    }
    if (i.opcode == Opcode::AtomicCompareExchange) {
        // The expected operand denotes storage, and failure updates that storage.
        // Stage the atomic address in rcx so resolving expected cannot clobber it.
        address.address = true; move(Operand::r(XR_RCX),address,Type::Ptr);
        auto expected = memory(arg(i,1));
        expected.address = true; move(Operand::r(XR_R11),expected,Type::Ptr);
        move(Operand::r(XR_RAX),Operand::mem(XR_R11),i.type);
        move(Operand::r(XR_R10),value(arg(i,2),i.type),value_type(arg(i,2),i.type));
        emit(Op::Cmpxchg,i.type,{Operand::mem(XR_RCX),Operand::r(XR_R10)});
        emit(Op::Set,Type::I1,{Operand::r(XR_R10)}).condition = XC_E;
        emit(Op::ExtendUnsigned,Type::U8,{Operand::r(XR_R10),Operand::r(XR_R10)});
        unsigned success = next_label++;
        emit(Op::Jcc,Type(),{Operand::label(success)}).condition = XC_E;
        emit(Op::Store,i.type,{Operand::mem(XR_R11),Operand::r(XR_RAX)});
        begin_block(success,0);
        if (i.destination) move(allocate(i.destination.index,Type::I64),Operand::r(XR_R10),Type::I64);
        return;
    }
    move(Operand::r(XR_R10),value(arg(i,1),i.type),value_type(arg(i,1),i.type));
    emit(i.opcode == Opcode::AtomicExchange ? Op::Exchange : Op::Xadd,i.type,{address,Operand::r(XR_R10)});
    // LowIR atomic_add_fetch returns the updated value (unlike xadd).
    if (i.opcode == Opcode::AtomicAddFetch)
        emit(Op::Add,i.type,{Operand::r(XR_R10),value(arg(i,1),i.type)});
    normalize_register(Operand::r(XR_R10),i.type);
    if (i.destination) move(allocate(i.destination.index,i.type),Operand::r(XR_R10),i.type);
}
} // namespace native
