#include "lowir/writer.h"
namespace lowir_model {
void Writer::instruction(const Instruction& i)
{
    if (i.destination) { operand(Operand::value(i.destination)); out_ << " = "; }
    out_ << spelling(i.opcode);
    auto arg = [&](unsigned j, Type t = Type()) { operand(p_.operands.at(i.operands.begin+j), t); };
    auto all = [&](unsigned first, Type t) {
        for (unsigned j = first; j < i.operands.count; ++j) { if (j != first) out_ << ", "; arg(j, t); }
    };
    switch (i.opcode) {
    case Opcode::Call:
        out_ << ' '; type(i.type); out_ << ' '; arg(0); out_ << '(';
        all(1, Type()); out_ << ')';
        if (i.copy_elision) out_ << " [elision=copy]";
        if (i.signature) {
            out_ << " as "; const Signature& s = p_.signatures[i.signature.index-1];
            signature(s); metadata(0, &s.boundary);
        }
        break;
    case Opcode::Phi:
        out_ << ' '; type(i.type); out_ << " [";
        for (unsigned j = 0; j < i.operands.count; j += 2) {
            if (j) out_ << ", ";
            arg(j); out_ << ": "; arg(j+1, i.type);
        }
        out_ << ']'; break;
    case Opcode::Switch:
        out_ << ' '; arg(0); out_ << ", "; arg(1);
        for (unsigned j = 2; j < i.operands.count; j += 2) {
            out_ << ", "; arg(j); out_ << ':'; arg(j+1);
        }
        break;
    case Opcode::CopyObject: case Opcode::ZeroInit:
        out_ << ' ' << i.bytes << 'x' << i.alignment << ' '; all(0, Type()); break;
    case Opcode::Unary: case Opcode::Binary: case Opcode::Compare: case Opcode::Convert:
        out_ << ' ' << spelling(i.operation) << ' '; type(i.type); out_ << ' ';
        if (i.opcode == Opcode::Convert) { type(i.source_type); out_ << ' '; }
        all(0, i.opcode == Opcode::Convert ? i.source_type : i.type); break;
    case Opcode::Const: case Opcode::Copy: case Opcode::Load: case Opcode::Store:
    case Opcode::AtomicLoad: case Opcode::AtomicStore: case Opcode::AtomicAddFetch:
    case Opcode::AtomicExchange: case Opcode::AtomicCompareExchange: case Opcode::VaArg:
    case Opcode::Index: case Opcode::Throw: case Opcode::Exception: case Opcode::ExceptionSelector:
    case Opcode::Return:
        if (i.is_volatile) out_ << " volatile";
        out_ << ' '; type(i.type);
        if (i.projection != IPK_NONE) out_ << " [projection=" << (i.projection == IPK_ARRAY_ELEMENT ? "array_element" : "field") << ']';
        if (i.operands.count) { out_ << ' '; all(0, i.type); }
        break;
    case Opcode::EhCatchAll:
        if (i.operands.count) { out_ << ", "; arg(0); }
        break;
    case Opcode::EhFilter:
        if (i.operands.count) {
            out_ << (p_.operands[i.operands.begin].kind == Operand::Integer ? ", " : " ");
            all(0, Type());
        }
        break;
    default:
        if (i.operands.count) { out_ << ' '; all(0, Type()); }
        break;
    }
    debug(i.debug);
}
} // namespace lowir_model
