#include "native/encoding.h"
#include "support/id_index.h"
namespace native {
void Encoder::host_runtime(const Instruction& i)
{
    if (i.op == Op::EhPush || i.op == Op::EhPop || i.op == Op::EhDispatch) return;
    if (i.op == Op::Resume) {
        if (i.host_handler && i.host_handler != ~0u) {
            load(Operand::r(XR_RAX),function->host_exception,Type::Ptr,false);
            load(Operand::r(XR_RDX),function->host_raw_selector,Type::I32,false);
            branch(host_pad_labels.at(i.host_handler));
        } else branch(resume_label);
        return;
    }
    lowir_model::require(false,"unsupported raw host exception transfer");
}
void Encoder::host_landing_pads()
{
    const auto& f = *function;
    for (unsigned target : host_landings) {
        host_landing_offsets[target] = code.size();
        auto label = host_pad_labels[f.blocks[target].id]; labels[label] = code.size(); label_owners[label] = f.symbol;
        store(f.host_exception,Operand::r(XR_RAX),Type::Ptr);
        if (f.host_outer[target]) store(f.host_raw_selector,Operand::r(XR_RDX),Type::I32);
        // SysV landing registers hold the exception and an LSDA-local selector.
        // Source selectors remain unchanged throughout the shared LowIR graph.
        store(f.host_selector,Operand::imm(0),Type::I32);
        const auto& block = f.blocks[target];
        if (block.instructions.count && f.instructions[block.instructions.begin].op == Op::EhDispatch) {
            const auto& h = f.exception_handlers[f.instructions[block.instructions.begin].args[0].bits];
            for (unsigned n = h.clauses.begin; n < h.clauses.end(); ++n) {
                const auto& c = f.exception_clauses[n];
                form(0x81,32,7,Operand::r(XR_RDX),4,c.host_selector);
                auto next = local_jump(XC_NE);
                store(f.host_selector,Operand::imm(c.selector),Type::I32);
                local_target(next);
            }
        }
        // Outgoing stack arguments of the throwing call are no longer live.
        if (f.stack_floor.kind != Operand::None) load(Operand::r(XR_RSP),f.stack_floor,Type::Ptr,false);
        else form(0x8d,64,XR_RSP,Operand::mem(XR_RBP,-std::int64_t(f.stack_size)));
        branch(block.id);
    }
    if (resume_label) {
        labels[resume_label] = code.size(); label_owners[resume_label] = f.symbol;
        load(Operand::r(XR_RDI),f.host_exception,Type::Ptr,false);
        auto begin = code.size(); call(Operand::symbol(SymbolId(image.host_resume)));
        host_sites.push_back({begin,code.size(),0}); byte(0x0f); byte(0x0b);
    }
}
void Encoder::cfi_advance(std::size_t from, std::size_t to)
{
    unwind_record.cfi_pc = to;
    auto n = to-from; auto& cfi = unwind_record.cfi;
    if (n < 64) cfi.push_back(0x40+n);
    else { cfi.push_back(4); for (unsigned k = 0; k < 4; ++k) cfi.push_back(n>>(8*k)); }
}
} // namespace native
