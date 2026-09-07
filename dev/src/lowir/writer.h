#pragma once
#include "lowir/model.h"
#include <ostream>
namespace lowir_model {
class Writer {
    const Program& p_;
    std::ostream& out_;
    void type(Type t);
    void operand(const Operand& v, Type context = Type());
    void debug(const DebugLocation& d);
    void symbol(SymbolId id);
    void metadata(const SymbolMetadata* m, const FunctionBoundaryMetadata* b);
    void signature(const Signature& s);
    void instruction(const Instruction& i);
    void global(const Global& g);
    void function(const Function& f);
    void data(const DataItem& d, bool structured);
public:
    Writer(const Program& p, std::ostream& out) : p_(p), out_(out) {}
    void write();
};
}
