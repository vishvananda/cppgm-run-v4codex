#include "lowir/model.h"
#include <limits>

namespace lowir_model {
void require(bool condition, const char* message) { if (!condition) throw ParseError(message); }
static std::size_t hash(Name name) { return std::uint32_t(name * 2654435761u); }
std::uint32_t NameIndex::find(Name name) const
{
    std::size_t i = hash(name) & (slots_.size() - 1);
    while (slots_[i].key) {
        if (slots_[i].key == name) return slots_[i].value;
        i = (i + 1) & (slots_.size() - 1);
    }
    return 0;
}
void NameIndex::grow()
{
    std::vector<Entry> old;
    old.swap(slots_);
    slots_.resize(old.size() * 2);
    size_ = 0;
    for (const Entry& e : old) if (e.key) insert(e.key, e.value);
}
bool NameIndex::insert(Name name, std::uint32_t value)
{
    require(name && value, "invalid index identity");
    if ((size_ + 1) * 2 > slots_.size()) grow();
    std::size_t i = hash(name) & (slots_.size() - 1);
    while (slots_[i].key) {
        if (slots_[i].key == name) return false;
        i = (i + 1) & (slots_.size() - 1);
    }
    slots_[i].key = name;
    slots_[i].value = value;
    ++size_;
    return true;
}
Type Type::object(std::uint32_t bytes, std::uint32_t alignment)
{
    require(bytes && alignment && !(alignment & (alignment - 1)), "invalid object layout");
    unsigned shift = 0;
    while ((std::uint32_t(1) << shift) != alignment) ++shift;
    Type t(Object);
    t.code_ |= std::uint64_t(bytes) << 8 | std::uint64_t(shift) << 40;
    return t;
}
std::uint32_t Type::bytes() const
{
    static const unsigned sizes[] = {0,1,1,1,2,2,4,4,8,4,8,16,8};
    return kind() == Object ? std::uint32_t(code_ >> 8) : sizes[kind()];
}
std::uint32_t Type::alignment() const
{
    return kind() == Object ? std::uint32_t(1) << (code_ >> 40) : bytes();
}
unsigned Type::width() const
{
    return kind() == I1 ? 1 : kind() == F80 ? 80 : bytes() * 8;
}
Operand Operand::integer(std::uint64_t n) { Operand o; o.data.integer = n; return o; }
Operand Operand::floating(long double n) { Operand o; o.kind = Floating; o.data.floating = n; return o; }
Operand Operand::null() { Operand o; o.kind = Null; return o; }
Operand Operand::value(ValueId id) { Operand o; o.kind = Temporary; o.ref = id.index; return o; }
Operand Operand::slot(SlotId id) { Operand o; o.kind = Slot; o.ref = id.index; return o; }
Operand Operand::symbol(SymbolId id) { Operand o; o.kind = Symbol; o.ref = id.index; return o; }
Operand Operand::label(BlockId id) { Operand o; o.kind = Label; o.ref = id.index; return o; }
Name Program::intern(const std::string& s) { return names.intern(cppgm::TextView(s.data(), s.size())); }
std::string Program::name(Name id) const
{
    if (!id) return "";
    cppgm::TextView s = names.spelling(id);
    return std::string(s.data, s.size);
}
SymbolId Program::symbol(Name name)
{
    if (auto id = symbol_names.find(name)) return SymbolId(id);
    Symbol symbol;
    symbol.name = name;
    symbols.push_back(symbol);
    symbol_names.insert(name, symbols.size());
    return SymbolId(symbols.size());
}
ValueId FunctionBuilder::value(Name name)
{
    if (auto id = values_.find(name)) return ValueId(id);
    Value v;
    v.name = name;
    v.owner = function_;
    p_.values.push_back(v);
    values_.insert(name, p_.values.size());
    return ValueId(p_.values.size());
}
BlockId FunctionBuilder::block(Name name)
{
    if (auto id = blocks_.find(name)) return BlockId(id);
    Block b;
    b.name = name;
    b.owner = function_;
    p_.blocks.push_back(b);
    blocks_.insert(name, p_.blocks.size());
    return BlockId(p_.blocks.size());
}
SlotId FunctionBuilder::slot(Name name) const
{
    auto id = slots_.find(name);
    require(id != 0, "undefined slot");
    return SlotId(id);
}
void FunctionBuilder::parameter(Parameter param)
{
    require(param.value.index && param.value.index <= p_.values.size(), "invalid parameter value");
    Value& v = p_.values[param.value.index - 1];
    require(!v.defined && v.owner == function_, "duplicate parameter");
    require(param.type != Type(), "void parameter");
    v.type = param.type;
    v.defined = true;
    p_.parameters.push_back(param);
}
SlotId FunctionBuilder::add_slot(Name name, Type type)
{
    require(!slots_.find(name) && type != Type(), "duplicate or void slot");
    Slot s;
    s.name = name;
    s.type = type;
    s.owner = function_;
    p_.slots.push_back(s);
    SlotId id(p_.slots.size());
    slots_.insert(name, id.index);
    Function& f = p_.functions[function_.index - 1];
    if (!f.slots.count) f.slots.begin = p_.slot_order.size();
    ++f.slots.count;
    p_.slot_order.push_back(id);
    return id;
}
void FunctionBuilder::start_block(Name name)
{
    if (current_) {
        const Block& previous = p_.blocks[current_.index - 1];
        require(previous.instructions.count && terminator(p_.instructions.back().opcode), "missing terminator");
    }
    BlockId id = block(name);
    Block& b = p_.blocks[id.index - 1];
    require(!b.defined, "duplicate block");
    b.defined = true;
    b.instructions.begin = p_.instructions.size();
    Function& f = p_.functions[function_.index - 1];
    if (!f.blocks.count) f.blocks.begin = p_.block_order.size();
    ++f.blocks.count;
    p_.block_order.push_back(id);
    current_ = id;
}
ValueId FunctionBuilder::append(Instruction inst, std::initializer_list<Operand> operands, Name dest)
{
    inst.operands.begin = p_.operands.size();
    inst.operands.count = operands.size();
    p_.operands.insert(p_.operands.end(), operands);
    if (dest) inst.destination = value(dest);
    append(inst);
    return inst.destination;
}
void FunctionBuilder::append(Instruction inst)
{
    require(bool(current_), "instruction outside a block");
    Block& b = p_.blocks[current_.index - 1];
    require(!b.instructions.count || !terminator(p_.instructions.back().opcode), "instruction after terminator");
    Type result = inst.result_type();
    require(bool(inst.destination) == (result != Type()), "incorrect instruction destination");
    if (inst.destination) {
        Value& v = p_.values.at(inst.destination.index - 1);
        require(v.owner == function_, "foreign result");
        require(!v.defined || v.type == result, "temporary changes type");
        if (!v.defined) v.definition = p_.instructions.size() + 1;
        v.defined = true;
        v.type = result;
        v.truth = inst.opcode == Opcode::Compare || inst.opcode == Opcode::AtomicCompareExchange;
    }
    p_.instructions.push_back(inst);
    ++b.instructions.count;
}
} // namespace lowir_model
