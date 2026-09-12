#include "lowir/validator.h"
#include "lowir/metadata.h"
#include <algorithm>
#include <utility>
namespace lowir_model {
void Validator::symbols() const
{
    if (!p_.function_order.empty()) {
        require(p_.function_order.size() == p_.functions.size(), "incomplete function schedule");
        std::vector<bool> seen(p_.functions.size());
        for (FunctionId f : p_.function_order) {
            require(f.index && f.index <= seen.size() && !seen[f.index-1], "invalid function schedule");
            seen[f.index-1] = true;
        }
    }
    std::uint32_t roles[SR_RTTI_DATA+1] = {};
    std::vector<bool> tls(p_.symbols.size());
    for (const Symbol& s : p_.symbols) {
        require(s.kind != Symbol::Unknown, "undefined top-level symbol");
        const SymbolMetadata& m = s.metadata;
        if (m.role != SR_NONE) {
            require((s.kind == Symbol::FunctionSymbol) == (m.role < SR_RTTI_CLASS), "role on wrong entity kind");
            if (m.role != SR_RTTI_DATA) require(++roles[m.role] == 1, "duplicate singleton role");
        }
        if (m.tls_for) {
            require(s.kind == Symbol::FunctionSymbol, "TLS wrapper must be function");
            const Symbol& target = p_.symbols.at(m.tls_for.index-1);
            require(target.kind == Symbol::GlobalSymbol && target.metadata.storage == GSM_THREAD_LOCAL, "invalid TLS target");
            require(!tls[m.tls_for.index-1], "duplicate TLS wrapper");
            tls[m.tls_for.index-1] = true;
        }
    }
    NameIndex aliases;
    for (const ObjectAlias& a : p_.aliases) {
        require(aliases.insert(a.name, 1), "duplicate object alias");
        require(p_.symbols.at(a.target.index-1).kind != Symbol::Unknown, "undefined object alias target");
    }
    for (const DataItem& d : p_.data) {
        if (d.kind == DataItem::Scalar) validate_literal(d.value, d.type);
        if (d.kind == DataItem::Address) require(d.type == Type::Ptr && p_.symbols.at(d.symbol.index-1).kind != Symbol::Unknown, "invalid address initializer");
    }
    for (const Function& f : p_.functions) {
        validate_signature(p_, p_.signatures.at(f.signature.index-1), false);
        require(f.declaration || f.blocks.count, "function has no blocks");
        if (f.debug.file) require(f.debug.line && f.debug.column && !f.declaration, "invalid function debug location");
    }
    for (const Block& b : p_.blocks) require(b.defined, "undefined block target");
}
void Validator::run()
{
    symbols();
    // Ordinary CFG edges exclude EH-handler registration. Sort/deduplicate
    // once: switch arms and branches can share the same predecessor edge.
    using Edge = std::pair<std::uint32_t, std::uint32_t>;
    std::vector<Edge> edges;
    // Handler membership is a CFG fact, independent of ordinary predecessors
    // and source order. Recompute it for each validation so model edits cannot
    // leave a stale permission to place a phi in a handler.
    std::vector<bool> handlers(p_.blocks.size());
    for (std::uint32_t n = 0; n < p_.blocks.size(); ++n) {
        const Block& b = p_.blocks[n];
        require(b.instructions.count && b.instructions.end() <= p_.instructions.size(), "empty or invalid block");
        for (unsigned k = b.instructions.begin; k < b.instructions.end(); ++k) {
            const Instruction& i = p_.instructions[k];
            if (i.opcode != Opcode::EhTry && i.opcode != Opcode::EhCleanup) continue;
            validate_instruction_shape(i);
            require(i.operands.end() <= p_.operands.size(), "invalid handler operand range");
            if (!i.operands.count) continue; // cleanup clause without a target
            const Operand& a = p_.operands[i.operands.begin];
            require(a.kind == Operand::Label && a.ref && a.ref <= p_.blocks.size(), "invalid handler target");
            const Block& target = p_.blocks[a.ref-1];
            require(target.defined && target.owner == b.owner, "foreign handler target");
            handlers[a.ref-1] = true;
        }
        const Instruction& t = p_.instructions[b.instructions.end()-1];
        require(terminator(t.opcode), "missing terminator");
        if (t.opcode == Opcode::Jump || t.opcode == Opcode::Branch || t.opcode == Opcode::Switch) {
            for (unsigned j = t.operands.begin; j < t.operands.end(); ++j) {
                const Operand& a = p_.operands.at(j);
                if (a.kind == Operand::Label) {
                    const Block& target = p_.blocks.at(a.ref-1);
                    require(target.defined && target.owner == b.owner, "invalid control flow target");
                    edges.push_back(Edge(a.ref-1, n));
                }
            }
        }
    }
    std::sort(edges.begin(), edges.end());
    edges.erase(std::unique(edges.begin(), edges.end()), edges.end());
    p_.stats.cfg_edges = edges.size();
    std::vector<unsigned> offsets(p_.blocks.size()+1, 0), seen(p_.blocks.size(), 0), predecessor(p_.blocks.size(), 0);
    for (const Edge& e : edges) ++offsets[e.first+1];
    for (unsigned n = 1; n < offsets.size(); ++n) offsets[n] += offsets[n-1];
    unsigned stamp = 0;
    for (std::uint32_t n = 0; n < p_.blocks.size(); ++n) {
        const Block& b = p_.blocks[n];
        function_ = b.owner;
        bool ordinary = false;
        for (unsigned k = offsets[n]; k < offsets[n+1]; ++k) predecessor[edges[k].second] = n+1;
        for (unsigned k = b.instructions.begin; k < b.instructions.end(); ++k) {
            const Instruction& i = p_.instructions[k];
            ordinal_ = k+1;
            phi_ = i.opcode == Opcode::Phi;
            if (phi_) {
                require(!handlers[n], "phi in exception handler block");
                require(!ordinary, "phi after ordinary instruction");
                require(i.operands.count / 2 == offsets[n+1]-offsets[n], "phi predecessor count mismatch");
                ++stamp;
                for (unsigned j = 0; j < i.operands.count; j += 2) {
                    const Operand& a = p_.operands.at(i.operands.begin+j);
                    require(a.kind == Operand::Label && a.ref && a.ref <= p_.blocks.size(), "invalid phi predecessor");
                    require(predecessor[a.ref-1] == n+1 && seen[a.ref-1] != stamp, "invalid or repeated phi predecessor");
                    seen[a.ref-1] = stamp;
                }
            } else ordinary = true;
            require(k+1 == b.instructions.end() || !terminator(i.opcode), "instruction after terminator");
            instruction(i);
            ++p_.stats.validated_instructions;
        }
    }
}
void validate(Program& p) { p.stats.validated_instructions = 0; Validator(p).run(); }
} // namespace lowir_model
