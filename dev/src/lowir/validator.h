#pragma once
#include "lowir/model.h"
namespace lowir_model {
class Validator {
    Program& p_;
    FunctionId function_;
    std::uint32_t ordinal_ = 0;
    bool phi_ = false;
    Type value_type(const Operand& o) const;
    void value(const Operand& o, Type type) const;
    void integer(const Operand& o) const;
    void pointer(const Operand& o, bool object = false) const;
    void storage(const Operand& o, Type type) const;
    void call(const Instruction& i) const;
    void conversion(const Instruction& i, const Operand& o) const;
    void atomic(const Instruction& i) const;
    void instruction(const Instruction& i) const;
    void symbols() const;
public:
    explicit Validator(Program& p) : p_(p) {}
    void run();
};
void validate_literal(const Operand& o, Type type);
}
