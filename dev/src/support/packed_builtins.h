#pragma once
#include "preprocess/source.h"
namespace cppgm {
enum class PackedOp : unsigned char {
    None, PackSigned, PackUnsigned, UnpackLow, UnpackHigh,
    Add, Sub, AddSigned, AddUnsigned, SubSigned, SubUnsigned,
    MultiplyLow, MultiplyHigh, MultiplyAdd, Equal, Greater,
    And, AndNot, Or, Xor, ShiftLeft, ShiftRight, ShiftArithmetic
};
struct PackedBuiltin {
    const char* name;
    PackedOp op;
    unsigned char bytes, lane, result_lane;
    bool immediate;
};
unsigned packed_builtin(TextView name);
const PackedBuiltin& packed_builtin(unsigned id);
}
