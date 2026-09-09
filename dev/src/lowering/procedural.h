#pragma once
#include "semantic/analyzer.h"
#include "lowir/model.h"
#include "abi/itanium/graph.h"
#include <memory>
namespace cppgm { namespace lowering {
using semantic::EntityId;
using semantic::TypeId;
using semantic::NodeId;
using semantic::TypeKind;
using semantic::ValueCategory;
using lowir_model::Operand;
using lowir_model::Instruction;
using lowir_model::Opcode;
using lowir_model::Operation;
using lowir_model::SymbolId;
using lowir_model::SlotId;
using lowir_model::BlockId;
using lowir_model::FunctionId;
using lowir_model::SignatureId;
using IRType = lowir_model::Type;
struct Value {
    Operand operand;
    IRType ir;
    TypeId type = 0;
    bool address = false, cached = false;
    Operand stored;
    Value() {}
    Value(Operand o, IRType i, TypeId t = 0, bool a = false) : operand(o), ir(i), type(t), address(a) {}
};
// Program-owned linkage identities survive individual semantic TUs. Keys use
// canonical typed ABI entities, never their rendered manglings.
struct Linkage {
    abi_mangle::Graph abi;
    semantic::Index external;
    std::size_t requests = 0, hits = 0;
    std::uint64_t disambiguator = 0;
    bool merge;
    explicit Linkage(bool merge) : merge(merge) {}
};
// The semantic TU outlives this adapter; all mappings are dense canonical IDs.
// Function-local construction state is discarded when its body is complete.
class Procedural {
    syntax::Ast& ast;
    semantic::Analyzer& sem;
    IdentifierTable& identifiers;
    lowir_model::Program& p;
    Linkage& linkage;
    abi_mangle::Graph& abi;
    std::vector<abi_mangle::Id> abi_types, abi_scopes;
    std::vector<SymbolId> symbols, strings;
    std::vector<SlotId> objects;
    std::vector<BlockId> labels;
    std::vector<unsigned char> control_entries;
    std::vector<unsigned char> discard_accesses;
    std::vector<SignatureId> indirect_signatures;
    std::vector<Operand> call_work;
    std::vector<EntityId> definitions, global_initializers;
    std::unique_ptr<lowir_model::FunctionBuilder> builder;
    FunctionId function;
    TypeId returned = 0;
    SlotId this_slot;
    BlockId break_target, continue_target;
    bool ended = false;
    std::uint32_t live = 0;
    bool emitting_cleanup = false, resume_emitted = false;
    std::size_t cleanup_cursor = 0;
    semantic::Index slot_names;
    SlotId cleanup_return;
    BlockId resume_terminal, destructor_handler, destructor_end, destructor_epilogue;
    semantic::Index cleanup_index, return_terminals;
    struct Cleanup { std::uint32_t state; BlockId next, block; };
    std::vector<Cleanup> cleanup_blocks;
    struct Constructed { semantic::SubobjectAction action; BlockId handler; };
    std::vector<Constructed> constructed_subobjects;
    EntityId active_function = 0;
    NodeId child(NodeId n, syntax::Kind k) const;
    std::string spelling(IdentifierId id) const;
    abi_mangle::Id abi_type(TypeId t);
    abi_mangle::Id abi_scope(semantic::ScopeId s);
    SymbolId symbol(EntityId e);
    SymbolId fresh_symbol(const std::string& preferred);
    SignatureId signature(TypeId t, FunctionId owner = FunctionId());
    void function_body(EntityId e);
    void reset_lifetime(EntityId e);
    void destroy(EntityId destructor, TypeId t, Value object);
    void destroy_object(EntityId object, EntityId destructor);
    void clean_inline(std::uint32_t state, std::uint32_t stop);
    BlockId cleanup_suffix(std::uint32_t state, BlockId terminal);
    void emit_cleanups();
    void flush_cleanups();
    SlotId source_slot(EntityId e);
    Value guarded_call(Instruction i, const Operand* args, std::size_t count);
    void return_statement(NodeId n);
    void destructor_prologue(EntityId e);
    void destructor_finish(EntityId e);
    void destroy_subobjects(EntityId e);
    void finish_constructor_handlers();
    void constructor_cleanup(semantic::SubobjectAction action);
    struct InitProjection { std::uint64_t offset; bool field; };
    Value initialization_address(Value root, bool indirect, const std::vector<InitProjection>& path);
    void aggregate_initialize(NodeId n, TypeId t, Value root, bool indirect, std::vector<InitProjection>& path);
    void constructor_body(EntityId e);
    void construct(EntityId ctor, NodeId init, Value object);
    bool constant_initializer(NodeId n, TypeId t);
    void global_initialization();
    void global_finalization();
    void global(EntityId e);
    void string_literal(NodeId n);
    void global_data(NodeId n, TypeId t);
    lowir_model::DataItem constant_data(NodeId n, TypeId t);
    IRType type(TypeId t);
    bool reference(TypeId t) const;
    Value emit(Instruction i, const Operand* args, std::size_t count);
    Value emit(Instruction i, const std::vector<Operand>& args);
    Value emit(Instruction i, std::initializer_list<Operand> args);
    Value emit(Opcode op, IRType t, std::initializer_list<Operand> args, Operation action = Operation::None);
    Value load(Value v);
    Value address(Value v);
    Value convert(Value v, TypeId target, bool fold_widen = false);
    Value coerce(Value v, IRType target, bool unsign = false, bool to_unsigned = false, bool fold_widen = false);
    Value converted(NodeId n, const semantic::Conversion& c);
    Value incoming(NodeId n);
    Value expression(NodeId n, bool location = false);
    bool discarded_access(NodeId n);
    void discard(NodeId n, bool access = true);
    Value unary(NodeId n);
    Value binary(NodeId n, bool location);
    Value conditional(NodeId n, bool location);
    Value logical(NodeId n);
    Value call(NodeId n);
    Value operation(ETokenType op, Value a, Value b, TypeId result);
    Value binding(EntityId e);
    Value field(Value base, EntityId e, unsigned steps = 0);
    Value base_projection(Value base, unsigned steps);
    void store(Value v, Value location);
    void initialize(NodeId n, TypeId t, Value location);
    void object(EntityId e);
    void statement(NodeId n);
    bool mark_control_entries(NodeId n);
    void condition(NodeId n, BlockId yes, BlockId no);
    void switch_statement(NodeId n);
    void collect_cases(NodeId n, std::vector<NodeId>& cases, NodeId& fallback);
    BlockId block();
    void start(BlockId b);
    void jump(BlockId b);
public:
    std::size_t control_work = 0, discard_work = 0;
    Procedural(syntax::Ast& a, semantic::Analyzer& s, IdentifierTable& ids, lowir_model::Program& out, Linkage& links);
    void run();
};
int emit_lowir(const std::string& output, const std::vector<std::string>& inputs, bool stats, bool audit = false);
} }
