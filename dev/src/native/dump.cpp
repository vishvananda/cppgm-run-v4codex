#include "native/model.h"
#include <ostream>
#include <iomanip>
#include <limits>
#include <cmath>
namespace native {
static void operand(const lowir_model::Program& p, Operand o, std::ostream& out)
{
    switch (o.kind) {
    case Operand::Reg: out << register_name(o.reg); break;
    case Operand::Immediate: out << std::int64_t(o.bits); break;
    case Operand::Symbol:
        out << p.name(p.symbols[o.id-1].name);
        if (o.displacement) out << (o.displacement > 0 ? "+" : "") << o.displacement;
        break;
    case Operand::Label:
        if (o.id <= p.blocks.size()) out << p.name(p.blocks[o.id-1].name);
        else out << "^native" << o.id;
        break;
    case Operand::Memory:
        out << '[' << register_name(o.reg);
        if (o.index >= 0) { out << '+' << register_name(o.index); if (o.scale != 1) out << '*' << o.scale; }
        if (o.displacement) out << (o.displacement > 0 ? "+" : "") << o.displacement;
        out << ']'; break;
    default: break;
    }
}
static void debug(const lowir_model::Program& p, const DebugLocation& d, std::ostream& out)
{
    if (d.file) out << " !dbg(" << p.name(d.file) << ", " << d.line << ", " << d.column << ')';
}
static const char* cc(X86Condition c)
{
    static const char* const names[] = {"o","no","b","ae","e","ne","be","a","s","ns","p","np","l","ge","le","g"};
    return names[c];
}
static void instruction(const lowir_model::Program& p, const Instruction& i, std::ostream& out)
{
    static const char* const names[] = {"mov","load","store","lea","add","sub","imul","and","or","xor","neg","not","bswap",
        "cmp","test","set","sext","zext","cqo","idiv","div","shl","shr","sar","jmp","j","call","ret","exit","ud2",
        "copy_bytes","zero_bytes","mfence","lock_xadd","xchg","lock_cmpxchg"};
    out << "    " << names[unsigned(i.op)];
    if (i.op == Op::Jcc || i.op == Op::Set) out << cc(i.condition);
    bool typed = i.op == Op::Load || i.op == Op::Store || i.op == Op::Compare ||
        i.op == Op::ExtendSigned || i.op == Op::ExtendUnsigned || i.op == Op::Xadd || i.op == Op::Exchange || i.op == Op::Cmpxchg;
    if (typed) out << '.' << type_name(i.type);
    if (i.op == Op::CopyBytes || i.op == Op::ZeroBytes) out << ' ' << i.bytes << 'x' << i.alignment;
    for (unsigned n = 0; n < i.count; ++n) {
        out << (n || i.op == Op::CopyBytes || i.op == Op::ZeroBytes ? ", " : " ");
        if (i.op == Op::Call && i.args[n].kind != Operand::Symbol) out << '*';
        operand(p,i.args[n],out);
    }
    if (i.op == Op::Call) {
        out << " [args=(";
        bool first = true;
        for (unsigned reg = 0; reg < 16; ++reg) if (i.arg_registers & (1u<<reg)) {
            if (!first) out << ',';
            first = false; out << register_name(reg);
        }
        out << ')';
        if (i.bytes) out << ", stack=" << i.bytes;
        if (i.boundary.arity == ir_model::CAM_VARIADIC) out << ", variadic";
        if (i.boundary.unwind == ir_model::CUM_NO) out << ", unwind=no";
        if (i.boundary.returns == ir_model::CRM_NORETURN) out << ", returns=noreturn";
        out << ']';
    }
    debug(p,i.debug,out); out << '\n';
}
void dump_header(const lowir_model::Program& p, const std::vector<Instruction>& start, std::ostream& out)
{
    out << "machine_ir x86_64 linux\n";
    if (!start.empty()) {
        out << "\nstartup\n";
        for (const auto& i : start) instruction(p,i,out);
    }
    for (const auto& g : p.globals) if (!g.declaration) {
        out << "\nglobal " << p.name(p.symbols[g.symbol.index-1].name) << "\n  storage ";
        if (g.structured) out << "data\n";
        else out << "scalar " << type_name(g.type) << '\n';
        for (unsigned n = g.data.begin; n != g.data.end(); ++n) {
            const auto& d = p.data[n];
            out << (g.structured ? "  item " : "  init ");
            if (d.kind == lowir_model::DataItem::Zero) out << "zero" << (g.structured ? " "+std::to_string(d.zero_bytes) : "");
            else {
                out << type_name(d.type) << ' ';
                if (d.kind == lowir_model::DataItem::Address) {
                    out << "addr " << p.name(p.symbols[d.symbol.index-1].name);
                    if (d.addend) out << (d.addend > 0 ? "+" : "") << d.addend;
                } else if (d.type.floating()) {
                    long double value = d.value.kind == lowir_model::Operand::Floating ? d.value.data.floating :
                        d.value.negative_integer ? static_cast<long double>(std::int64_t(d.value.data.integer)) :
                        static_cast<long double>(d.value.data.integer);
                    if (d.type == Type::F32) value = static_cast<float>(value);
                    if (d.type == Type::F64) value = static_cast<double>(value);
                    if (d.value.signaling_nan) out << (std::signbit(value) ? "-snan" : "snan");
                    else out << std::setprecision(std::numeric_limits<long double>::max_digits10) << value;
                }
                else out << std::int64_t(d.value.data.integer);
            }
            out << '\n';
        }
    }
}
void dump_function(const lowir_model::Program& p, const Function& f, std::ostream& out)
{
    out << "\nfunction " << p.name(p.symbols[f.symbol.index-1].name); debug(p,f.debug,out);
    out << "\n  abi\n";
    for (const auto& param : f.params) {
        out << "    param " << p.name(param.name) << " -> "; operand(p,param.location,out);
        out << " : " << type_name(param.type) << '\n';
    }
    out << "    return " << type_name(f.result) << " -> " << (f.result == Type() ? "void" : "rax") << '\n';
    out << "  frame\n    stack_size " << f.stack_size << "\n    scratch_bytes 0\n    frame_pointer " << (f.frame_pointer ? "keep" : "omit")
        << "\n    epilogues " << (f.shared_epilogue ? "shared" : "direct") << '\n';
    if (f.preserved) {
        out << "    preserve";
        for (unsigned r = 0; r < 16; ++r) if (f.preserved & (1u<<r)) out << ' ' << register_name(r);
        out << '\n';
    }
    for (const auto& b : f.frame) {
        out << (b.temporary ? "    temp " : "    slot ");
        if (b.name) out << p.name(b.name); else out << "%native" << -b.offset;
        out << " -> "; operand(p,Operand::mem(XR_RBP,b.offset),out); out << " : " << type_name(b.type) << '\n';
    }
    for (const auto& b : f.blocks) {
        out << "\n  block "; operand(p,Operand::label(b.id),out); out << '\n';
        for (unsigned n = b.instructions.begin; n != b.instructions.end(); ++n) instruction(p,f.instructions[n],out);
    }
}
} // namespace native
