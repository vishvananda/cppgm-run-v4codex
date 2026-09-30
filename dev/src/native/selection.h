#pragma once
#include "native/model.h"
namespace native {
struct ValueState {
    Operand location;
    unsigned definition = 0, last = 0, uses = 0, block = 0, call_epoch = 0;
    bool crosses_block = false, crosses_call = false, compare_branch = false;
};
// Unit-owned dense identity tables, initialized once, reused by functions. No
// whole-unit scan per function and no string-keyed placement decisions.
struct Workspace {
    std::vector<ValueState> values;
    std::vector<Operand> slots;
    explicit Workspace(const lowir_model::Program& p) : values(p.values.size()+1), slots(p.slots.size()+1) {}
};
class Selector {
    const lowir_model::Program& p;
    const lowir_model::Function& source;
    Workspace& workspace;
    Statistics& stats;
    Function f;
    std::array<unsigned,16> live_until = {{0}};
    unsigned position = 0, block_id = 0;
    DebugLocation debug;
    std::vector<unsigned> definitions;
    struct EdgeMove { unsigned pred, target, destination; lowir_model::Operand source; Operand staging; };
    struct EdgeBlock { unsigned pred, target, label; };
    std::vector<EdgeMove> edge_moves;
    std::vector<EdgeBlock> edge_blocks;
    unsigned next_label = 0;
    void edge_transfers(unsigned pred, unsigned target);
    unsigned edge_target(unsigned target);
    void analyze();
    void analyze_instruction(const lowir_model::Instruction& i, unsigned epoch);
    void parameters();
    Operand home(Name name, Type type, bool temporary);
    Operand allocate(unsigned value, Type type);
    Operand value(lowir_model::Operand o, Type context);
    Type value_type(lowir_model::Operand o, Type fallback) const;
    Operand memory(lowir_model::Operand o, int scratch = XR_R11);
    Operand in_register(Operand o, Type type, int reg);
    void move(Operand to, Operand from, Type type);
    void normalize_register(Operand to, Type type);
    Instruction& emit(Op op, Type type, std::initializer_list<Operand> args);
    void select(const lowir_model::Instruction& i);
    void arithmetic(const lowir_model::Instruction& i);
    void compare(const lowir_model::Instruction& i, bool branch);
    void conversion(const lowir_model::Instruction& i);
    void index(const lowir_model::Instruction& i);
    void call(const lowir_model::Instruction& i);
    void atomic(const lowir_model::Instruction& i);
    void bulk(const lowir_model::Instruction& i);
    void control(const lowir_model::Instruction& i);
    void begin_block(unsigned id, Name name);
    void finish_frame();
    lowir_model::Operand arg(const lowir_model::Instruction& i, unsigned n) const { return p.operands[i.operands.begin+n]; }
    ValueState& state(unsigned v) { return workspace.values[v]; }
public:
    Selector(const lowir_model::Program& p, const lowir_model::Function& source, Workspace& workspace, Statistics& stats)
        : p(p), source(source), workspace(workspace), stats(stats) {}
    Function run();
};
} // namespace native
