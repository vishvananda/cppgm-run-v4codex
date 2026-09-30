#include "native/model.h"
#include "native/abi.h"
#include <ostream>
#include <iomanip>
#include <limits>
#include <cmath>
namespace native {
static void operand(const lowir_model::Program& p, Operand o, std::ostream& out)
{
    switch (o.kind) {
    case Operand::Reg: out << register_name(o.reg); break;
    case Operand::Floating: {
        long double value = o.floating_value();
        unsigned quiet_bit = o.id == Type::F32 ? 22 : o.id == Type::F64 ? 51 : 62;
        if (std::isnan(value) && !(o.bits & (std::uint64_t(1)<<quiet_bit)))
            out << (std::signbit(value) ? "-snan" : "snan");
        else out << std::setprecision(std::numeric_limits<long double>::max_digits10) << value;
        break;
    }
    case Operand::WideImmediate: {
        auto n = lowir_model::Operand::integer(o.bits); n.integer_high(o.displacement);
        out << lowir_model::integer_text(n); break;
    }
    case Operand::Immediate: out << std::int64_t(o.bits); break;
    case Operand::Symbol:
        if (o.id > p.symbols.size()) out << runtime_name(o.id-p.symbols.size()-1);
        else out << p.name(p.symbols[o.id-1].name);
        if (o.displacement) out << (o.displacement > 0 ? "+" : "") << o.displacement;
        break;
    case Operand::Label:
        if (o.id && o.id <= p.blocks.size() && p.blocks[o.id-1].name) out << p.name(p.blocks[o.id-1].name);
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
        "copy_bytes","zero_bytes","mfence","lock_xadd","xchg","lock_cmpxchg",
        "adc","sbb","mul","shld","shrd","lock_cmpxchg16b",
        "fmov","fadd","fsub","fmul","fdiv","fneg","fcmp","fset",
        "sitofp","uitofp","fptosi","fptoui","fpext","fptrunc","fret","fstp",
        "eh_push","eh_pop","eh_dispatch","throw","resume","stack_alloc","tls_addr","syscall"};
    // The scalar Boolean materialization has the canonical byte-to-register
    // spelling. Its typed ExtendUnsigned fact is also consumed by encoding.
    bool boolean_extend = i.op == Op::ExtendUnsigned && i.type == Type::U8;
    out << "    " << (boolean_extend ? "movzx" : names[unsigned(i.op)]);
    if (i.op == Op::Jcc || i.op == Op::Set || i.op == Op::Fset) out << cc(i.condition);
    bool typed = i.op == Op::Load || i.op == Op::Store || i.op == Op::Compare ||
        i.op == Op::ExtendSigned || (i.op == Op::ExtendUnsigned && !boolean_extend) || i.op == Op::Xadd || i.op == Op::Exchange || i.op == Op::Cmpxchg;
    if (i.op >= Op::Sitofp && i.op <= Op::Fptrunc) out << '.' << type_name(i.source_type);
    if ((i.op >= Op::Fmov && i.op <= Op::Fpop) || i.op == Op::Throw) typed = true;
    if (typed) out << '.' << type_name(i.type);
    if (i.op == Op::CopyBytes || i.op == Op::ZeroBytes) out << ' ' << i.bytes << 'x' << i.alignment;
    for (unsigned n = 0; n < i.count; ++n) {
        out << (n || i.op == Op::CopyBytes || i.op == Op::ZeroBytes ? ", " : " ");
        if (i.op == Op::Call && i.args[n].kind != Operand::Symbol) out << '*';
        operand(p,i.args[n],out);
    }
    if (i.op >= Op::Fadd && i.op <= Op::Fdiv && i.source_type == Type::F80) out << " [eval=f80]";
    if (i.op == Op::Fmul && i.source_type == i.type && i.type != Type::F80) out << " [eval=" << type_name(i.type) << ", exact=scale]";
    if (i.op == Op::Call || i.op == Op::Syscall) {
        out << " [args=(";
        bool first = true;
        for (unsigned reg = 0; reg < 32; ++reg) if (i.arg_registers & (1u<<reg)) {
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
void dump_header(const lowir_model::Program& p, const std::vector<Instruction>& start, std::ostream& out, bool exceptions)
{
    out << "machine_ir x86_64 linux\n";
    if (!start.empty()) {
        out << "\nstartup\n";
        for (const auto& i : start) instruction(p,i,out);
    }
    for (const auto& g : p.globals) if (!g.declaration) {
        out << "\nglobal " << p.name(p.symbols[g.symbol.index-1].name);
        if (p.symbols[g.symbol.index-1].metadata.storage == ir_model::GSM_THREAD_LOCAL) out << " thread_local";
        out << "\n  storage ";
        if (g.structured) {
            out << "data";
            if (g.type.kind() == Type::Object) out << " layout=" << g.type.bytes() << 'x' << g.type.alignment();
            out << '\n';
        }
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
                    value = Operand::floating(d.value,d.type,&p).floating_value();
                    if (d.value.signaling_nan) out << (std::signbit(value) ? "-snan" : "snan");
                    else out << std::setprecision(std::numeric_limits<long double>::max_digits10) << value;
                }
                else if (d.type == Type::I128) out << lowir_model::integer_text(d.value);
                else out << std::int64_t(d.value.data.integer);
            }
            out << '\n';
        }
    }
    if (exceptions) {
        out << "\nglobal " << runtime_name(unsigned(RuntimeEntity::ExceptionTop))
            << "\n  storage scalar ptr\n  init ptr 0\n";
        out << "\nglobal " << runtime_name(unsigned(RuntimeEntity::ExceptionValue))
            << "\n  storage data\n  item zero 16\n";
    }
}
void dump_function(const lowir_model::Program& p, const Function& f, std::ostream& out)
{
    out << "\nfunction " << p.name(p.symbols[f.symbol.index-1].name); debug(p,f.debug,out);
    for (unsigned h = 0; h < f.exception_handlers.size(); ++h) {
        const auto& handler = f.exception_handlers[h];
        out << "\n  handler " << h << " cleanup=" << handler.cleanup;
        for (unsigned n = handler.clauses.begin; n < handler.clauses.end(); ++n) {
            const auto& clause = f.exception_clauses[n];
            out << (clause.filtered ? "\n    filter" : "\n    catch ");
            if (clause.filtered) {
                for (unsigned i = clause.filter.begin; i < clause.filter.end(); ++i)
                    out << ' ' << p.name(p.symbols[f.exception_filter_types[i].index-1].name);
            } else if (clause.type) out << p.name(p.symbols[clause.type.index-1].name); else out << "all";
            out << " selector=" << (clause.filtered ? std::int64_t(std::int32_t(clause.selector)) : std::int64_t(clause.selector))
                << " binding=" << unsigned(clause.binding);
            if (f.host) out << " host_selector=" << clause.host_selector;
        }
    }
    out << "\n  abi\n";
    for (const auto& param : f.params) {
        out << "    param " << (param.name ? p.name(param.name) : "%native_arg") << " -> "; operand(p,param.location,out);
        out << " : " << type_name(param.second.kind != Operand::None ? Type::I64 : param.type) << '\n';
        if (param.second.kind != Operand::None) {
            out << "    param " << p.name(param.name) << ".1 -> "; operand(p,param.second,out); out << " : i64\n";
        }
    }
    out << "    return " << type_name(f.result) << " -> " << (f.result == Type() ? "void" : f.result == Type::F80 ? "st0" : f.result.floating() ? "xmm0" : "rax") << '\n';
    out << "  frame\n    stack_size " << f.stack_size << "\n    scratch_bytes " << f.scratch_bytes << "\n    frame_pointer " << (f.frame_pointer ? "keep" : "omit")
        << "\n    epilogues " << (f.shared_epilogue ? "shared" : "direct") << '\n';
    if (f.host_exception.kind != Operand::None) {
        out << "    host_exception "; operand(p,f.host_exception,out); out << '\n';
        out << "    host_selector "; operand(p,f.host_selector,out); out << '\n';
        out << "    host_raw_selector "; operand(p,f.host_raw_selector,out); out << '\n';
    }
    if (f.exception_base.kind != Operand::None) {
        out << "    exception_base "; operand(p,f.exception_base,out); out << '\n';
    }
    if (f.stack_floor.kind != Operand::None) {
        out << "    stack_floor "; operand(p,f.stack_floor,out); out << '\n';
    }
    if (f.frame_base != XR_RBP)
        out << "    frame_base " << register_name(f.frame_base) << "\n    frame_alignment " << f.frame_alignment << '\n';
    if (f.preserved) {
        out << "    preserve";
        for (unsigned r = 0; r < 16; ++r) if (f.preserved & (1u<<r)) out << ' ' << register_name(r);
        out << '\n';
    }
    for (const auto& b : f.frame) {
        out << (b.parameter ? "    param-slot " : b.temporary ? "    temp " : "    slot ");
        if (b.name) out << p.name(b.name); else out << "%native" << -b.offset;
        out << " -> "; operand(p,Operand::mem(f.frame_base,b.offset),out); out << " : " << type_name(b.type) << '\n';
    }
    for (unsigned bi = 0; bi < f.blocks.size(); ++bi) {
        const auto& b = f.blocks[bi];
        out << "\n  block "; operand(p,Operand::label(b.id),out); out << '\n';
        if (f.host && f.host_outer[bi]) {
            out << "    host_outer "; operand(p,Operand::label(f.blocks[f.host_outer[bi]-1].id),out); out << '\n';
        }
        for (unsigned n = b.instructions.begin; n != b.instructions.end(); ++n) {
            const auto& i = f.instructions[n];
            if (f.host && ((i.op == Op::Call && i.boundary.unwind != ir_model::CUM_NO) || i.op == Op::Resume)) {
                out << "    host_eh ";
                if (i.host_handler == ~0u) out << "unreachable";
                else if (!i.host_handler) out << "unprotected";
                else { out << "landing "; operand(p,Operand::label(i.host_handler),out); }
                out << '\n';
            }
            instruction(p,i,out);
        }
    }
}
} // namespace native
