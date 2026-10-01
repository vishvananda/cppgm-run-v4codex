#include "lowir/model.h"
#include "support/id_index.h"
#include <algorithm>
namespace lowir_model {
namespace {
// Mandatory attribute expansion, not an optimization-level heuristic. Original
// bodies are immutable during this traversal. Every call consumes its own budget;
// cycles, unsupported frame semantics and exhausted budgets retain valid calls.
// Eligibility never relaxes the existing call boundary. No fixed-point scan or semantic reconstruction is involved.
class Expander {
    Program& p;
    Pool<Instruction> instructions;
    Pool<Operand> operands;
    Pool<Block> blocks;
    Pool<BlockId> order;
    Pool<SlotId> slots;
    std::vector<Function> functions;
    std::vector<bool> active;
    std::vector<unsigned> costs;
    cppgm::IdIndex return_depths;
    FunctionId owner;
    BlockId current;
    unsigned work = 0;
    std::uint64_t total = 0;
    struct Context {
        cppgm::IdIndex values, slots, entries, exits;
        std::vector<std::pair<unsigned,unsigned>> phi_edges;
        bool clone = false, straight = false;
        BlockId continuation;
        SlotId result;
        ValueId straight_result;
    };
    void charge(unsigned amount) {
        // Admission reserves an upper bound before mutation; this counter
        // measures actual copied/synthesized instruction, operand and slot work.
        p.stats.inline_work += amount;
    }
    BlockId block() {
        Block b; b.owner = owner; p.blocks.push_back(b); return BlockId(p.blocks.size());
    }
    SlotId slot(Type type) {
        charge(1); Slot s; s.type = type; s.owner = owner; p.slots.push_back(s);
        SlotId id(p.slots.size()); p.slot_order.push_back(id); return id;
    }
    ValueId value(Type type) {
        Value v; v.type = type; v.owner = owner;
        p.values.push_back(v); return ValueId(p.values.size());
    }
    void start(BlockId id) {
        current = id; auto& b = p.blocks[id.index-1];
        b.defined = true; b.instructions.begin = p.instructions.size(); p.block_order.push_back(id);
    }
    void emit(Instruction i, const Operand* args, unsigned count, bool added) {
        if (added) charge(1+count);
        i.operands.begin = p.operands.size(); i.operands.count = count;
        for (unsigned n = 0; n < count; ++n) p.operands.push_back(args[n]);
        validate_instruction_shape(i);
        if (i.destination) {
            auto& v = p.values[i.destination.index-1];
            if (!v.defined) v.definition = p.instructions.size()+1;
            v.defined = true; v.type = i.result_type();
            v.truth = i.opcode == Opcode::Compare || i.opcode == Opcode::AtomicCompareExchange;
        }
        p.instructions.push_back(i); ++p.blocks[current.index-1].instructions.count;
    }
    void emit(Instruction i, const std::vector<Operand>& args, bool added = true) { emit(i,args.data(),args.size(),added); }
    void emit(Instruction i, std::initializer_list<Operand> args) { emit(i,args.begin(),args.size(),true); }
    Operand mapped(Operand a, const Context& c) {
        if (a.kind == Operand::Temporary && c.clone) a.ref = c.values.get(a.ref);
        else if (a.kind == Operand::Slot && c.clone) a.ref = c.slots.get(a.ref);
        else if (a.kind == Operand::Label) a.ref = c.entries.get(a.ref);
        require(a.literal() || a.ref,"missing inline operand identity"); return a;
    }
    unsigned target(const Instruction& i) const {
        if (i.opcode != Opcode::Call) return 0;
        auto callee = operands[i.operands.begin];
        if (callee.kind != Operand::Symbol) return 0;
        const auto& symbol = p.symbols[callee.ref-1];
        return symbol.kind == Symbol::FunctionSymbol && symbol.metadata.force_inline && !symbol.metadata.no_inline &&
            !functions[symbol.entity-1].declaration ? symbol.entity : 0;
    }
    std::uint64_t return_regions(unsigned f) {
        // A return retires all registrations belonging to the callee's frame.
        // A cloned return must perform those pops before joining its caller.
        // Analyze only demanded callee CFG edges, once per immutable body.
        cppgm::IdIndex incoming, cleanup;
        std::vector<unsigned> pending;
        auto body = functions[f-1].blocks;
        std::uint64_t retirement = 0;
        for (unsigned b = body.begin; b < body.end(); ++b) {
            auto id = order[b].index; auto range = blocks[id-1].instructions;
            for (unsigned n = range.begin; n < range.end(); ++n) {
                auto i = instructions[n];
                if (i.opcode == Opcode::EhCleanup && !i.operands.count) cleanup.put(id,1);
                if (i.opcode != Opcode::EhCatch && i.opcode != Opcode::EhCatchAll &&
                    i.opcode != Opcode::EhFilter && !(i.opcode == Opcode::EhCleanup && !i.operands.count)) break;
            }
        }
        auto edge = [&](unsigned id, unsigned depth) {
            auto old = incoming.get(id);
            if (!old) { incoming.put(id,depth+1); pending.push_back(id); }
            else require(old == depth+1,"inconsistent force-inline exception regions");
        };
        edge(order[body.begin].index,0);
        for (unsigned next = 0; next < pending.size(); ++next) {
            auto id = pending[next], depth = incoming.get(id)-1;
            auto range = blocks[id-1].instructions;
            for (unsigned n = range.begin; n < range.end(); ++n) {
                const auto& i = instructions[n];
                if ((i.opcode == Opcode::EhTry || i.opcode == Opcode::EhCleanup) && i.operands.count) {
                    auto handler = operands[i.operands.begin].ref;
                    edge(handler,depth+(i.opcode == Opcode::EhCleanup || cleanup.get(handler))); ++depth;
                } else if (i.opcode == Opcode::EhEnd) {
                    require(depth,"unbalanced force-inline exception region"); --depth;
                } else if (i.opcode == Opcode::Jump || i.opcode == Opcode::Branch || i.opcode == Opcode::Switch) {
                    for (unsigned k = i.operands.begin; k < i.operands.end(); ++k)
                        if (operands[k].kind == Operand::Label) edge(operands[k].ref,depth);
                } else if (i.opcode == Opcode::Return) { return_depths.put(n+1,depth+1); retirement += depth+1; }
            }
        }
        return retirement;
    }
    bool check(unsigned f, unsigned depth) {
        if (depth > 64 || active[f] || functions[f-1].declaration) return false;
        const auto& sig = p.signatures[functions[f-1].signature.index-1];
        if (sig.boundary.arity != CAM_FIXED) return false;
        if (!costs[f]) {
            // Reserve local work including parameter boundary copies, slots,
            // continuation plumbing and return-region retirement. Nested calls
            // request their own reservation; the parent has already reserved
            // enough to finish even if every nested request is declined.
            std::uint64_t cost = 5+8ull*sig.parameters.count+2ull*functions[f-1].slots.count;
            bool regions = false;
            auto body = functions[f-1].blocks;
            for (unsigned b = body.begin; b < body.end(); ++b) {
                auto range = blocks[order[b].index-1].instructions;
                for (unsigned n = range.begin; n < range.end(); ++n) {
                    const auto& i = instructions[n];
                    if (i.opcode == Opcode::StackAlloc || i.opcode == Opcode::VaStart || i.opcode == Opcode::VaArg) {
                        costs[f] = ~0u; return false;
                    }
                    cost += 1ull+i.operands.count;
                    regions |= i.opcode == Opcode::EhTry || i.opcode == Opcode::EhCleanup;
                    if (i.opcode == Opcode::Return) cost += 5;
                }
            }
            if (regions) cost += return_regions(f);
            costs[f] = cost <= 262144 ? unsigned(cost) : ~0u;
        }
        if (costs[f] > 262144-work || costs[f] > 4194304-total) return false;
        work += costs[f]; total += costs[f]; return true;
    }
    void body(unsigned f, Context& c, unsigned depth, const Operand* actuals = nullptr, unsigned actual_count = 0) {
        const auto source = functions[f-1];
        const auto sig = p.signatures[source.signature.index-1];
        active[f] = true;
        for (unsigned s = source.slots.begin; s < source.slots.end(); ++s) {
            auto old = slots[s];
            if (c.clone) { charge(1); c.slots.put(old.index,slot(p.slots[old.index-1].type).index); }
            else p.slot_order.push_back(old);
        }
        for (unsigned b = source.blocks.begin; b < source.blocks.end(); ++b) {
            auto old = order[b]; c.entries.put(old.index,c.straight ? current.index : block().index);
            if (c.clone) {
                auto range = blocks[old.index-1].instructions;
                for (unsigned n = range.begin; n < range.end(); ++n) {
                    auto dest = instructions[n].destination;
                    if (dest && !c.values.get(dest.index)) c.values.put(dest.index,value(p.values[dest.index-1].type).index);
                }
            }
        }
        if (c.clone) {
            require(actual_count == sig.parameters.count,"force-inline argument arity");
            for (unsigned j = 0; j < sig.parameters.count; ++j) {
                auto param = p.parameters[sig.parameters.begin+j];
                auto a = actuals[j];
                Type actual = a.kind == Operand::Temporary ? p.values[a.ref-1].type :
                    a.kind == Operand::Slot ? p.slots[a.ref-1].type : param.type;
                if (param.passing != PPM_DIRECT && (a.kind == Operand::Slot || actual != Type::Ptr)) {
                    if (a.kind != Operand::Slot) {
                        auto home = slot(actual); emit(Instruction(Opcode::Store,actual),{a,Operand::slot(home)}); a = Operand::slot(home);
                    }
                    Instruction address(Opcode::Addr); address.destination = value(Type::Ptr); emit(address,{a});
                    a = Operand::value(address.destination); actual = Type::Ptr;
                }
                // Scalar copies preserve boundary rounding/truncation without
                // manufacturing a stack home. Object values use typed storage.
                auto id = value(param.type);
                if (param.type.scalar()) {
                    Instruction copy(Opcode::Copy,param.type); copy.destination = id; emit(copy,{a});
                } else {
                    auto home = slot(param.type); emit(Instruction(Opcode::Store,param.type),{a,Operand::slot(home)});
                    Instruction load(Opcode::Load,param.type); load.destination = id; emit(load,{Operand::slot(home)});
                }
                c.values.put(param.value.index,id.index);
            }
            if (!c.straight) emit(Instruction(Opcode::Jump),{Operand::label(BlockId(c.entries.get(order[source.blocks.begin].index)))});
        }
        std::vector<Operand> args; // Reused scratch, never one allocation per instruction.
        for (unsigned b = source.blocks.begin; b < source.blocks.end(); ++b) {
            auto old = order[b]; if (!c.straight) start(BlockId(c.entries.get(old.index)));
            auto range = blocks[old.index-1].instructions;
            for (unsigned n = range.begin; n < range.end(); ++n) {
                auto i = instructions[n]; args.clear();
                for (unsigned k = i.operands.begin; k < i.operands.end(); ++k) args.push_back(mapped(operands[k],c));
                if (i.destination && c.clone) i.destination = ValueId(c.values.get(i.destination.index));
                auto callee = target(i);
                if (callee && !check(callee,depth+1)) { ++p.stats.inline_declined; callee = 0; }
                if (callee) {
                    ++p.stats.inline_calls;
                    Context nested; nested.clone = true;
                    auto range = functions[callee-1].blocks;
                    nested.straight = range.count == 1 && (i.type.scalar() || i.type == Type()) &&
                        instructions[blocks[order[range.begin].index-1].instructions.end()-1].opcode == Opcode::Return;
                    if (nested.straight) nested.straight_result = i.destination;
                    else {
                        nested.continuation = block();
                        if (i.destination) nested.result = slot(i.type);
                    }
                    body(callee,nested,depth+1,args.data()+1,args.size()-1);
                    if (!nested.straight) start(nested.continuation);
                    if (!nested.straight && i.destination) { Instruction load(Opcode::Load,i.type); load.destination = i.destination; load.debug = i.debug;
                        emit(load,{Operand::slot(nested.result)}); }
                } else if (c.clone && i.opcode == Opcode::Return) {
                    auto depth = return_depths.get(n+1);
                    for (unsigned k = 1; k < depth; ++k) { Instruction end(Opcode::EhEnd); end.debug = i.debug; emit(end,{}); }
                    if (c.straight) {
                        if (c.straight_result) { Instruction copy(Opcode::Copy,i.type); copy.destination = c.straight_result; copy.debug = i.debug; emit(copy,args); }
                        continue;
                    }
                    if (c.result) { Instruction store(Opcode::Store,i.type); store.debug = i.debug; emit(store,{args[0],Operand::slot(c.result)}); }
                    Instruction jump(Opcode::Jump); jump.debug = i.debug; emit(jump,{Operand::label(c.continuation)});
                } else {
                    if (i.opcode == Opcode::Phi) for (unsigned k = 0; k < i.operands.count; k += 2)
                        c.phi_edges.push_back({unsigned(p.operands.size()+k),operands[i.operands.begin+k].ref});
                    emit(i,args,c.clone);
                }
            }
            c.exits.put(old.index,current.index);
        }
        for (auto edge : c.phi_edges) p.operands[edge.first].ref = c.exits.get(edge.second);
        active[f] = false;
    }
public:
    explicit Expander(Program& program) : p(program), functions(p.functions.begin(),p.functions.end()), active(p.functions.size()+1), costs(p.functions.size()+1) {
        instructions.swap(p.instructions); operands.swap(p.operands); blocks.swap(p.blocks);
        order.swap(p.block_order); slots.swap(p.slot_order);
        // Canonical IDs remain semantic identities; local names are only an
        // adapter view. Regenerate them to avoid collisions with cloned names.
        for (auto& v : p.values) { v.name = 0; v.defined = false; v.definition = 0; }
        for (auto& s : p.slots) s.name = 0;
        for (const auto& f : functions) {
            auto sig = p.signatures[f.signature.index-1];
            for (unsigned n = sig.parameters.begin; n < sig.parameters.end(); ++n)
                if (auto id = p.parameters[n].value) p.values[id.index-1].defined = true;
        }
    }
    void run() {
        for (unsigned f = 1; f <= functions.size(); ++f) if (!functions[f-1].declaration) {
            owner = FunctionId(f); work = 0; Context c;
            auto block_begin = p.block_order.size(), slot_begin = p.slot_order.size();
            body(f,c,0);
            p.functions[f-1].blocks.begin = block_begin; p.functions[f-1].blocks.count = p.block_order.size()-block_begin;
            p.functions[f-1].slots.begin = slot_begin; p.functions[f-1].slots.count = p.slot_order.size()-slot_begin;
            p.stats.inline_max_function_work = std::max<std::uint64_t>(p.stats.inline_max_function_work,work);
        }
        p.stats.inline_budget_work = total;
        require(p.stats.inline_work <= total,"inline work exceeded its admission proof");
    }
};
}
void expand_forced_calls(Program& p)
{
    if (p.forced_calls_expanded) return;
    p.forced_calls_expanded = true;
    bool any = false;
    for (const auto& s : p.symbols) any |= s.metadata.force_inline && !s.metadata.no_inline;
    if (!any) return;
    bool call = false;
    for (const auto& i : p.instructions) if (i.opcode == Opcode::Call) {
        auto a = p.operands[i.operands.begin];
        if (a.kind != Operand::Symbol) continue;
        const auto& s = p.symbols[a.ref-1];
        call |= s.kind == Symbol::FunctionSymbol && s.metadata.force_inline && !s.metadata.no_inline &&
            !p.functions[s.entity-1].declaration;
    }
    if (call) Expander(p).run();
}
} // namespace lowir_model
