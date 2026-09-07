#include "lowir/model.h"
namespace lowir_model {
// Constant-time local checks used at construction. Cross-links and whole-CFG
// checks belong to the one external-input validation boundary.
void validate_instruction_shape(const Instruction& i)
{
    require(unsigned(i.opcode) <= unsigned(Opcode::Unreachable), "invalid opcode");
    unsigned arity = 0;
    bool scalar = false, variable = false;
    switch (i.opcode) {
    case Opcode::Const: case Opcode::Copy: case Opcode::Load: case Opcode::VaArg:
    case Opcode::Unary: case Opcode::Convert: case Opcode::Throw: arity = 1; scalar = true; break;
    case Opcode::Store: case Opcode::Binary: case Opcode::Compare: arity = 2; scalar = true; break;
    case Opcode::Exception: case Opcode::ExceptionSelector: scalar = true; break;
    case Opcode::Phi:
        variable = true; scalar = true;
        require(i.operands.count && !(i.operands.count % 2), "invalid phi arity"); break;
    case Opcode::Addr: case Opcode::VaStart: case Opcode::StackAlloc: case Opcode::Jump:
    case Opcode::EhTry: case Opcode::ZeroInit: case Opcode::AtomicThreadFence:
    case Opcode::AtomicSignalFence: arity = 1; break;
    case Opcode::Index: arity = 2; require(i.type != Type(), "void index element"); break;
    case Opcode::AtomicLoad: arity = 2; scalar = true; break;
    case Opcode::AtomicStore: case Opcode::AtomicAddFetch: case Opcode::AtomicExchange: arity = 3; scalar = true; break;
    case Opcode::AtomicCompareExchange: arity = 5; scalar = true; break;
    case Opcode::CopyObject: arity = 2; break;
    case Opcode::Branch: arity = 3; break;
    case Opcode::Switch:
        variable = true;
        require(i.operands.count >= 2 && !(i.operands.count % 2), "invalid switch arity"); break;
    case Opcode::Call: variable = true; require(i.operands.count != 0, "missing callee"); break;
    case Opcode::EhCleanup: case Opcode::EhCatchAll:
        variable = true; require(i.operands.count <= 1, "invalid exception clause arity"); break;
    case Opcode::EhCatch:
        variable = true; require(i.operands.count >= 1 && i.operands.count <= 2, "invalid catch arity"); break;
    case Opcode::EhFilter: variable = true; break;
    case Opcode::Return: arity = i.type == Type() ? 0 : 1; break;
    case Opcode::EhEnd: case Opcode::Resume: case Opcode::Unreachable: break;
    }
    require(variable || i.operands.count == arity, "invalid instruction arity");
    if (scalar) require(i.type.scalar(), "invalid scalar instruction type");
    require(!i.is_volatile || i.opcode == Opcode::Load || i.opcode == Opcode::Store, "misplaced volatile flag");
    require(i.projection == IPK_NONE || i.opcode == Opcode::Index, "misplaced projection");
    require((!i.signature && !i.copy_elision) || i.opcode == Opcode::Call, "misplaced call metadata");
    if (i.opcode == Opcode::Unary) require(i.operation >= Operation::Neg && i.operation <= Operation::Bswap, "invalid unary operator");
    else if (i.opcode == Opcode::Binary) require(i.operation >= Operation::Add && i.operation <= Operation::Ushr, "invalid binary operator");
    else if (i.opcode == Opcode::Compare) require(i.operation >= Operation::Eq && i.operation <= Operation::Uge, "invalid comparison predicate");
    else if (i.opcode == Opcode::Convert) require(i.operation >= Operation::Sext && i.operation <= Operation::Fptrunc, "invalid conversion operator");
    else require(i.operation == Operation::None, "misplaced scalar operator");
    if (i.opcode == Opcode::CopyObject || i.opcode == Opcode::ZeroInit)
        require(i.bytes && i.alignment && !(i.alignment & (i.alignment-1)), "invalid object span");
}
} // namespace lowir_model
