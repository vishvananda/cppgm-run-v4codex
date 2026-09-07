#include "lowir/model.h"
#include <cassert>
#include <iostream>
#include <sstream>
using namespace lowir_model;
int main()
{
    assert(sizeof(Type) == 8 && sizeof(Operand) == 32);
    Type a = Type::object(24, 8), b = Type::object(24, 8);
    assert(a == b && a != Type::object(24, 16) && a.bytes() == 24 && a.alignment() == 8);
    Program p = construct_exercise("sum");
    assert(p.functions.size() == 1 && p.instructions.size() == 4);
    assert(p.instructions[0].opcode == Opcode::Binary && p.instructions[0].operation == Operation::Add);
    auto old = p.functions[0].symbol;
    auto name = p.symbols[old.index-1].name;
    for (int i = 0; i < 5000; ++i) p.intern("unused_" + std::to_string(i));
    assert(p.symbol(name) == old && p.name(name) == "@sum_to");
    // Prove the writer consumes editable numeric model facts, with no source
    // buffer or original instruction spelling available.
    p.operands[p.instructions[0].operands.begin+1] = Operand::integer(3);
    auto text = serialize_lowir_program(p);
    assert(text.find("%n, 3") != std::string::npos);
    Program roundtrip = parse_lowir_program_text(text);
    assert(serialize_lowir_program(roundtrip) == text);
    for (const char* exercise : {"swap", "call"}) {
        Program model = construct_exercise(exercise);
        auto output = serialize_lowir_program(model);
        assert(serialize_lowir_program(parse_lowir_program_text(output)) == output);
    }
    // Separate units own independent identity tables and can be released in any order.
    Program q = construct_exercise("swap");
    assert(q.name(q.symbols.front().name) == "@swap_values");
    assert(p.name(name) == "@sum_to");
    Program direct;
    SymbolId symbol = direct.symbol(direct.intern("@direct"));
    Function f;
    f.symbol = symbol;
    Signature s;
    s.result = Type::I64;
    direct.signatures.push_back(s);
    f.signature = SignatureId(1);
    direct.functions.push_back(f);
    direct.symbols[0].kind = Symbol::FunctionSymbol;
    direct.symbols[0].entity = 1;
    FunctionBuilder builder(direct, FunctionId(1));
    builder.start_block(direct.intern("^entry"));
    bool rejected = false;
    try { builder.append(Instruction(Opcode::Const, Type::I64), {}, direct.intern("%bad")); }
    catch (const ParseError&) { rejected = true; }
    assert(rejected && direct.instructions.empty());
    ValueId answer = builder.append(Instruction(Opcode::Const, Type::I64), {Operand::integer(42)}, direct.intern("%answer"));
    builder.append(Instruction(Opcode::Return, Type::I64), {Operand::value(answer)});
    // The unused forward destination reserved by the rejected append remains
    // harmless; only operand uses require a definition.
    validate(direct);
    assert(direct.stats.validated_instructions == 2);
    rejected = false;
    try { builder.append(Instruction(Opcode::Return, Type::I64), {Operand::integer(1)}); }
    catch (const ParseError&) { rejected = true; }
    assert(rejected);
    direct.instructions[0].is_volatile = true;
    rejected = false;
    try { validate(direct); } catch (const ParseError&) { rejected = true; }
    assert(rejected);
    std::cout << "PASS: typed identity, independent units, direct construction, local invariants and model writer\n";
}
