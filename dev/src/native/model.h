#pragma once
#include "lowir/model.h"
#include "native/mir/registers.h"
#include <array>
#include <iosfwd>

namespace native {
using lowir_model::Type;
using lowir_model::Name;
using lowir_model::SymbolId;
using lowir_model::DebugLocation;
struct Operand {
    enum Kind { None, Reg, Immediate, Memory, Symbol, Label, Floating, WideImmediate } kind = None;
    int reg = -1, index = -1;
    unsigned scale = 1;
    std::int64_t displacement = 0;
    std::uint64_t bits = 0;
    std::uint32_t id = 0;
    bool address = false, temporary = false;
    static Operand r(int n);
    static Operand imm(std::uint64_t n);
    static Operand mem(int base, std::int64_t offset = 0);
    static Operand symbol(SymbolId id, bool address = true);
    static Operand label(std::uint32_t id);
    static Operand floating(lowir_model::Operand value, Type type, const lowir_model::Program* program = nullptr);
    long double floating_value() const;
};
enum class Op {
    Mov, Load, Store, Lea, Add, Sub, Mul, And, Or, Xor, Neg, Not, Bswap,
    Compare, Test, Set, ExtendSigned, ExtendUnsigned, SignDividend, Div, Udiv,
    Shl, Shr, Sar, Jump, Jcc, Call, Return, Exit, Trap,
    CopyBytes, ZeroBytes, Fence, Xadd, Exchange, Cmpxchg,
    Adc, Sbb, MulWide, Shld, Shrd, CmpxchgWide,
    Fmov, Fadd, Fsub, Fmul, Fdiv, Fneg, Fcompare, Fset,
    Sitofp, Uitofp, Fptosi, Fptoui, Fpext, Fptrunc, Freturn, Fpop,
    EhPush, EhPop, Throw, Resume, StackAlloc
};
// Image-owned runtime entities have identities after the external symbol range.
// They are not semantic declarations, nor are their spellings lookup keys.
enum class RuntimeEntity { ExceptionTop, ExceptionValue, Count };
inline Operand runtime_operand(const lowir_model::Program& p, RuntimeEntity entity) {
    return Operand::symbol(SymbolId(p.symbols.size()+1+unsigned(entity)),false);
}
const char* runtime_name(unsigned entity);
struct Instruction {
    Op op;
    Type type;
    Type source_type;
    std::array<Operand,3> args;
    unsigned count = 0;
    X86Condition condition = XC_E;
    unsigned arg_registers = 0;
    std::uint64_t bytes = 0;
    unsigned alignment = 1;
    ir_model::FunctionBoundaryMetadata boundary;
    DebugLocation debug;
    explicit Instruction(Op op = Op::Mov, Type type = Type::I64) : op(op), type(type) {}
};
struct FrameBinding { Name name; Type type; std::int64_t offset; bool temporary; bool parameter; };
struct Parameter { Name name; Type type; Operand location; Operand second; };
struct Block { std::uint32_t id; Name name; lowir_model::Range instructions; };
struct Function {
    SymbolId symbol;
    Type result;
    DebugLocation debug;
    std::vector<Parameter> params;
    std::vector<FrameBinding> frame;
    std::vector<Block> blocks;
    std::vector<Instruction> instructions;
    unsigned preserved = 0;
    unsigned frame_alignment = 16;
    int frame_base = XR_RBP;
    Operand exception_base, stack_floor;
    std::uint64_t frame_bytes = 0, stack_size = 0, scratch_bytes = 0;
    bool frame_pointer = true, shared_epilogue = true;
};
struct Statistics {
    std::uint64_t functions = 0, instructions = 0, frame_bytes = 0, text_bytes = 0;
    std::uint64_t value_visits = 0, scratch_carried_reloads = 0;
    std::uint64_t parameter_flow_visits = 0, carry_window_visits = 0, xmm_reuses = 0;
    double selection_ms = 0, encoding_ms = 0;
};
const char* register_name(int reg);
std::string type_name(Type type);
bool unsigned_type(Type type);
std::uint64_t normalize(std::uint64_t value, Type type);
bool scalar_integer(Type type);
// Registers 16..29 are ordinary XMM values; 30/31 are reserved encoder scratch.
inline int xmm(unsigned n) { return 16+n; }
void dump_header(const lowir_model::Program& p, const std::vector<Instruction>& startup, std::ostream& out);
void dump_function(const lowir_model::Program& p, const Function& f, std::ostream& out);
} // namespace native
