#include "lowir/validator.h"
namespace lowir_model {
void Validator::instruction(const Instruction& i) const
{
    validate_instruction_shape(i);
    require(bool(i.destination) == (i.result_type() != Type()), "invalid instruction destination");
    if (i.destination) {
        const Value& v = p_.values.at(i.destination.index-1);
        require(v.owner == function_ && v.defined && v.type == i.result_type(), "invalid result identity");
    }
    require(i.operands.end() <= p_.operands.size(), "invalid operand range");
    auto count = [&](unsigned n) { require(i.operands.count == n, "invalid instruction arity"); };
    auto arg = [&](unsigned j) -> const Operand& { require(j < i.operands.count, "missing operand"); return p_.operands[i.operands.begin+j]; };
    auto scalar = [&]() { require(i.type.scalar(), "expected scalar instruction type"); };
    auto label = [&](unsigned j) {
        const Operand& o = arg(j);
        require(o.kind == Operand::Label, "expected block target");
        const Block& b = p_.blocks.at(o.ref-1);
        require(b.defined && b.owner == function_, "undefined or foreign block");
    };
    switch (i.opcode) {
    case Opcode::Const: count(1); validate_literal(arg(0), i.type); break;
    case Opcode::Copy:
        count(1); scalar();
        if (arg(0).literal()) value(arg(0), i.type);
        else {
            Type t = value_type(arg(0));
            require(t == i.type || (t.integer() && i.type.integer() && t.width() == i.type.width()) ||
                (t.width() == 64 && i.type.width() == 64 &&
                (t == Type::Ptr || i.type == Type::Ptr) && !t.floating() && !i.type.floating()), "invalid copy retype");
        }
        break;
    case Opcode::Phi:
        scalar(); require(i.operands.count && !(i.operands.count % 2), "invalid phi operands");
        for (unsigned j = 0; j < i.operands.count; j += 2) { label(j); value(arg(j+1), i.type); }
        break;
    case Opcode::Addr:
        count(1);
        require(arg(0).kind == Operand::Slot || arg(0).kind == Operand::Symbol, "invalid addressable");
        value_type(arg(0)); break;
    case Opcode::Load: count(1); scalar(); storage(arg(0), i.type); break;
    case Opcode::Store: count(2); scalar(); value(arg(0), i.type); storage(arg(1), i.type); break;
    case Opcode::Index:
        count(2); require(i.type != Type(), "void index element"); pointer(arg(0), true); integer(arg(1)); break;
    case Opcode::Unary:
        count(1); scalar(); value(arg(0), i.type);
        require(i.operation >= Operation::Neg && i.operation <= Operation::Bswap, "invalid unary operator");
        if (i.operation == Operation::Bitnot || i.operation == Operation::Bswap) require(i.type.integer(), "integer unary operation on noninteger");
        if (i.operation == Operation::Bswap) require(i.type == Type::I16 || i.type == Type::I32 || i.type == Type::I64, "invalid bswap width");
        break;
    case Opcode::Binary:
        count(2); require(i.type.integer() || i.type.floating(), "invalid binary type");
        require(i.operation >= Operation::Add && i.operation <= Operation::Ushr, "invalid binary operator");
        if (i.type.floating()) require(i.operation <= Operation::Div, "invalid floating binary operator");
        value(arg(0), i.type); value(arg(1), i.type); break;
    case Opcode::Compare:
        count(2); scalar(); require(i.operation >= Operation::Eq && i.operation <= Operation::Uge, "invalid comparison predicate");
        value(arg(0), i.type); value(arg(1), i.type); break;
    case Opcode::Convert: count(1); conversion(i, arg(0)); break;
    case Opcode::AtomicLoad: count(2); atomic(i); break;
    case Opcode::AtomicStore: case Opcode::AtomicAddFetch: case Opcode::AtomicExchange: count(3); atomic(i); break;
    case Opcode::AtomicCompareExchange: count(5); atomic(i); break;
    case Opcode::AtomicThreadFence: case Opcode::AtomicSignalFence: count(1); atomic(i); break;
    case Opcode::VaStart: count(1); pointer(arg(0)); break;
    case Opcode::VaArg: count(1); scalar(); pointer(arg(0)); break;
    case Opcode::StackAlloc: count(1); integer(arg(0)); break;
    case Opcode::Call: require(i.operands.count != 0, "call without callee"); call(i); break;
    case Opcode::CopyObject: case Opcode::ZeroInit:
        count(i.opcode == Opcode::CopyObject ? 2 : 1);
        require(i.bytes && i.alignment && !(i.alignment & (i.alignment-1)), "invalid bulk memory span");
        if (i.opcode == Opcode::CopyObject) {
            Type t = value_type(arg(0));
            require(t == Type::Ptr || (t.kind() == Type::Object && t.bytes() == i.bytes && t.alignment() == i.alignment), "invalid object copy source");
            pointer(arg(1));
        } else pointer(arg(0));
        break;
    case Opcode::EhTry: count(1); label(0); break;
    case Opcode::EhCleanup: require(i.operands.count <= 1, "invalid cleanup"); if (i.operands.count) label(0); break;
    case Opcode::EhCatch: case Opcode::EhFilter: case Opcode::EhCatchAll:
        if (i.opcode == Opcode::EhCatch) require(i.operands.count >= 1 && i.operands.count <= 2, "invalid catch clause");
        if (i.opcode == Opcode::EhCatchAll) require(i.operands.count <= 1, "invalid catch-all clause");
        for (unsigned j = 0; j < i.operands.count; ++j) {
            const Operand& a = arg(j);
            if (a.kind == Operand::Symbol) require(p_.symbols.at(a.ref-1).kind == Symbol::GlobalSymbol, "invalid catch type info");
            else require(j+1 == i.operands.count && a.kind == Operand::Integer, "invalid exception selector");
        }
        break;
    case Opcode::Exception: case Opcode::ExceptionSelector: count(0); scalar(); break;
    case Opcode::Throw: count(1); scalar(); value(arg(0), i.type); break;
    case Opcode::Jump: count(1); label(0); break;
    case Opcode::Branch:
        count(3); require(value_type(arg(0)).scalar(), "invalid branch condition"); label(1); label(2); break;
    case Opcode::Switch:
        require(i.operands.count >= 2 && !(i.operands.count % 2), "invalid switch operands");
        integer(arg(0)); label(1);
        for (unsigned j = 2; j < i.operands.count; j += 2) { value(arg(j), value_type(arg(0))); label(j+1); }
        break;
    case Opcode::Return:
        require(i.type == p_.signatures[p_.functions[function_.index-1].signature.index-1].result, "return boundary mismatch");
        count(i.type == Type() ? 0 : 1);
        if (i.type != Type()) value(arg(0), i.type);
        break;
    case Opcode::EhEnd: case Opcode::Resume: case Opcode::Unreachable: count(0); break;
    }
    if (i.debug.file) require(i.debug.line && i.debug.column, "invalid instruction debug location");
}
} // namespace lowir_model
