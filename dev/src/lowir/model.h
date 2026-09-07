#pragma once
#include "ir_symbol_model.h"
#include "preprocess/identifier_table.h"
#include <initializer_list>
#include <iosfwd>
#include <stdexcept>
#include <vector>

namespace lowir_model {
using namespace ir_model;
using Name = cppgm::IdentifierId;
struct ParseError : std::runtime_error { using std::runtime_error::runtime_error; };
void require(bool condition, const char* message);
template<class Tag> struct Id {
    std::uint32_t index;
    explicit Id(std::uint32_t n = 0) : index(n) {}
    explicit operator bool() const { return index != 0; }
    bool operator==(Id b) const { return index == b.index; }
};
using SymbolId = Id<struct SymbolTag>;
using ValueId = Id<struct ValueTag>;
using SlotId = Id<struct SlotTag>;
using BlockId = Id<struct BlockTag>;
using FunctionId = Id<struct FunctionTag>;
using SignatureId = Id<struct SignatureTag>;
struct Range {
    std::uint32_t begin = 0, count = 0;
    std::uint32_t end() const { return begin + count; }
};
// Numeric open addressing, no per-entry allocation. Zero means absent.
// Each index has one owner (unit symbols or one function's local names).
class NameIndex {
    struct Entry { Name key = 0; std::uint32_t value = 0; };
    std::vector<Entry> slots_;
    std::size_t size_ = 0;
    void grow();
public:
    NameIndex() : slots_(16) {}
    std::uint32_t find(Name name) const;
    bool insert(Name name, std::uint32_t value);
};
// Canonical inline identity: kind, object byte count, log2 alignment.
class Type {
    std::uint64_t code_;
public:
    enum Kind { Void, I1, I8, U8, I16, U16, I32, U32, I64, F32, F64, F80, Ptr, Object };
    Type(Kind k = Void) : code_(k) {}
    static Type object(std::uint32_t bytes, std::uint32_t alignment);
    Kind kind() const { return Kind(code_ & 255); }
    bool integer() const { return kind() >= I1 && kind() <= I64; }
    bool floating() const { return kind() >= F32 && kind() <= F80; }
    bool scalar() const { return integer() || floating() || kind() == Ptr; }
    std::uint32_t bytes() const;
    std::uint32_t alignment() const;
    unsigned width() const;
    bool operator==(Type b) const { return code_ == b.code_; }
    bool operator!=(Type b) const { return !(*this == b); }
};
struct Operand {
    enum Kind { Integer, Floating, Null, Temporary, Slot, Symbol, Label } kind = Integer;
    std::uint32_t ref = 0;
    bool signaling_nan = false;
    bool negative_integer = false;
    union Payload {
        std::uint64_t integer;
        long double floating;
        Payload() : integer(0) {}
    } data;
    static Operand integer(std::uint64_t n);
    static Operand floating(long double n, bool signaling = false);
    static Operand null();
    static Operand value(ValueId id);
    static Operand slot(SlotId id);
    static Operand symbol(SymbolId id);
    static Operand label(BlockId id);
    bool literal() const { return kind <= Null; }
};
struct DebugLocation { Name file = 0; std::uint32_t line = 0, column = 0; };
struct SymbolMetadata {
    SymbolRole role = SR_NONE;
    LanguageLinkageMode linkage = LLM_DEFAULT;
    SymbolBindingMode binding = SBM_DEFAULT;
    GlobalStorageMode storage = GSM_DEFAULT;
    Name object = 0, section = 0;
    SymbolId tls_for;
    bool keep_alias = false, prefer_local = false, object_root = false;
    bool force_inline = false, inline_hint = false, no_inline = false;
};
struct Parameter {
    ValueId value;
    Type type;
    ParamPassingMode passing = PPM_DIRECT;
    ParamAliasMode alias = PALM_DEFAULT;
    std::uint64_t object_bytes = 0;
};
struct Signature { Range parameters; Type result; FunctionBoundaryMetadata boundary; };
struct Value {
    Name name = 0;
    Type type;
    FunctionId owner;
    bool defined = false, truth = false;
    // First definition ordinal; phi edge inputs are checked after the body.
    std::uint32_t definition = 0;
};
struct Slot { Name name = 0; Type type; FunctionId owner; };
struct Block {
    Name name = 0;
    FunctionId owner;
    Range instructions;
    bool defined = false;
};
enum class Opcode {
    Const, Copy, Phi, Addr, Load, Store, Index, Unary, Binary, Compare, Convert,
    AtomicLoad, AtomicStore, AtomicAddFetch, AtomicExchange, AtomicCompareExchange,
    AtomicThreadFence, AtomicSignalFence, VaStart, VaArg, StackAlloc, Call,
    CopyObject, ZeroInit, EhTry, EhCleanup, EhCatch, EhFilter, EhCatchAll, EhEnd,
    Throw, Exception, ExceptionSelector, Resume, Jump, Branch, Switch, Return, Unreachable
};
enum class Operation {
    None, Neg, Not, Bitnot, Bswap, Add, Sub, Mul, Div, Mod, Udiv, Umod, And, Or,
    Xor, Shl, Shr, Ushr, Eq, Ne, Lt, Le, Gt, Ge, Ult, Ule, Ugt, Uge,
    Sext, Zext, Trunc, Sitofp, Uitofp, Fptosi, Fptoui, Fpext, Fptrunc
};
const char* spelling(Opcode op);
const char* spelling(Operation op);
Opcode parse_opcode(const std::string& word);
Operation parse_operation(const std::string& word);
bool terminator(Opcode op);
struct Instruction {
    Opcode opcode;
    Operation operation = Operation::None;
    Type type, source_type;
    ValueId destination;
    Range operands;
    SignatureId signature;
    std::uint64_t bytes = 0;
    std::uint32_t alignment = 1;
    IndexProjectionKind projection = IPK_NONE;
    bool is_volatile = false, copy_elision = false;
    DebugLocation debug;
    explicit Instruction(Opcode op = Opcode::Const, Type t = Type()) : opcode(op), type(t) {}
    Type result_type() const;
};
void validate_instruction_shape(const Instruction& i);
struct Function {
    SymbolId symbol;
    SignatureId signature;
    Range slots, blocks; // slices of IDs, preserving source order
    DebugLocation debug;
    bool declaration = false;
};
struct DataItem {
    enum Kind { Scalar, Address, Zero } kind = Zero;
    Type type;
    Operand value;
    SymbolId symbol;
    std::int64_t addend = 0;
    std::uint64_t zero_bytes = 0;
};
struct Global {
    SymbolId symbol;
    Type type;
    Range data;
    bool declaration = false, structured = false;
};
struct Symbol {
    enum Kind { Unknown, FunctionSymbol, GlobalSymbol } kind = Unknown;
    Name name = 0;
    std::uint32_t entity = 0;
    SymbolMetadata metadata;
};
struct ObjectAlias { Name name = 0; SymbolId target; };
struct Statistics {
    std::uint64_t source_bytes = 0, tokens = 0, validated_instructions = 0, cfg_edges = 0;
};
// Pool growth is counted where it happens, without a global allocator hook.
// Timing/peak RSS and capacity accounting are emitted only when requested.
template<class T> class Pool : public std::vector<T> {
    using Base = std::vector<T>;
public:
    std::uint64_t allocations = 0;
    void push_back(const T& v) {
        bool grows = this->size() == this->capacity();
        Base::push_back(v);
        allocations += grows;
    }
    void insert(typename Base::const_iterator at, std::initializer_list<T> items) {
        bool grows = this->size() + items.size() > this->capacity();
        Base::insert(at, items);
        allocations += grows;
    }
    std::size_t storage_bytes() const { return this->capacity() * sizeof(T); }
};
// One unit owns all pools. IDs survive geometric growth. Instructions and
// operands have no owning children; signatures and block bodies use slices.
// Text input buffers die after each read; no textual IR survives in this model.
struct Program {
    cppgm::IdentifierTable names;
    NameIndex symbol_names;
    Pool<Symbol> symbols;
    Pool<Function> functions;
    Pool<Signature> signatures;
    Pool<Parameter> parameters;
    Pool<Value> values;
    Pool<Slot> slots;
    Pool<Block> blocks;
    Pool<SlotId> slot_order;
    Pool<BlockId> block_order;
    Pool<Instruction> instructions;
    Pool<Operand> operands;
    Pool<Global> globals;
    Pool<DataItem> data;
    Pool<ObjectAlias> aliases;
    Statistics stats;
    Name intern(const std::string& name);
    std::string name(Name id) const;
    SymbolId symbol(Name name);
    std::size_t pool_allocations() const;
    std::size_t pool_storage_bytes() const;
};
// A single function construction scope also owns its forward local references.
// append enforces local result/terminator structure; validate resolves external
// input-wide type, signature and CFG requirements once after all files arrive.
class FunctionBuilder {
    Program& p_;
    FunctionId function_;
    NameIndex values_, slots_, blocks_;
    BlockId current_;
public:
    FunctionBuilder(Program& p, FunctionId f) : p_(p), function_(f) {}
    ValueId value(Name name);
    BlockId block(Name name);
    SlotId slot(Name name) const;
    void parameter(Parameter parameter);
    SlotId add_slot(Name name, Type type);
    void start_block(Name name);
    ValueId append(Instruction inst, std::initializer_list<Operand> operands, Name dest = 0);
    void append(Instruction inst);
};
void read_program(Program& p, const std::string& text, const std::string& source);
void validate(Program& p);
void write_program(const Program& p, std::ostream& out);
Program construct_exercise(const std::string& name);
Program parse_lowir_program_text(const std::string& text, const std::string& source = "<memory>");
Program parse_lowir_program_files(const std::vector<std::string>& paths);
std::string serialize_lowir_program(const Program& p);
using LowirProgram = Program;
} // namespace lowir_model
