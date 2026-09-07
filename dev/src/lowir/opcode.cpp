#include "lowir/model.h"
namespace lowir_model {
static const char* const opcodes[] = {
    "const", "copy", "phi", "addr", "load", "store", "index", "unary", "binary", "cmp", "convert",
    "atomic_load", "atomic_store", "atomic_add_fetch", "atomic_exchange", "atomic_compare_exchange",
    "atomic_thread_fence", "atomic_signal_fence", "va_start", "va_arg", "stack_alloc", "call",
    "copyobj", "zeroinit", "eh_try", "eh_cleanup", "eh_catch", "eh_filter", "eh_catch_all", "eh_end",
    "throw", "exception", "exception_selector", "resume", "jump", "branch", "switch", "return", "unreachable"
};
static const char* const operations[] = {
    "", "neg", "not", "bitnot", "bswap", "add", "sub", "mul", "div", "mod", "udiv", "umod", "and", "or",
    "xor", "shl", "shr", "ushr", "eq", "ne", "lt", "le", "gt", "ge", "ult", "ule", "ugt", "uge",
    "sext", "zext", "trunc", "sitofp", "uitofp", "fptosi", "fptoui", "fpext", "fptrunc"
};
const char* spelling(Opcode op) { return opcodes[unsigned(op)]; }
const char* spelling(Operation op) { return operations[unsigned(op)]; }
Opcode parse_opcode(const std::string& word)
{
    for (unsigned i = 0; i < sizeof(opcodes)/sizeof(*opcodes); ++i)
        if (word == opcodes[i]) return Opcode(i);
    throw ParseError("unknown instruction: " + word);
}
Operation parse_operation(const std::string& word)
{
    for (unsigned i = 1; i < sizeof(operations)/sizeof(*operations); ++i)
        if (word == operations[i]) return Operation(i);
    throw ParseError("unknown operation: " + word);
}
bool terminator(Opcode op)
{
    return op == Opcode::Throw || op == Opcode::Resume || op == Opcode::Jump ||
        op == Opcode::Branch || op == Opcode::Switch || op == Opcode::Return || op == Opcode::Unreachable;
}
Type Instruction::result_type() const
{
    switch (opcode) {
    case Opcode::Compare: case Opcode::AtomicCompareExchange: return Type::I64;
    case Opcode::Addr: case Opcode::Index: case Opcode::StackAlloc: return Type::Ptr;
    case Opcode::Const: case Opcode::Copy: case Opcode::Phi: case Opcode::Load:
    case Opcode::Unary: case Opcode::Binary: case Opcode::Convert: case Opcode::AtomicLoad:
    case Opcode::AtomicAddFetch: case Opcode::AtomicExchange: case Opcode::VaArg:
    case Opcode::Call: case Opcode::Exception: case Opcode::ExceptionSelector: return type;
    default: return Type();
    }
}
} // namespace lowir_model
