#include "lowir/validator.h"
#include "lowir/metadata.h"
namespace lowir_model {
void validate_literal(const Operand& o, Type type)
{
    require(o.literal() && type.scalar(), "invalid literal type");
    if (o.kind == Operand::Floating) require(type.floating(), "floating literal in non-floating operand");
    if (o.kind == Operand::Null) require(type == Type::Ptr, "null literal in non-pointer operand");
}
Type Validator::value_type(const Operand& o) const
{
    switch (o.kind) {
    case Operand::Temporary: {
        const Value& v = p_.values.at(o.ref-1);
        require(v.defined && v.owner == function_, "undefined or foreign temporary");
        require(phi_ || v.definition < ordinal_, "temporary used before definition");
        return v.type;
    }
    case Operand::Slot: {
        const Slot& s = p_.slots.at(o.ref-1);
        require(s.owner == function_, "foreign slot");
        return s.type;
    }
    case Operand::Symbol: {
        const Symbol& s = p_.symbols.at(o.ref-1);
        if (s.kind == Symbol::FunctionSymbol) return Type::Ptr;
        require(s.kind == Symbol::GlobalSymbol, "undefined symbol");
        return p_.globals.at(s.entity-1).type;
    }
    case Operand::Integer: return Type::I64;
    case Operand::Floating: return Type::F80;
    case Operand::Null: return Type::Ptr;
    default: throw ParseError("block label used as value");
    }
}
void Validator::value(const Operand& o, Type type) const
{
    if (o.literal()) { validate_literal(o, type); return; }
    Type actual = value_type(o);
    bool truth = o.kind == Operand::Temporary && p_.values[o.ref-1].truth && type == Type::I1;
    require(actual == type || truth, "incompatible operand type");
}
void Validator::integer(const Operand& o) const { require(value_type(o).integer(), "expected integer value"); }
void Validator::pointer(const Operand& o, bool object) const
{
    // Literal operands acquire their type from the operation. PA13 virtual
    // delete retains a load from literal zero in its unreachable nonnull arm.
    if (o.literal()) { validate_literal(o, Type::Ptr); return; }
    Type t = value_type(o);
    require(t == Type::Ptr || (object && t.kind() == Type::Object), "expected pointer value");
}
void Validator::storage(const Operand& o, Type t) const
{
    if (o.kind == Operand::Slot) {
        Type actual = value_type(o);
        require(actual == t || actual.kind() == Type::Object, "incompatible slot access");
    } else if (o.kind == Operand::Symbol) {
        const Symbol& s = p_.symbols.at(o.ref-1);
        require(s.kind == Symbol::GlobalSymbol, "function used as storage");
        const Global& g = p_.globals.at(s.entity-1);
        require(g.structured || g.type == Type() || g.type == t, "incompatible global access");
    } else {
        require(o.kind == Operand::Temporary || o.literal(), "invalid storage operand");
        pointer(o, true);
    }
}
void Validator::conversion(const Instruction& i, const Operand& o) const
{
    Type a = i.source_type, b = i.type;
    value(o, a);
    bool legal = false;
    switch (i.operation) {
    case Operation::Sext: case Operation::Zext: legal = a.integer() && b.integer() && a.width() < b.width(); break;
    case Operation::Trunc: legal = a.integer() && b.integer() && a.width() > b.width(); break;
    case Operation::Sitofp: case Operation::Uitofp: legal = a.integer() && b.floating(); break;
    case Operation::Fptosi: case Operation::Fptoui: legal = a.floating() && b.integer(); break;
    case Operation::Fpext: legal = a.floating() && b.floating() && a.width() < b.width(); break;
    case Operation::Fptrunc: legal = a.floating() && b.floating() && a.width() > b.width(); break;
    default: break;
    }
    require(legal, "invalid scalar conversion");
}
void Validator::call(const Instruction& i) const
{
    const Operand& callee = p_.operands[i.operands.begin];
    const Signature* sig = 0;
    bool direct = false;
    if (callee.kind == Operand::Symbol) {
        const Symbol& s = p_.symbols.at(callee.ref-1);
        if (s.kind == Symbol::FunctionSymbol) {
            direct = true;
            sig = &p_.signatures[p_.functions[s.entity-1].signature.index-1];
        }
    }
    if (!direct) {
        pointer(callee);
        require(bool(i.signature), "indirect call requires signature");
        sig = &p_.signatures.at(i.signature.index-1);
        validate_signature(p_, *sig, true);
    }
    require(sig && sig->result == i.type, "call result type mismatch");
    unsigned count = i.operands.count-1;
    require(count >= sig->parameters.count && (sig->boundary.arity == CAM_VARIADIC || count == sig->parameters.count), "call arity mismatch");
    for (unsigned j = 0; j < count; ++j) {
        const Operand& a = p_.operands[i.operands.begin+j+1];
        if (j < sig->parameters.count) value(a, p_.parameters[sig->parameters.begin+j].type);
        else require(value_type(a).scalar(), "invalid variadic value");
    }
    if (i.copy_elision) require(direct && i.type == Type() && count >= 2, "invalid copy elision permission");
    if (direct && i.signature) {
        const Signature& explicit_sig = p_.signatures.at(i.signature.index-1);
        validate_signature(p_, explicit_sig, true);
        require(explicit_sig.result == sig->result && explicit_sig.parameters.count == sig->parameters.count &&
            explicit_sig.boundary.arity == sig->boundary.arity, "incompatible explicit direct call signature");
        for (unsigned j = 0; j < sig->parameters.count; ++j)
            require(p_.parameters[explicit_sig.parameters.begin+j].type == p_.parameters[sig->parameters.begin+j].type, "incompatible signature parameter");
    }
}
void Validator::atomic(const Instruction& i) const
{
    const Operand* a = p_.operands.data() + i.operands.begin;
    bool fence = i.opcode == Opcode::AtomicThreadFence || i.opcode == Opcode::AtomicSignalFence;
    bool compare = i.opcode == Opcode::AtomicCompareExchange;
    if (!fence) {
        require(i.type.integer() || i.type == Type::Ptr, "invalid atomic value type");
        if (i.opcode == Opcode::AtomicStore) { value(a[0], i.type); pointer(a[1]); }
        else {
            pointer(a[0]);
            if (compare) { pointer(a[1]); value(a[2], i.type); }
            else if (i.opcode != Opcode::AtomicLoad) value(a[1], i.type);
        }
    }
    unsigned first_order = i.operands.count - (compare ? 2 : 1);
    for (unsigned j = first_order; j < i.operands.count; ++j)
        require(a[j].kind == Operand::Integer && a[j].data.integer <= 5, "invalid atomic ordering");
}
} // namespace lowir_model
