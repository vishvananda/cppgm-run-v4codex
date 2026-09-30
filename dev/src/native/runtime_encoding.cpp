#include "native/encoding.h"
namespace native {
void Encoder::runtime(const Instruction& i)
{
    // Cleanup records stay installed while their landing pad runs. Resume
    // retires an active cleanup; eh.end can retire it explicitly first.
    const unsigned record_bytes = 96;
    const int saved[] = {XR_RBP,XR_RBX,XR_R12,XR_R13,XR_R14,XR_R15};
    auto top = image.runtime(RuntimeEntity::ExceptionTop);
    auto r10 = Operand::r(XR_R10), r11 = Operand::r(XR_R11);
    if (i.op == Op::EhDispatch) {
        const auto& h = function->exception_handlers.at(i.args[0].bits);
        auto selector = image.runtime(RuntimeEntity::ExceptionSelector);
        store(selector,Operand::imm(0),Type::I32);
        std::vector<std::size_t> done;
        for (unsigned n = h.clauses.begin; n < h.clauses.end(); ++n) {
            const auto& c = function->exception_clauses[n];
            std::size_t miss = 0;
            if (c.type) {
                mov(Operand::r(XR_RDI),Operand::symbol(c.type));
                mov(Operand::r(XR_RSI),Operand::imm(unsigned(c.binding)));
                load(r11,image.runtime(RuntimeEntity::ExceptionMatcher),Type::Ptr,false); call(r11);
                form(0x85,32,XR_RAX,Operand::r(XR_RAX)); miss = local_jump(XC_E);
            }
            store(selector,Operand::imm(c.selector),Type::I32);
            done.push_back(local_jump(-1));
            if (miss) local_target(miss);
            if (!c.type) break;
        }
        if (!h.cleanup) { Instruction resume(Op::Resume); runtime(resume); }
        for (auto fix : done) local_target(fix);
        return;
    }
    if (i.op == Op::StackAlloc) {
        form(0x83,64,0,r10,1,15);
        // Wrapped sizes cannot silently allocate less than the requested size.
        auto valid = local_jump(XC_AE); byte(0x0f); byte(0x0b); local_target(valid);
        form(0x83,64,4,r10,1,std::uint64_t(-16));
        form(0x29,64,XR_R10,Operand::r(XR_RSP));
        auto fits = local_jump(XC_AE); byte(0x0f); byte(0x0b); local_target(fits);
        if (function->stack_floor.kind != Operand::None)
            store(function->stack_floor,Operand::r(XR_RSP),Type::Ptr);
        mov(i.args[0],Operand::r(XR_RSP)); return;
    }
    if (i.op == Op::EhPush) {
        load(r11,top,Type::Ptr,false);
        form(0x83,64,5,Operand::r(XR_RSP),1,record_bytes);
        store(Operand::mem(XR_RSP),r11,Type::Ptr);
        // RIP-relative handler address uses the same final-label patch table
        // as branches; no textual symbol or additional assembler is involved.
        byte(0x4c); byte(0x8d); byte(0x1d);
        branches.push_back({code.size(),i.args[0].id}); number(0,4);
        store(Operand::mem(XR_RSP,8),r11,Type::Ptr);
        form(0x8d,64,XR_R11,Operand::mem(XR_RSP,record_bytes));
        store(Operand::mem(XR_RSP,16),r11,Type::Ptr);
        for (unsigned k = 0; k < 6; ++k)
            store(Operand::mem(XR_RSP,24+8*k),Operand::r(saved[k]),Type::I64);
        if (function->stack_floor.kind != Operand::None) {
            form(0x8d,64,XR_R11,function->stack_floor);
            store(Operand::mem(XR_RSP,72),r11,Type::Ptr);
        } else store(Operand::mem(XR_RSP,72),Operand::imm(0),Type::Ptr);
        store(Operand::mem(XR_RSP,80),i.args[1],Type::I64);
        store(Operand::mem(XR_RSP,88),Operand::imm(0),Type::I64);
        store(top,Operand::r(XR_RSP),Type::Ptr); return;
    }
    load(r11,top,Type::Ptr,false);
    if (i.op == Op::Resume) {
        form(0x85,64,XR_R11,r11); auto absent = local_jump(XC_E);
        form(0x83,64,7,Operand::mem(XR_R11,88),1,0); auto inactive = local_jump(XC_E);
        load(r11,Operand::mem(XR_R11),Type::Ptr,false); store(top,r11,Type::Ptr);
        local_target(inactive); local_target(absent);
    }
    form(0x85,64,XR_R11,r11);
    auto available = local_jump(XC_NE);
    // A throw without a handler terminates this standalone process. Host C++
    // termination/unwinding is the separate PA26 object-runtime boundary.
    mov(Operand::r(XR_RDI),Operand::imm(1));
    mov(Operand::r(XR_RAX),Operand::imm(60)); byte(0x0f); byte(0x05);
    byte(0x0f); byte(0x0b); local_target(available);
    if (i.op == Op::EhPop) {
        load(r10,Operand::mem(XR_R11),Type::Ptr,false); store(top,r10,Type::Ptr);
        // Dynamic allocations within the region live until function return.
        // Reclaim the record immediately only when nothing was allocated below.
        form(0x39,64,XR_R11,Operand::r(XR_RSP));
        auto allocated = local_jump(XC_NE);
        load(Operand::r(XR_RSP),Operand::mem(XR_R11,16),Type::Ptr,false);
        local_target(allocated); return;
    }
    lowir_model::require(i.op == Op::Throw || i.op == Op::Resume,"invalid runtime transfer");
    form(0x83,64,7,Operand::mem(XR_R11,88),1,0);
    auto not_active = local_jump(XC_E);
    mov(Operand::r(XR_RDI),Operand::imm(1)); mov(Operand::r(XR_RAX),Operand::imm(60)); byte(0x0f); byte(0x05);
    local_target(not_active);
    form(0x83,64,7,Operand::mem(XR_R11,80),1,0);
    auto keep = local_jump(XC_NE);
    load(r10,Operand::mem(XR_R11),Type::Ptr,false); store(top,r10,Type::Ptr);
    local_target(keep);
    store(Operand::mem(XR_R11,88),Operand::imm(1),Type::I64);
    load(r10,Operand::mem(XR_R11,8),Type::Ptr,false);
    for (unsigned k = 0; k < 6; ++k)
        load(Operand::r(saved[k]),Operand::mem(XR_R11,24+8*k),Type::I64,false);
    load(Operand::r(XR_RSP),Operand::mem(XR_R11,16),Type::Ptr,false);
    form(0x83,64,7,Operand::mem(XR_R11,80),1,0);
    auto popped = local_jump(XC_E); mov(Operand::r(XR_RSP),r11); local_target(popped);
    // Allocations in the destination function retain their function lifetime,
    // including ones made after handler registration. The record holds the
    // address of that frame's floor, independent of the unwound current frame.
    load(r11,Operand::mem(XR_R11,72),Type::Ptr,false);
    form(0x85,64,XR_R11,r11);
    auto no_allocations = local_jump(XC_E);
    load(r11,Operand::mem(XR_R11),Type::Ptr,false);
    form(0x39,64,XR_R11,Operand::r(XR_RSP));
    auto already_below = local_jump(XC_BE);
    mov(Operand::r(XR_RSP),r11);
    local_target(already_below); local_target(no_allocations);
    form(0xff,64,4,r10);
}
} // namespace native
