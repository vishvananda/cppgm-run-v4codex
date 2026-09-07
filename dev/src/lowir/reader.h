#pragma once
#include "lowir/model.h"
namespace lowir_model {
// One-token cursor borrowing an immutable input buffer. Named tokens enter the
// unit interner directly; temporary strings are only numeric/keyword adapters.
class Reader {
    Program& p_;
    const std::string& source_;
    std::string path_;
    std::size_t pos_ = 0, line_ = 1;
    cppgm::TextView token_;
    void advance();
    bool at(const char* s) const;
    bool accept(const char* s);
    void expect(const char* s);
    std::string word();
    Name name(char prefix = 0);
    std::uint64_t natural();
    Type type();
    Operand literal();
    Operand operand(FunctionBuilder& b);
    void span(std::uint64_t& bytes, std::uint32_t& alignment);
    DebugLocation debug();
    SymbolMetadata metadata(bool function, FunctionBoundaryMetadata* boundary = 0);
    void boundary_field(FunctionBoundaryMetadata& m, const std::string& key, const std::string& value);
    void parameter_metadata(Parameter& p);
    Signature signature(FunctionBuilder* b);
    void global(bool declaration);
    void function(bool declaration);
    DataItem data_item(Type t, bool structured);
    void instruction(FunctionBuilder& b);
    void instruction_body(Instruction& i, FunctionBuilder& b);
    void call(Instruction& i, FunctionBuilder& b);
    void add_operand(FunctionBuilder& b);
    void comma_operand(FunctionBuilder& b);
public:
    Reader(Program& p, const std::string& text, const std::string& path);
    void read();
};
} // namespace lowir_model
