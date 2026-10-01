#include "lowir/model.h"
#include <limits>
#include <algorithm>
#include <cstring>

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
Type Type::object(std::uint64_t bytes, std::uint64_t alignment)
{
    // Validate the producer width before packing: a 4-GiB source object must
    // not wrap to the valid zero-byte extension layout.
    require(bytes <= UINT32_MAX && alignment && alignment <= UINT32_MAX &&
        !(alignment & (alignment - 1)), "invalid object layout");
    unsigned shift = 0;
    while ((std::uint32_t(1) << shift) != alignment) ++shift;
    Type t(Object);
    t.code_ |= std::uint64_t(bytes) << 8 | std::uint64_t(shift) << 40;
    return t;
}
Type Type::vector(std::uint64_t bytes, std::uint64_t alignment, bool extended)
{
    require(bytes && !(bytes & (bytes-1)),"invalid vector extent");
    Type t = object(bytes,alignment); t.code_ |= std::uint64_t(extended ? 3 : 1)<<56; return t;
}
Type Type::complex(Type component)
{
    require(component == F32 || component == F64 || component == F80,"invalid complex component");
    Type t = object(component.bytes()*2,component.alignment());
    t.code_ |= std::uint64_t(component.kind()) << 48; return t;
}
std::uint32_t Type::bytes() const
{
    static const unsigned sizes[] = {0,1,1,1,2,2,4,4,8,16,4,8,16,8};
    return kind() == Object ? std::uint32_t(code_ >> 8) : sizes[kind()];
}
std::uint32_t Type::alignment() const
{
    return kind() == Object ? std::uint32_t(1) << ((code_ >> 40) & 255) : kind() == I128 ? 8 : bytes();
}
unsigned Type::width() const
{
    return kind() == I1 ? 1 : kind() == F80 ? 80 : bytes() * 8;
}
Operand Operand::integer(std::uint64_t n) { Operand o; o.data.integer = n; return o; }
std::uint64_t Operand::integer_high() const
{
    if (!wide_integer) return negative_integer ? ~std::uint64_t(0) : 0;
    std::uint64_t high; std::memcpy(&high,reinterpret_cast<const unsigned char*>(&data)+8,8); return high;
}
void Operand::integer_high(std::uint64_t high)
{
    // The unused half of the 16-byte numeric union stores wide integer bits.
    // Object-representation access avoids reading an inactive union member.
    std::memcpy(reinterpret_cast<unsigned char*>(&data)+8,&high,8); wide_integer = true;
}
std::string integer_text(const Operand& value)
{
    std::uint64_t lo = value.data.integer, hi = value.integer_high();
    if (value.negative_integer) { hi = ~hi + (lo == 0); lo = 0-lo; }
    std::string result;
    do {
        std::uint32_t limbs[4] = {std::uint32_t(lo), std::uint32_t(lo>>32), std::uint32_t(hi), std::uint32_t(hi>>32)};
        std::uint64_t remainder = 0;
        for (int n = 3; n >= 0; --n) {
            std::uint64_t part = (remainder << 32) | limbs[n];
            limbs[n] = part/10; remainder = part%10;
        }
        result += char('0'+remainder);
        lo = std::uint64_t(limbs[1])<<32 | limbs[0]; hi = std::uint64_t(limbs[3])<<32 | limbs[2];
    } while (lo || hi);
    if (value.negative_integer) result += '-';
    std::reverse(result.begin(),result.end()); return result;
}
Operand Operand::floating(long double n, bool signaling)
{
    Operand o;
    o.kind = Floating;
    o.data.floating = n;
    o.signaling_nan = signaling;
    return o;
}
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
std::size_t Program::pool_allocations() const
{
    return symbols.allocations + functions.allocations + signatures.allocations + parameters.allocations +
        values.allocations + slots.allocations + blocks.allocations + slot_order.allocations +
        block_order.allocations + instructions.allocations + operands.allocations + globals.allocations +
        data.allocations + aliases.allocations + floating_literals.allocations;
}
std::size_t Program::pool_storage_bytes() const
{
    return symbols.storage_bytes() + functions.storage_bytes() + signatures.storage_bytes() + parameters.storage_bytes() +
        values.storage_bytes() + slots.storage_bytes() + blocks.storage_bytes() + slot_order.storage_bytes() +
        block_order.storage_bytes() + instructions.storage_bytes() + operands.storage_bytes() + globals.storage_bytes() +
        data.storage_bytes() + aliases.storage_bytes() + floating_literals.storage_bytes() + function_order.capacity()*sizeof(FunctionId);
}
ValueId FunctionBuilder::value(Name name)
{
    if (auto id = values_.find(name)) return ValueId(id);
    Value v;
    v.name = name;
    v.owner = function_;
    p_.values.push_back(v);
    if (name) values_.insert(name, p_.values.size());
    return ValueId(p_.values.size());
}
BlockId FunctionBuilder::block(Name name)
{
    if (auto id = blocks_.find(name)) return BlockId(id);
    Block b;
    b.name = name;
    b.owner = function_;
    p_.blocks.push_back(b);
    if (name) blocks_.insert(name, p_.blocks.size());
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
    Function& f = p_.functions.at(function_.index - 1);
    require(!f.slots.count || f.slots.end() == p_.slot_order.size(), "interleaved function slots");
    Slot s;
    s.name = name;
    s.type = type;
    s.owner = function_;
    p_.slots.push_back(s);
    SlotId id(p_.slots.size());
    if (name) slots_.insert(name, id.index);
    if (!f.slots.count) f.slots.begin = p_.slot_order.size();
    ++f.slots.count;
    p_.slot_order.push_back(id);
    return id;
}
void FunctionBuilder::start_block(Name name) { start_block(block(name)); }
void FunctionBuilder::start_block(BlockId id)
{
    Function& f = p_.functions.at(function_.index - 1);
    require(!f.blocks.count || f.blocks.end() == p_.block_order.size(), "interleaved function blocks");
    if (current_) {
        const Block& previous = p_.blocks[current_.index - 1];
        require(previous.instructions.count && terminator(p_.instructions[previous.instructions.end()-1].opcode), "missing terminator");
    }
    Block& b = p_.blocks[id.index - 1];
    require(!b.defined, "duplicate block");
    b.defined = true;
    b.instructions.begin = p_.instructions.size();
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
    validate_instruction_shape(inst);
    require(inst.operands.end() <= p_.operands.size(), "invalid operand slice");
    require(bool(current_), "instruction outside a block");
    Block& b = p_.blocks[current_.index - 1];
    require(b.instructions.end() == p_.instructions.size(), "interleaved block instructions");
    require(!b.instructions.count || !terminator(p_.instructions.back().opcode), "instruction after terminator");
    Type result = inst.result_type();
    require(bool(inst.destination) == (result != Type()), "incorrect instruction destination");
    if (inst.destination) {
        Value& v = p_.values.at(inst.destination.index - 1);
        require(v.owner == function_, "foreign result");
        require(!v.defined || v.type == result, "temporary changes type");
        bool truth = inst.opcode == Opcode::Compare || inst.opcode == Opcode::AtomicCompareExchange;
        v.truth = v.defined ? v.truth && truth : truth;
        if (!v.defined) v.definition = p_.instructions.size() + 1;
        v.defined = true;
        v.type = result;
    }
    p_.instructions.push_back(inst);
    ++b.instructions.count;
}
} // namespace lowir_model
