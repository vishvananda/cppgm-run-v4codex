#include "lowering/flow_proof.h"
#include <stdexcept>
namespace cppgm { namespace lowering {
using namespace lowir_model;
namespace {
// Sparse executable-edge proof, never an IR transform. Each integer fact moves
// Pending -> Constant -> Varying at most once. Only its instruction users are
// revisited. Whole-slot, nonvolatile, unescaped storage may carry a join value;
// all other memory stays unknown. Storage and queues die with this function.
class Reachability {
    const Program& p;
    FunctionId function;
    BlockId target;
    Range range;
    const std::vector<BlockId>& handlers;
    std::size_t& work;
    std::size_t& edges;
    unsigned first_value = 0, value_count = 0, first_slot = 0, first_block;
    struct Use { unsigned instruction, next; };
    std::vector<Use> uses = std::vector<Use>(1);
    std::vector<unsigned> heads, pending;
    std::vector<FlowInteger> facts;
    std::vector<unsigned char> safe, active, queued, reached;
    bool found = false;
    unsigned value(unsigned id) const {
        return id >= first_value && id-first_value < value_count ? id-first_value : ~0u;
    }
    unsigned slot(unsigned id) const {
        return id >= first_slot && id-first_slot < safe.size() ? id-first_slot : ~0u;
    }
    FlowInteger get(const Operand& operand) const {
        if (operand.kind == Operand::Integer && !operand.wide_integer)
            return FlowInteger(FlowInteger::Constant,operand.data.integer);
        auto id = operand.kind == Operand::Temporary ? value(operand.ref) : ~0u;
        return id != ~0u ? facts[id] : FlowInteger(FlowInteger::Varying);
    }
    void enqueue(unsigned n) {
        auto i = n-range.begin;
        if (active[i] && !queued[i]) { queued[i] = 1; pending.push_back(n); }
    }
    void publish(unsigned id, FlowInteger next) {
        auto& old = facts[id];
        if (next.state == FlowInteger::Pending || old.state == FlowInteger::Varying) return;
        if (old.state == FlowInteger::Constant) {
            if (next.state == FlowInteger::Constant && next.bits == old.bits) return;
            next.state = FlowInteger::Varying;
        }
        old = next;
        for (auto u = heads[id]; u; u = uses[u].next) { ++edges; enqueue(uses[u].instruction); }
    }
    SignatureId signature(const Instruction& i) const {
        auto result = i.signature;
        auto callee = p.operands[i.operands.begin];
        if (callee.kind == Operand::Symbol) {
            const auto& s = p.symbols[callee.ref-1];
            if (s.kind == Symbol::FunctionSymbol) result = p.functions[s.entity-1].signature;
        }
        return result;
    }
    void reach(BlockId b) {
        ++edges;
        if (b.index < first_block || b.index > p.blocks.size() || p.blocks[b.index-1].owner.index != function.index)
            throw std::logic_error("foreign fallthrough edge");
        auto& seen = reached[b.index-first_block]; if (seen) return; seen = 1;
        auto body = p.blocks[b.index-1].instructions;
        for (auto n = body.begin; n < body.end(); ++n) {
            ++work; active[n-range.begin] = 1; enqueue(n);
            if (p.instructions[n].opcode == Opcode::Call) {
                auto sig = signature(p.instructions[n]);
                if (sig && p.signatures[sig.index-1].boundary.returns == ir_model::CRM_NORETURN) return;
            }
        }
        found |= b == target;
    }
    void process(unsigned n) {
        ++work;
        const auto& i = p.instructions[n]; auto begin = i.operands.begin;
        auto a = i.operands.count ? get(p.operands[begin]) : FlowInteger(FlowInteger::Varying);
        auto b = i.operands.count > 1 ? get(p.operands[begin+1]) : FlowInteger(FlowInteger::Varying);
        if (i.destination) {
            auto result = flow_integer(i,a,b);
            if (i.opcode == Opcode::Load && p.operands[begin].kind == Operand::Slot) {
                auto s = slot(p.operands[begin].ref);
                if (s != ~0u && safe[s]) result = facts[value_count+s];
            }
            publish(value(i.destination.index),result);
        }
        if (i.opcode == Opcode::Store && p.operands[begin+1].kind == Operand::Slot) {
            auto s = slot(p.operands[begin+1].ref);
            if (s != ~0u && safe[s]) publish(value_count+s,flow_integer(Instruction(Opcode::Copy,i.type),a,b));
        }
        if (i.opcode == Opcode::Call) {
            auto sig = signature(i);
            if ((!sig || p.signatures[sig.index-1].boundary.unwind != ir_model::CUM_NO) && handlers[n-range.begin])
                reach(handlers[n-range.begin]);
        }
        if ((i.opcode == Opcode::Throw || i.opcode == Opcode::Resume) && handlers[n-range.begin]) reach(handlers[n-range.begin]);
        if (i.opcode == Opcode::Branch || i.opcode == Opcode::Switch) {
            if (a.state == FlowInteger::Pending) return;
            if (a.state == FlowInteger::Constant) {
                if (i.opcode == Opcode::Branch) reach(BlockId(p.operands[begin+(a.bits ? 1 : 2)].ref));
                else {
                    auto selected = p.operands[begin+1];
                    for (auto j = begin+2; j < i.operands.end(); j += 2)
                        if (p.operands[j].data.integer == a.bits) { selected = p.operands[j+1]; break; }
                    reach(BlockId(selected.ref));
                }
                return;
            }
        } else if (i.opcode != Opcode::Jump) return;
        for (auto j = begin; j < i.operands.end(); ++j)
            if (p.operands[j].kind == Operand::Label) reach(BlockId(p.operands[j].ref));
    }
public:
    Reachability(const Program& program, FunctionId f, BlockId end, Range instructions,
        const std::vector<BlockId>& h, std::size_t& w, std::size_t& e)
        : p(program), function(f), target(end), range(instructions), handlers(h), work(w), edges(e),
          first_block(p.block_order[p.functions[f.index-1].blocks.begin].index) {
        unsigned last = 0;
        for (auto n = range.begin; n < range.end(); ++n) {
            ++work; auto id = p.instructions[n].destination.index; if (!id) continue;
            if (!first_value || id < first_value) first_value = id;
            if (id > last) last = id;
        }
        value_count = first_value ? last-first_value+1 : 0;
        auto slots = p.functions[f.index-1].slots;
        if (slots.count) first_slot = p.slot_order[slots.begin].index;
        safe.resize(slots.count,1); facts.resize(value_count+slots.count); heads.resize(facts.size());
        active.resize(range.count); queued.resize(range.count); reached.resize(p.blocks.size()-first_block+1);
        for (auto n = range.begin; n < range.end(); ++n) {
            ++work; const auto& i = p.instructions[n];
            for (auto j = i.operands.begin; j < i.operands.end(); ++j) {
                ++edges; const auto& operand = p.operands[j]; unsigned id = ~0u;
                if (operand.kind == Operand::Temporary) id = value(operand.ref);
                if (operand.kind == Operand::Slot) {
                    auto s = slot(operand.ref);
                    if (s == ~0u) throw std::logic_error("foreign flow slot");
                    bool load = i.opcode == Opcode::Load && j == i.operands.begin;
                    bool store = i.opcode == Opcode::Store && j == i.operands.begin+1;
                    if ((!load && !store) || i.is_volatile || !i.type.integer() || i.type.width() > 64 || i.type != p.slots[operand.ref-1].type) safe[s] = 0;
                    if (load) id = value_count+s;
                }
                if (id != ~0u) { uses.push_back({n,heads[id]}); heads[id] = uses.size()-1; }
            }
        }
    }
    bool run() {
        reach(BlockId(first_block));
        for (unsigned pass = 0; pass != 2; ++pass) {
            while (!pending.empty() && !found) {
                auto n = pending.back(); pending.pop_back(); queued[n-range.begin] = 0; process(n);
            }
            if (found) return true;
            // Unresolved cycles/undefined inputs never prove an edge dead.
            // Seed them conservatively once, then drain only their users.
            if (!pass) for (unsigned id = 0; id < facts.size(); ++id)
                if (facts[id].state == FlowInteger::Pending) publish(id,FlowInteger(FlowInteger::Varying));
        }
        return false;
    }
};
}
bool flow_reaches(const Program& p, FunctionId f, BlockId target, Range range,
    const std::vector<BlockId>& handlers, std::size_t& work, std::size_t& edges)
{ return Reachability(p,f,target,range,handlers,work,edges).run(); }
} }
