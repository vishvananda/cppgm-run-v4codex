#pragma once
#include <cstdint>
#include <string>
#include <vector>
namespace cppgm { namespace syntax {
// Source-owned instruction recipes. Operand ordinals refer to the assembly
// statement's children, whose expression edges project during substitution.
enum class AsmOp : unsigned char { Nop, Pause, Fence, Move, Add, Sub, And, Or, Xor, Inc, Dec, Not, Neg, Bswap, Exchange, Xadd };
enum AsmConstraint { AsmOutput=1, AsmRead=2, AsmMemory=4, AsmRegister=8, AsmImmediate=16, AsmEarly=32 };
struct AsmInstruction {
    AsmOp op = AsmOp::Nop;
    unsigned char width = 0, destination = 0, source = 0;
    bool locked = false, immediate = false;
    std::uint64_t value = 0;
};
struct AsmPlan { std::uint32_t begin = 0, count = 0; bool memory = false, extended = false; };
struct AsmOperandName { std::string name; unsigned flags = 0, match = 0; };
void parse_assembly_template(const std::string&, const std::vector<AsmOperandName>&,
    std::vector<AsmInstruction>&);
} }
