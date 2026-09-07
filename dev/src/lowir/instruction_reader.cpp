#include "lowir/reader.h"
namespace lowir_model {
void Reader::add_operand(FunctionBuilder& b) { p_.operands.push_back(operand(b)); }
void Reader::comma_operand(FunctionBuilder& b) { expect(","); add_operand(b); }
void Reader::call(Instruction& i, FunctionBuilder& b)
{
    i.type = type();
    add_operand(b);
    expect("(");
    if (!at(")")) { add_operand(b); while (accept(",")) add_operand(b); }
    expect(")");
    if (accept("[")) { expect("elision"); expect("="); expect("copy"); expect("]"); i.copy_elision = true; }
    if (accept("as")) {
        Signature s = signature(0);
        SymbolMetadata m = metadata(true, &s.boundary);
        require(m.role == SR_NONE && m.linkage == LLM_DEFAULT && m.binding == SBM_DEFAULT &&
            !m.object && !m.section && !m.tls_for && !m.keep_alias && !m.prefer_local &&
            !m.object_root && !m.force_inline && !m.inline_hint && !m.no_inline, "symbol metadata on call signature");
        p_.signatures.push_back(s);
        i.signature = SignatureId(p_.signatures.size());
    }
}
void Reader::instruction(FunctionBuilder& b)
{
    ValueId dest;
    if (token_.size && token_.data[0] == '%') { dest = b.value(name('%')); expect("="); }
    Instruction i(parse_opcode(word()));
    i.destination = dest;
    i.operands.begin = p_.operands.size();
    instruction_body(i, b);
    i.operands.count = p_.operands.size() - i.operands.begin;
    i.debug = debug();
    b.append(i);
}
void Reader::instruction_body(Instruction& i, FunctionBuilder& b)
{
    switch (i.opcode) {
    case Opcode::Const:
        i.type = type(); p_.operands.push_back(literal()); break;
    case Opcode::Copy: case Opcode::VaArg: case Opcode::Throw:
        i.type = type(); add_operand(b); break;
    case Opcode::Phi:
        i.type = type(); expect("[");
        do { p_.operands.push_back(Operand::label(b.block(name('^')))); expect(":"); add_operand(b); } while (accept(","));
        expect("]"); break;
    case Opcode::Addr: case Opcode::VaStart: case Opcode::StackAlloc: case Opcode::Jump:
        add_operand(b); break;
    case Opcode::Load: case Opcode::Store:
        i.is_volatile = accept("volatile"); i.type = type(); add_operand(b);
        if (i.opcode == Opcode::Store) comma_operand(b);
        break;
    case Opcode::Index:
        i.type = type();
        if (accept("[")) {
            expect("projection"); expect("=");
            std::string v = word();
            require(v == "array_element" || v == "field", "invalid index projection");
            i.projection = v == "array_element" ? IPK_ARRAY_ELEMENT : IPK_FIELD;
            expect("]");
        }
        add_operand(b); comma_operand(b); break;
    case Opcode::Unary: case Opcode::Binary: case Opcode::Compare: case Opcode::Convert:
        i.operation = parse_operation(word()); i.type = type();
        if (i.opcode == Opcode::Convert) i.source_type = type();
        add_operand(b);
        if (i.opcode == Opcode::Binary || i.opcode == Opcode::Compare) comma_operand(b);
        break;
    case Opcode::AtomicLoad: case Opcode::AtomicStore: case Opcode::AtomicAddFetch:
    case Opcode::AtomicExchange: case Opcode::AtomicCompareExchange: {
        i.type = type(); add_operand(b);
        if (i.opcode != Opcode::AtomicLoad) comma_operand(b);
        if (i.opcode == Opcode::AtomicCompareExchange) comma_operand(b);
        expect(","); p_.operands.push_back(Operand::integer(natural()));
        if (i.opcode == Opcode::AtomicCompareExchange) { expect(","); p_.operands.push_back(Operand::integer(natural())); }
        break;
    }
    case Opcode::AtomicThreadFence: case Opcode::AtomicSignalFence:
        p_.operands.push_back(Operand::integer(natural())); break;
    case Opcode::Call: call(i, b); break;
    case Opcode::CopyObject: case Opcode::ZeroInit:
        span(i.bytes, i.alignment); add_operand(b);
        if (i.opcode == Opcode::CopyObject) comma_operand(b);
        break;
    case Opcode::EhTry:
        p_.operands.push_back(Operand::label(b.block(name('^')))); break;
    case Opcode::EhCleanup:
        if (token_.size && token_.data[0] == '^') p_.operands.push_back(Operand::label(b.block(name('^'))));
        break;
    case Opcode::EhCatch:
        p_.operands.push_back(Operand::symbol(p_.symbol(name('@'))));
        if (accept(",")) p_.operands.push_back(literal());
        break;
    case Opcode::EhFilter:
        if (token_.size && token_.data[0] == '@') {
            p_.operands.push_back(Operand::symbol(p_.symbol(name('@'))));
            while (accept(",")) {
                if (token_.size && token_.data[0] == '@') p_.operands.push_back(Operand::symbol(p_.symbol(name('@'))));
                else { p_.operands.push_back(literal()); break; }
            }
        } else if (accept(",")) p_.operands.push_back(literal());
        break;
    case Opcode::EhCatchAll:
        if (accept(",")) p_.operands.push_back(literal());
        break;
    case Opcode::Exception: case Opcode::ExceptionSelector: i.type = type(); break;
    case Opcode::Branch:
        add_operand(b); comma_operand(b); comma_operand(b); break;
    case Opcode::Switch:
        add_operand(b); comma_operand(b);
        while (accept(",")) { add_operand(b); expect(":"); add_operand(b); }
        break;
    case Opcode::Return:
        i.type = type(); if (i.type != Type()) add_operand(b); break;
    case Opcode::EhEnd: case Opcode::Resume: case Opcode::Unreachable: break;
    }
}
} // namespace lowir_model
