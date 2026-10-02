#pragma once
#include "preprocess/source.h"
namespace cppgm {
// Fixed target vocabulary, shared by semantics, external LowIR validation and
// encoding. Each instruction consumes a 64-byte record: result at 0, inputs
// at 16/32/48. The record is local to one expression and never escapes.
enum class X86Type : unsigned char { Void, I32, I64, U32, U64, I16, Float4, Double2, Int2, Int4, Long2, Long1, Byte8, Byte16, Short4, Short8, Float2Ptr, ConstFloat2Ptr, FloatPtr, DoublePtr, Long2Ptr, Int2Ptr, IntPtr, LongPtr, ConstVoidPtr, ConstCharPtr, ULongPtr };
enum class X86Form : unsigned char { Binary, Unary, ToInt, FromInt, Compare, Mask, GetControl, SetControl, Fence, PairToFloat, FloatToPair, LoadHalf, StoreHalf, StreamStore, Flush, MaskStore, LoadVector, DynamicCompare, Shuffle, ShuffleOne, ByteShift, Insert, Extract };
struct X86Builtin {
    const char* name;
    X86Type result, first, second;
    X86Form form;
    unsigned short opcode;
    unsigned char prefix, predicate;
    bool reverse;
};
unsigned x86_builtin(TextView name);
const X86Builtin& x86_builtin(unsigned id);
bool valid_x86_builtin(unsigned id);
}
