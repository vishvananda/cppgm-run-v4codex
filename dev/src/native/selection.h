#pragma once
#include "native/model.h"
namespace native {
struct ValueState {
    Operand location;
    unsigned writes = 1;
    unsigned definition = 0, last = 0, uses = 0, block = 0, call_epoch = 0, alias = 0, other_block = 0;
    bool folded_load = false, folded_index = false, address_only = true, single_edge = false;
    bool crosses_block = false, crosses_call = false, compare_branch = false;
    bool converted_boolean = false;
};
// Unit-owned dense identity tables, initialized once, reused by functions. No
// whole-unit scan per function and no string-keyed placement decisions.
struct SlotState { unsigned stored = 0, position = 0; bool escape = false, observed = false; };
struct Workspace {
    unsigned strlen_prefix_budget = 128;
    std::vector<unsigned> value_indices, tls_wrappers, exception_handlers;
    std::vector<Operand> slots;
    std::vector<SlotState> slot_facts;
    std::vector<unsigned> block_epochs, predecessor_count, successor, next_block, parameter_clobbers, parameter_exits, block_local;
    explicit Workspace(const lowir_model::Program& p) : value_indices(p.values.size()+1), tls_wrappers(p.symbols.size()+1), exception_handlers(p.blocks.size()+1), slots(p.slots.size()+1), slot_facts(p.slots.size()+1), block_epochs(p.blocks.size()+1), predecessor_count(p.blocks.size()+1), successor(p.blocks.size()+1), next_block(p.blocks.size()+1), parameter_clobbers(p.blocks.size()+1), parameter_exits(p.blocks.size()+1), block_local(p.blocks.size()+1) {
        for (const auto& s : p.functions) {
            auto target = p.symbols[s.symbol.index-1].metadata.tls_for;
            if (target) tls_wrappers[target.index] = s.symbol.index;
        }
    }
};
class Selector {
    const lowir_model::Program& p;
    const lowir_model::Function& source;
    Workspace& workspace;
    Statistics& stats;
    Function f;
    unsigned level;
    unsigned strlen_prefix_sites = 0;
    void retain_global_values(bool handlers);
    void cleanup_control();
    bool mixed_conversion_abi = false;
    std::uint64_t parameter_bytes = 0;
    Operand vararg_save, indirect_result, atomic_scratch;
    struct Assignment { Operand to, from; Type type; Operand address_home; bool done = false; };
    // Call setup reuses function-owned scratch. Capacity tracks the largest
    // argument list and is released with this selector, never per call node.
    std::vector<Assignment> call_register_moves, call_stack_moves;
    unsigned vararg_gp = 0, vararg_fp = 0, vararg_stack = 16;
    std::vector<ValueState> values;
    void index_exception_clauses();
    void initialize_values();
    std::array<unsigned,32> live_until = {{0}};
    unsigned position = 0, block_id = 0;
    unsigned parameter_clobbers = 0;
    std::array<unsigned,16> first_clobber;
    void promote_parameters();
    void aliases();
    void folds();
    unsigned root(unsigned v) const { return state(v).alias ? state(v).alias : v; }
    DebugLocation debug;
    std::vector<unsigned> definitions;
    struct EdgeMove { unsigned pred, target, destination, label = 0; lowir_model::Operand source; Operand staging; };
    struct EdgeBlock { unsigned pred, target, label; };
    std::vector<EdgeMove> edge_moves;
    std::vector<EdgeBlock> edge_blocks;
    unsigned next_label = 0;
    void edge_transfers(unsigned pred, unsigned target);
    unsigned edge_target(unsigned target);
    void analyze();
    void control_edges();
    void parameter_availability();
    unsigned clobbers(const lowir_model::Instruction& i) const;
    bool call_effect(const lowir_model::Instruction& i) const;
    void analyze_instruction(const lowir_model::Instruction& i, unsigned epoch);
    void parameters();
    Operand fragment(Operand value, unsigned byte_offset);
    void object_move(Operand to, Operand from, Type type);
    void wide_arithmetic(const lowir_model::Instruction& i);
    void wide_compare(const lowir_model::Instruction& i, bool branch);
    void wide_division(const lowir_model::Instruction& i);
    void wide_shift(const lowir_model::Instruction& i);
    void wide_atomic(const lowir_model::Instruction& i);
    void wide_conversion(const lowir_model::Instruction& i);
    void save_variadic_registers();
    void variadic(const lowir_model::Instruction& i);
    Operand home(Name name, Type type, bool temporary);
    Operand allocate(unsigned value, Type type);
    Operand value(lowir_model::Operand o, Type context);
    Type value_type(lowir_model::Operand o, Type fallback) const;
    Type consumed_type(lowir_model::Operand o, Type context) const;
    Operand memory(lowir_model::Operand o, int scratch = XR_R11);
    bool tls_symbol(unsigned id) const;
    bool imported_data(unsigned id) const;
    void tls_address(Operand to, Operand symbol);
    Operand in_register(Operand o, Type type, int reg);
    void move(Operand to, Operand from, Type type);
    void normalize_register(Operand to, Type type);
    Instruction& emit(Op op, Type type, std::initializer_list<Operand> args);
    void select(const lowir_model::Instruction& i);
    void arithmetic(const lowir_model::Instruction& i);
    void compare(const lowir_model::Instruction& i, bool branch);
    void conversion(const lowir_model::Instruction& i);
    void floating_arithmetic(const lowir_model::Instruction& i);
    void floating_compare(const lowir_model::Instruction& i, bool branch);
    Operand convert_value(Operand from, Type source, Type target, bool unsigned_input = false, bool unsigned_output = false);
    void convert_to(Operand to, Operand from, Type source, Type target, bool unsigned_input = false, bool unsigned_output = false);
    void index(const lowir_model::Instruction& i);
    void call(const lowir_model::Instruction& i);
    bool dynamic_copy(const lowir_model::Instruction&, lowir_model::SignatureId) const;
    void atomic(const lowir_model::Instruction& i);
    void bulk(const lowir_model::Instruction& i);
    void control(const lowir_model::Instruction& i);
    void runtime(const lowir_model::Instruction& i);
    void begin_block(unsigned id, Name name);
    void finish_frame();
    void carry_reloads();
    lowir_model::Operand arg(const lowir_model::Instruction& i, unsigned n) const { return p.operands[i.operands.begin+n]; }
    ValueState& state(unsigned v) { return values[workspace.value_indices[v]]; }
    const ValueState& state(unsigned v) const { return values[workspace.value_indices[v]]; }
public:
    Selector(const lowir_model::Program& p, const lowir_model::Function& source, Workspace& workspace, Statistics& stats, bool host = false, unsigned level = 0)
        : p(p), source(source), workspace(workspace), stats(stats), level(level) { f.host = host; }
    Function run();
};
} // namespace native
