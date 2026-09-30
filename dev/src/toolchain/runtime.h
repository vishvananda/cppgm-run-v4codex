#pragma once
#include "toolchain/object.h"
namespace cppgm { namespace toolchain {
struct RuntimeRequest { std::string name; ir_model::SymbolRole role; };
// This owner contains only runtime IR. It never owns source or semantic nodes.
struct RuntimeProgram {
    lowir_model::Program p;
    lowir_model::SymbolId tables[3];
    lowir_model::SymbolId symbol(const std::string& name, ir_model::SymbolRole role);
    lowir_model::FunctionId function(lowir_model::SymbolId symbol, lowir_model::Type result);
    void table(lowir_model::SymbolId symbol);
};
struct RuntimeBody {
    using Type = lowir_model::Type;
    using Operand = lowir_model::Operand;
    using Opcode = lowir_model::Opcode;
    using Operation = lowir_model::Operation;
    lowir_model::Program& p;
    lowir_model::FunctionId function;
    lowir_model::FunctionBuilder builder;
    unsigned ordinal = 0;
    RuntimeBody(RuntimeProgram& program, lowir_model::FunctionId f);
    Operand parameter(Type type);
    Operand emit(Opcode op, Type type, std::initializer_list<Operand> args, Operation operation = Operation::None);
    Operand call(lowir_model::FunctionId f, std::initializer_list<Operand> args);
    Operand offset(Operand base, std::int64_t bytes);
    Operand load(Operand base, std::int64_t bytes = 0, Type type = Type::Ptr);
    void store(Operand base, std::int64_t bytes, Operand value, Type type = Type::Ptr);
    Operand compare(Operand a, Operand b, Operation op = Operation::Eq, Type type = Type::Ptr);
    lowir_model::BlockId block();
    void start(lowir_model::BlockId b);
    void jump(lowir_model::BlockId b);
    void branch(Operand condition, lowir_model::BlockId yes, lowir_model::BlockId no);
};
void build_dynamic_cast(RuntimeProgram&, lowir_model::SymbolId);
bool runtime_role(ir_model::SymbolRole);
Object runtime_object(const std::vector<RuntimeRequest>&, native::Statistics&);
} }
namespace native { Function process_runtime(SymbolId,ir_model::SymbolRole); }
