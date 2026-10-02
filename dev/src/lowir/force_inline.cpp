#include "lowir/inline_policy.h"
#include "lowir/folding.h"
#include "lowir/constant_objects.h"
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
    const InlinePolicy* policy;
    NameIndex names;
    unsigned serial = 0;
    std::uint64_t unit_limit, function_limit;
    Name fresh(const char* prefix) {
        for (;;) { auto name = p.intern(std::string(prefix)+std::to_string(++serial));
            if (names.insert(name,1)) return name; }
    }
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
    unsigned work = 0, growth = 0;
    std::uint64_t total = 0;
    std::uint64_t object_work = 0, proof_left = 0;
    ConstantObjects objects;
    std::vector<unsigned> definitions;
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
        Block b; b.owner = owner; if (policy) b.name = fresh("^inline"); p.blocks.push_back(b); return BlockId(p.blocks.size());
    }
    SlotId slot(Type type) {
        charge(1); Slot s; s.type = type; s.owner = owner; if (policy) s.name = fresh("$inline"); p.slots.push_back(s);
        SlotId id(p.slots.size()); p.slot_order.push_back(id); return id;
    }
    ValueId value(Type type, unsigned count = 1) {
        Value v; v.type = type; v.owner = owner; if (policy) v.name = fresh("%inline");
        p.values.push_back(v); definitions.push_back(count); return ValueId(p.values.size());
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
        return symbol.kind == Symbol::FunctionSymbol &&
            !(policy && policy->costly_callers[owner.index] && policy->costly[symbol.entity]) &&
            (policy ? policy->eligible[symbol.entity] : symbol.metadata.force_inline) && !symbol.metadata.no_inline &&
            !functions[symbol.entity-1].declaration ? symbol.entity : 0;
    }
    bool proof_charge(unsigned n = 1) {
        if (n > proof_left || n > function_limit-work || n > unit_limit-total) return false;
        proof_left -= n; work += n; total += n; p.stats.inline_context_work += n; return true;
    }
    Operand actual_constant(Operand a, unsigned depth = 0) {
        if (a.kind != Operand::Temporary || depth == 16 || !proof_charge()) return a;
        if (definitions[a.ref] != 1 || !p.values[a.ref-1].definition) return a;
        const auto& i = p.instructions[p.values[a.ref-1].definition-1];
        if (i.operands.count > 2 || !proof_charge(i.operands.count)) return a;
        Operand args[2];
        for (unsigned n = 0; n < i.operands.count; ++n)
            args[n] = actual_constant(p.operands[i.operands.begin+n],depth+1);
        Operand result;
        if (fold_integer(i,args,result) || fold_floating(p,i,args,result) || objects.fold(i,args,result)) return result;
        if ((i.opcode == Opcode::Addr || (i.opcode == Opcode::Copy && i.type == Type::Ptr)) &&
            args[0].kind == Operand::Symbol) return args[0];
        return a;
    }
    bool context_safe(unsigned fn, const Operand* actuals, unsigned count) {
        ++p.stats.inline_context_sites; proof_left = 4096;
        const auto& f = functions[fn-1]; const auto& sig = p.signatures[f.signature.index-1];
        if (count != sig.parameters.count) return false;
        cppgm::IdIndex known, visited;
        std::vector<Operand> facts(1), args;
        auto publish = [&](unsigned id, Operand a) {
            if (definitions[id] == 1 && (a.literal() || a.kind == Operand::Symbol)) {
                facts.push_back(a); known.put(id,facts.size()-1);
            }
        };
        for (unsigned n = 0; n < count; ++n) {
            if (!proof_charge()) return false;
            const auto& param = p.parameters[sig.parameters.begin+n];
            auto a = actual_constant(actuals[n]);
            // Parameter copies execute the declared boundary conversion.
            Instruction copy(Opcode::Copy,param.type); copy.operands.count = 1; Operand converted;
            if (param.passing != PPM_DIRECT) continue;
            if (fold_integer(copy,&a,converted) || fold_floating(p,copy,&a,converted)) publish(param.value.index,converted);
            else if (param.type == Type::Ptr && a.kind == Operand::Symbol) publish(param.value.index,a);
        }
        std::vector<unsigned> pending;
        auto edge = [&](unsigned b) { if (!visited.get(b)) { visited.put(b,1); pending.push_back(b); } };
        edge(order[f.blocks.begin].index);
        for (unsigned next = 0; next < pending.size(); ++next) {
            auto r = blocks[pending[next]-1].instructions;
            for (unsigned n = r.begin; n < r.end(); ++n) {
                const auto& i = instructions[n];
                if (!proof_charge(1+i.operands.count)) return false;
                args.clear();
                for (unsigned k = i.operands.begin; k < i.operands.end(); ++k) {
                    auto a = operands[k];
                    if (a.kind == Operand::Temporary) if (auto fact = known.get(a.ref)) a = facts[fact];
                    args.push_back(a);
                }
                if (i.opcode == Opcode::Throw || i.opcode == Opcode::Resume) return false;
                if (i.opcode == Opcode::Call) {
                    FunctionBoundaryMetadata boundary;
                    if (i.signature) boundary = p.signatures[i.signature.index-1].boundary;
                    else if (args[0].kind == Operand::Symbol) {
                        const auto& s = p.symbols[args[0].ref-1];
                        if (s.kind == Symbol::FunctionSymbol) boundary = p.signatures[functions[s.entity-1].signature.index-1].boundary;
                    }
                    if (boundary.unwind != CUM_NO) return false;
                }
                if (i.destination) {
                    Operand result;
                    if (fold_integer(i,args.data(),result) || fold_floating(p,i,args.data(),result) ||
                        objects.fold(i,args.data(),result)) publish(i.destination.index,result);
                    else if ((i.opcode == Opcode::Addr || (i.opcode == Opcode::Copy && i.type == Type::Ptr)) &&
                        args[0].kind == Operand::Symbol) publish(i.destination.index,args[0]);
                }
                if (i.opcode == Opcode::Branch && args[0].kind == Operand::Integer) {
                    edge(args[args[0].data.integer || args[0].integer_high() ? 1 : 2].ref);
                } else if (i.opcode == Opcode::Switch && args[0].kind == Operand::Integer) {
                    auto to = args[1];
                    for (unsigned k = 2; k < args.size(); k += 2)
                        if (same_scalar(args[0],args[k])) { to = args[k+1]; break; }
                    edge(to.ref);
                } else if (terminator(i.opcode)) for (auto a : args) if (a.kind == Operand::Label) edge(a.ref);
            }
        }
        return proof_left != 0;
    }
    std::uint64_t return_regions(unsigned f) {
        // A return retires all registrations belonging to the callee's frame.
        // A cloned return must perform those pops before joining its caller.
        // Analyze only demanded callee CFG edges, once per immutable body.
        cppgm::IdIndex incoming, cleanup;
        std::vector<unsigned> pending;
        auto body = functions[f-1].blocks;
        std::uint64_t retirement = 0;
        bool valid = true;
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
            else if (old != depth+1) valid = false;
        };
        edge(order[body.begin].index,0);
        for (unsigned next = 0; next < pending.size() && valid; ++next) {
            auto id = pending[next], depth = incoming.get(id)-1;
            auto range = blocks[id-1].instructions;
            for (unsigned n = range.begin; n < range.end(); ++n) {
                const auto& i = instructions[n];
                if ((i.opcode == Opcode::EhTry || i.opcode == Opcode::EhCleanup) && i.operands.count) {
                    auto handler = operands[i.operands.begin].ref;
                    edge(handler,depth+(i.opcode == Opcode::EhCleanup || cleanup.get(handler))); ++depth;
                } else if (i.opcode == Opcode::EhEnd) {
                    if (!depth) { valid = false; break; } --depth;
                } else if (i.opcode == Opcode::Jump || i.opcode == Opcode::Branch || i.opcode == Opcode::Switch) {
                    for (unsigned k = i.operands.begin; k < i.operands.end(); ++k)
                        if (operands[k].kind == Operand::Label) edge(operands[k].ref,depth);
                } else if (i.opcode == Opcode::Return) { return_depths.put(n+1,depth+1); retirement += depth+1; }
            }
        }
        return valid ? retirement : ~std::uint64_t(0);
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
            if (regions) {
                auto retirement = return_regions(f);
                if (retirement == ~std::uint64_t(0)) { costs[f] = ~0u; return false; }
                cost += retirement;
            }
            costs[f] = cost <= 262144 ? unsigned(cost) : ~0u;
        }
        if (costs[f] > function_limit-work || costs[f] > unit_limit-total) return false;
        if (policy) {
            unsigned added = policy->growth[f];
            if (added > 1 && added+growth > (policy->single[f] ? 2048u : 1536u)) return false;
            if (added > 1) growth += added;
        }
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
            if (policy && !c.clone) p.blocks[c.entries.get(old.index)-1].name = blocks[old.index-1].name;
            if (c.clone) {
                auto range = blocks[old.index-1].instructions;
                for (unsigned n = range.begin; n < range.end(); ++n) {
                    auto dest = instructions[n].destination;
                    if (dest && !c.values.get(dest.index)) c.values.put(dest.index,value(p.values[dest.index-1].type,definitions[dest.index]).index);
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
                auto id = value(param.type,definitions[param.value.index]);
                if (param.type.scalar()) {
                    Instruction copy(Opcode::Copy,param.type); copy.destination = id; emit(copy,{a});
                } else {
                    auto home = slot(param.type);
                    Instruction copy(Opcode::CopyObject); copy.bytes = param.type.bytes(); copy.alignment = param.type.alignment();
                    emit(copy,{a,Operand::slot(home)});
                    Instruction address(Opcode::Addr); address.destination = id; emit(address,{Operand::slot(home)});
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
                if (callee && policy && policy->contextual[callee] &&
                    !context_safe(callee,args.data()+1,args.size()-1)) { ++p.stats.inline_declined; callee = 0; }
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
                    if (!nested.straight && i.destination) { Instruction load(i.type.scalar() ? Opcode::Load : Opcode::Addr,i.type.scalar() ? i.type : Type()); load.destination = i.destination; load.debug = i.debug;
                        emit(load,{Operand::slot(nested.result)}); }
                } else if (c.clone && i.opcode == Opcode::Return) {
                    auto depth = return_depths.get(n+1);
                    for (unsigned k = 1; k < depth; ++k) { Instruction end(Opcode::EhEnd); end.debug = i.debug; emit(end,{}); }
                    if (c.straight) {
                        if (c.straight_result) { Instruction copy(Opcode::Copy,i.type); copy.destination = c.straight_result; emit(copy,args); }
                        continue;
                    }
                    if (c.result) {
                        Instruction store(i.type.scalar() ? Opcode::Store : Opcode::CopyObject,i.type.scalar() ? i.type : Type());
                        store.bytes = i.type.bytes(); store.alignment = i.type.alignment();
                        store.debug = i.debug; emit(store,{args[0],Operand::slot(c.result)});
                    }
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
    explicit Expander(Program& program, const InlinePolicy* admission = nullptr) : p(program), policy(admission), functions(p.functions.begin(),p.functions.end()), active(p.functions.size()+1), costs(p.functions.size()+1), objects(p,object_work), definitions(p.values.size()+1) {
        unit_limit = policy ? policy->unit_work : 4194304;
        function_limit = policy ? policy->function_work : 262144;
        for (const auto& a : p.parameters) ++definitions[a.value.index];
        for (const auto& i : p.instructions) if (i.destination) ++definitions[i.destination.index];
        if (policy) {
            for (auto v : p.values) if (v.name) names.insert(v.name,1);
            for (auto s : p.slots) if (s.name) names.insert(s.name,1);
            for (auto b : p.blocks) if (b.name) names.insert(b.name,1);
        }
        instructions.swap(p.instructions); operands.swap(p.operands); blocks.swap(p.blocks);
        order.swap(p.block_order); slots.swap(p.slot_order);
        // Canonical IDs remain semantic identities; local names are only an
        // adapter view. Regenerate them to avoid collisions with cloned names.
        for (auto& v : p.values) { if (!policy) v.name = 0; v.defined = false; v.definition = 0; }
        if (!policy) for (auto& s : p.slots) s.name = 0;
        for (const auto& f : functions) {
            auto sig = p.signatures[f.signature.index-1];
            for (unsigned n = sig.parameters.begin; n < sig.parameters.end(); ++n)
                if (auto id = p.parameters[n].value) p.values[id.index-1].defined = true;
        }
    }
    void run() {
        for (unsigned f = 1; f <= functions.size(); ++f) if (!functions[f-1].declaration) {
            owner = FunctionId(f); work = 0; growth = 0; Context c;
            auto block_begin = p.block_order.size(), slot_begin = p.slot_order.size();
            body(f,c,0);
            p.functions[f-1].blocks.begin = block_begin; p.functions[f-1].blocks.count = p.block_order.size()-block_begin;
            p.functions[f-1].slots.begin = slot_begin; p.functions[f-1].slots.count = p.slot_order.size()-slot_begin;
            p.stats.inline_max_function_work = std::max<std::uint64_t>(p.stats.inline_max_function_work,work);
        }
        p.stats.inline_budget_work += total;
        p.stats.inline_context_work += object_work;
        require(p.stats.inline_work <= p.stats.inline_budget_work,"inline work exceeded its admission proof");
    }
};
}
void expand_optional_calls(Program& p, const InlinePolicy& policy) { Expander(p,&policy).run(); }
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
