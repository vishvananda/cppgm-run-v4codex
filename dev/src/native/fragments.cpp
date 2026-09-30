#include "native/selection.h"
#include "native/abi.h"
namespace native {
using namespace lowir_model;
Operand Selector::fragment(Operand input, unsigned offset)
{
    if (input.kind == Operand::WideImmediate) return Operand::imm(offset ? std::uint64_t(input.displacement) : input.bits);
    require(input.kind == Operand::Memory || input.kind == Operand::Symbol,"multiword value has no storage");
    input.displacement += offset; input.address = false; return input;
}
void Selector::object_move(Operand to, Operand from, Type t)
{
    if (to.kind == from.kind && to.reg == from.reg && to.id == from.id && to.displacement == from.displacement) return;
    auto& copy = emit(Op::CopyBytes,Type(),{to,from});
    copy.bytes = t.bytes(); copy.alignment = t.alignment();
}
}
