#include "lowir/model.h"
namespace lowir_model {
namespace {
FunctionId helper(Program& p, const char* name, Type result)
{
    SymbolId sid = p.symbol(p.intern(name));
    Function f;
    f.symbol = sid;
    Signature signature;
    signature.result = result;
    signature.parameters.begin = p.parameters.size();
    p.signatures.push_back(signature);
    f.signature = SignatureId(p.signatures.size());
    p.functions.push_back(f);
    Symbol& s = p.symbols[sid.index-1];
    s.kind = Symbol::FunctionSymbol;
    s.entity = p.functions.size();
    return FunctionId(p.functions.size());
}
ValueId parameter(Program& p, FunctionBuilder& b, FunctionId f, const char* name, Type type)
{
    Parameter a;
    a.type = type;
    a.value = b.value(p.intern(name));
    b.parameter(a);
    ++p.signatures[p.functions[f.index-1].signature.index-1].parameters.count;
    return a.value;
}
ValueId binary(Program& p, FunctionBuilder& b, const char* name, Operation op, Operand a, Operand c)
{
    Instruction i(Opcode::Binary, Type::I64);
    i.operation = op;
    return b.append(i, {a, c}, p.intern(name));
}
void sum(Program& p)
{
    FunctionId f = helper(p, "@sum_to", Type::I64);
    FunctionBuilder b(p, f);
    ValueId n = parameter(p, b, f, "%n", Type::I64);
    b.start_block(p.intern("^entry"));
    // The exercise's 0..1,000,000 domain keeps both n*(n+1) and its
    // mathematical sum in i64. This constructs the chosen algorithm directly;
    // it is not an optimizer rewriting arbitrary possibly-overflowing IR.
    ValueId next = binary(p, b, "%next", Operation::Add, Operand::value(n), Operand::integer(1));
    ValueId product = binary(p, b, "%product", Operation::Mul, Operand::value(n), Operand::value(next));
    ValueId result = binary(p, b, "%result", Operation::Div, Operand::value(product), Operand::integer(2));
    b.append(Instruction(Opcode::Return, Type::I64), {Operand::value(result)});
}
void swap(Program& p)
{
    FunctionId f = helper(p, "@swap_values", Type::Void);
    FunctionBuilder b(p, f);
    ValueId left = parameter(p, b, f, "%left", Type::Ptr);
    ValueId right = parameter(p, b, f, "%right", Type::Ptr);
    b.start_block(p.intern("^entry"));
    ValueId a = b.append(Instruction(Opcode::Load, Type::I64), {Operand::value(left)}, p.intern("%a"));
    ValueId c = b.append(Instruction(Opcode::Load, Type::I64), {Operand::value(right)}, p.intern("%b"));
    b.append(Instruction(Opcode::Store, Type::I64), {Operand::value(c), Operand::value(left)});
    b.append(Instruction(Opcode::Store, Type::I64), {Operand::value(a), Operand::value(right)});
    b.append(Instruction(Opcode::Return), {});
}
void call(Program& p)
{
    FunctionId f = helper(p, "@call_twice", Type::I64);
    FunctionBuilder b(p, f);
    ValueId fn = parameter(p, b, f, "%fn", Type::Ptr);
    ValueId x = parameter(p, b, f, "%x", Type::I64);
    Signature s;
    s.result = Type::I64;
    s.parameters.begin = p.parameters.size();
    s.parameters.count = 1;
    Value v;
    v.name = p.intern("%argument");
    v.type = Type::I64;
    v.defined = true;
    p.values.push_back(v);
    Parameter a;
    a.type = Type::I64;
    a.value = ValueId(p.values.size());
    p.parameters.push_back(a);
    p.signatures.push_back(s);
    Instruction invoke(Opcode::Call, Type::I64);
    invoke.signature = SignatureId(p.signatures.size());
    b.start_block(p.intern("^entry"));
    ValueId first = b.append(invoke, {Operand::value(fn), Operand::value(x)}, p.intern("%first"));
    ValueId second = b.append(invoke, {Operand::value(fn), Operand::value(first)}, p.intern("%second"));
    b.append(Instruction(Opcode::Return, Type::I64), {Operand::value(second)});
}
}
Program construct_exercise(const std::string& name)
{
    Program p;
    if (name == "sum") sum(p);
    else if (name == "swap") swap(p);
    else if (name == "call") call(p);
    else throw ParseError("unknown exercise");
    validate(p);
    return p;
}
} // namespace lowir_model
