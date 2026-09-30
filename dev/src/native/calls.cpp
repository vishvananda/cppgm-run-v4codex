#include "native/selection.h"
#include "native/abi.h"
namespace native {
using namespace lowir_model;
void Selector::call(const lowir_model::Instruction& i)
{
    SignatureId signature_id = i.signature;
    auto target_input = arg(i,0);
    if (!signature_id && target_input.kind == lowir_model::Operand::Symbol) {
        const auto& symbol = p.symbols[target_input.ref-1];
        signature_id = p.functions[symbol.entity-1].signature;
    }
    require(bool(signature_id), "call has no ABI signature");
    const auto& signature = p.signatures[signature_id.index-1];
    require(signature.result == Type() || signature.result.scalar() || signature.result.kind() == Type::Object, "invalid call result class");
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
    struct Assignment { Operand to, from; Type type; Operand address_home; bool done = false; };
    std::vector<Assignment> moves;
    std::vector<Assignment> stack_moves;
    AbiCursor abi;
    Operand result_home;
    if (aggregate(i.type)) {
        result_home = i.destination ? allocate(i.destination.index,i.type) : home(0,i.type,true);
        if (indirect_return(i.type)) {
            Assignment m; m.to = Operand::r(XR_RDI); m.from = result_home;
            m.from.address = true; m.type = Type::Ptr; moves.push_back(m); abi.gp = 1;
        }
    }
    for (unsigned n = 0; n < count; ++n) {
        const auto& a = arg(i,n+1);
        bool fixed = n < signature.parameters.count;
        Type t = fixed ? p.parameters[signature.parameters.begin+n].type : value_type(a,Type::I64);
        if (!fixed && a.kind == lowir_model::Operand::Floating) t = Type::F64;
        require(t.scalar() || t.kind() == Type::Object, "invalid argument ABI class");
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
        auto placement = abi.take(t,XR_RSP);
        if (aggregate(t)) {
            if (t.kind() == Type::Object && actual.kind() != Type::Object) from = memory(a);
            if (placement.memory) {
                Assignment m; m.from = from; m.to = placement.parts[0]; m.type = t;
                stack_moves.push_back(m);
            } else for (unsigned part = 0; part < placement.count; ++part) {
                Assignment m; m.from = fragment(from,part*8); m.to = placement.parts[part];
                m.type = chunk_type(std::min(8u,t.bytes()-part*8)); moves.push_back(m);
            }
            continue;
        }
        Assignment m; m.from = from; m.type = actual;
        m.to = placement.parts[0];
        if (placement.memory) { stack_moves.push_back(m); continue; }
        moves.push_back(m);
    }
    bool bulk_stack = false;
    for (const auto& m : stack_moves) if (m.type.kind() == Type::Object &&
        m.type.bytes() > 32 && (m.type.bytes() > 64 || m.type.alignment() < 8)) bulk_stack = true;
    // REP copies clobber ABI argument carriers. Snapshot each pending dependency
    // before the first copy, including addresses of subsequent stack objects.
    auto capture = [&](Assignment& m) {
        if (m.from.kind == Operand::Immediate || m.from.kind == Operand::WideImmediate || m.from.kind == Operand::Floating ||
            m.from.kind == Operand::Symbol || (m.from.kind == Operand::Memory && m.from.reg == XR_RBP)) return;
        if (m.type.kind() == Type::Object) {
            m.address_home = home(0,Type::Ptr,true);
            auto address = m.from; address.address = true; move(m.address_home,address,Type::Ptr);
        } else {
            auto saved = home(0,m.type,true); move(saved,m.from,m.type); m.from = saved;
        }
    };
    if (bulk_stack) { for (auto& m : moves) capture(m); for (auto& m : stack_moves) capture(m); }
    Operand target_home;
    if (bulk_stack && target.kind == Operand::Reg) { target_home = home(0,Type::Ptr,true); move(target_home,target,Type::Ptr); }
    std::uint64_t stack = (abi.stack+15)&~std::uint64_t(15);
    if (stack) emit(Op::Sub,Type::I64,{Operand::r(XR_RSP),Operand::imm(stack)});
    for (const auto& m : stack_moves) {
        auto from = m.from;
        if (m.address_home.kind != Operand::None) {
            move(Operand::r(XR_R10),m.address_home,Type::Ptr); from = Operand::mem(XR_R10);
        }
        move(m.to,from,m.type);
    }
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
        emit(Op::Mov,Type::I64,{Operand::r(XR_RAX),Operand::imm(abi.fp)});
    if (target_home.kind != Operand::None) move(target,target_home,Type::Ptr);
    auto& site = emit(Op::Call,i.type,{target});
    site.bytes = stack; site.boundary = signature.boundary;
    for (const auto& m : moves) site.arg_registers |= 1u << m.to.reg;
    if (stack) emit(Op::Add,Type::I64,{Operand::r(XR_RSP),Operand::imm(stack)});
    if (i.destination && state(i.destination.index).uses) {
        auto dest = allocate(i.destination.index,i.type);
        if (aggregate(i.type)) {
            if (!indirect_return(i.type)) {
                move(fragment(dest,0),Operand::r(XR_RAX),chunk_type(std::min(8u,i.type.bytes())));
                if (i.type.bytes() > 8) move(fragment(dest,8),Operand::r(XR_RDX),chunk_type(i.type.bytes()-8));
            }
        }
        else if (i.type == Type::F80) { f.scratch_bytes = 48; emit(Op::Fpop,i.type,{dest}); }
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
    if (i.type == Type::I128) { wide_atomic(i); return; }
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
