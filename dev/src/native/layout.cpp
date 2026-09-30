#include "native/encoding.h"
#include <algorithm>
#include <climits>
namespace native {
using lowir_model::require;
void Encoder::startup(const std::vector<Instruction>& instructions)
{
    if (!instructions.empty() && image.has_tls) tls_startup();
    // Linux process entry starts with an aligned stack. Native functions follow
    // SysV's call-entry alignment after each direct call pushes a return address.
    for (const auto& i : instructions) instruction(i);
}
void Encoder::epilogue_code()
{
    if (function->exception_base.kind != Operand::None) {
        load(Operand::r(XR_R11),function->exception_base,Type::Ptr,false);
        store(image.runtime(RuntimeEntity::ExceptionTop),Operand::r(XR_R11),Type::Ptr);
    }
    unsigned n = 0;
    for (unsigned reg = 0; reg < 16; ++reg) if (function->preserved & (1u << reg)) {
        auto slot = Operand::mem(XR_RBP,-std::int64_t((function->frame_base == XR_RBP ? function->frame_bytes : 0)+8*++n));
        load(Operand::r(reg),slot,Type::I64,false);
    }
    if (function->frame_pointer) { byte(0xc9); }
    else if (function->stack_size) form(0x81,64,0,Operand::r(XR_RSP),4,function->stack_size);
    byte(0xc3);
}
void Encoder::encode(const Function& f)
{
    function = &f;
    image.symbols[f.symbol.index] = code.size(); image.defined[f.symbol.index] = true;
    if (f.frame_pointer) { byte(0x55); form(0x89,64,XR_RSP,Operand::r(XR_RBP)); }
    if (f.stack_size) form(0x81,64,5,Operand::r(XR_RSP),4,f.stack_size);
    unsigned n = 0;
    for (unsigned reg = 0; reg < 16; ++reg) if (f.preserved & (1u << reg))
        store(Operand::mem(XR_RBP,-std::int64_t((f.frame_base == XR_RBP ? f.frame_bytes : 0)+8*++n)),Operand::r(reg),Type::I64);
    if (f.frame_base != XR_RBP) {
        form(0x8d,64,f.frame_base,Operand::mem(XR_RBP,-std::int64_t(8*n)));
        form(0x81,64,4,Operand::r(f.frame_base),4,-std::uint64_t(f.frame_alignment));
    }
    if (f.exception_base.kind != Operand::None) {
        load(Operand::r(XR_R11),image.runtime(RuntimeEntity::ExceptionTop),Type::Ptr,false);
        store(f.exception_base,Operand::r(XR_R11),Type::Ptr);
    }
    if (f.stack_floor.kind != Operand::None) store(f.stack_floor,Operand::r(XR_RSP),Type::Ptr);
    epilogue = 0;
    for (const auto& b : f.blocks) epilogue = std::max(epilogue,b.id+1);
    if (labels.size() <= epilogue) labels.resize(epilogue+1);
    branches.clear();
    for (unsigned k = 0; k < f.blocks.size(); ++k) {
        const auto& block = f.blocks[k]; labels[block.id] = code.size();
        for (unsigned j = block.instructions.begin; j != block.instructions.end(); ++j) {
            const auto& i = f.instructions[j];
            // The fallthrough target is already selected; suppressing its jump
            // changes neither MIR control flow nor label identity.
            if (i.op == Op::Jump && k+1 < f.blocks.size() && i.args[0].id == f.blocks[k+1].id) continue;
            instruction(i);
        }
    }
    labels[epilogue] = code.size();
    if (f.shared_epilogue) epilogue_code();
    for (const auto& fix : branches) {
        std::int64_t relative = std::int64_t(labels.at(fix.label)) - std::int64_t(fix.offset+4);
        require(relative >= INT32_MIN && relative <= INT32_MAX, "native branch out of range");
        for (unsigned k = 0; k < 4; ++k) code[fix.offset+k] = std::uint64_t(relative) >> (8*k);
    }
}
std::vector<Instruction> startup(const lowir_model::Program& p)
{
    SymbolId entry;
    std::vector<SymbolId> init, fini;
    for (const auto& f : p.functions) if (!f.declaration) {
        auto role = p.symbols[f.symbol.index-1].metadata.role;
        if (role == ir_model::SR_ENTRY) { require(!entry, "multiple native entry functions"); entry = f.symbol; }
        if (role == ir_model::SR_INIT) init.push_back(f.symbol);
        if (role == ir_model::SR_FINI) fini.push_back(f.symbol);
    }
    std::vector<Instruction> result;
    if (!entry) return result;
    auto call = [&](SymbolId symbol) {
        Instruction i(Op::Call); i.args[0] = Operand::symbol(symbol); i.count = 1; result.push_back(i);
    };
    auto mov = [&](int to, int from) {
        Instruction i(Op::Mov); i.args[0] = Operand::r(to); i.args[1] = Operand::r(from); i.count = 2; result.push_back(i);
    };
    for (auto s : init) call(s);
    const auto& entry_function = p.functions[p.symbols[entry.index-1].entity-1];
    const auto& entry_signature = p.signatures[entry_function.signature.index-1];
    require(entry_signature.parameters.count <= 2,"entry accepts at most argc and argv");
    if (entry_signature.parameters.count) {
        Instruction argc(Op::Load,p.parameters[entry_signature.parameters.begin].type);
        argc.args[0] = Operand::r(XR_RDI); argc.args[1] = Operand::mem(XR_RSP); argc.count = 2; result.push_back(argc);
    }
    if (entry_signature.parameters.count == 2) {
        Instruction argv(Op::Lea,Type::Ptr);
        argv.args[0] = Operand::r(XR_RSI); argv.args[1] = Operand::mem(XR_RSP,8); argv.count = 2; result.push_back(argv);
    }
    call(entry);
    if (entry_signature.parameters.count) result.back().arg_registers |= 1u<<XR_RDI;
    if (entry_signature.parameters.count == 2) result.back().arg_registers |= 1u<<XR_RSI;
    if (!fini.empty()) {
        mov(XR_R12,XR_RAX);
        for (auto i = fini.rbegin(); i != fini.rend(); ++i) call(*i);
        mov(XR_RDI,XR_R12);
    } else mov(XR_RDI,XR_RAX);
    result.push_back(Instruction(Op::Exit));
    return result;
}
} // namespace native
