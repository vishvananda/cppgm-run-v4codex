#include "lowir/folding.h"
namespace lowir_model {
void propagate_call_constants(Program& p, std::uint64_t& work)
{
    struct Fact { unsigned state = 0; Operand value; };
    std::vector<Fact> facts(p.values.size()+1);
    std::vector<bool> eligible(p.functions.size()+1);
    for (unsigned fn = 1; fn <= p.functions.size(); ++fn) {
        const auto& f = p.functions[fn-1]; const auto& s = p.symbols[f.symbol.index-1];
        eligible[fn] = !f.declaration && s.metadata.binding == SBM_INTERNAL && !s.metadata.object_root &&
            !s.metadata.keep_alias && s.metadata.role == SR_NONE &&
            p.signatures[f.signature.index-1].boundary.arity == CAM_FIXED;
    }
    auto escape = [&](unsigned id) { const auto& s = p.symbols[id-1];
        if (s.kind == Symbol::FunctionSymbol) eligible[s.entity] = false; };
    for (auto a : p.aliases) escape(a.target.index);
    for (auto d : p.data) if (d.kind == DataItem::Address) escape(d.symbol.index);
    for (const auto& i : p.instructions) {
        ++work;
        if (i.destination) facts[i.destination.index].state = 2; // Includes mutable parameters.
        for (unsigned n = i.operands.begin; n < i.operands.end(); ++n) {
            auto a = p.operands[n]; ++work;
            if (a.kind == Operand::Symbol && !(i.opcode == Opcode::Call && n == i.operands.begin)) escape(a.ref);
        }
        if (i.opcode != Opcode::Call) continue;
        auto target = p.operands[i.operands.begin]; if (target.kind != Operand::Symbol) continue;
        const auto& symbol = p.symbols[target.ref-1];
        if (symbol.kind != Symbol::FunctionSymbol || !eligible[symbol.entity]) continue;
        auto parameters = p.signatures[p.functions[symbol.entity-1].signature.index-1].parameters;
        for (unsigned n = 0; n < parameters.count; ++n) {
            const auto& parameter = p.parameters[parameters.begin+n];
            auto& fact = facts[parameter.value.index]; auto a = p.operands[i.operands.begin+n+1];
            bool constant = parameter.passing == PPM_DIRECT &&
                ((parameter.type.integer() && a.kind == Operand::Integer) ||
                 (parameter.type == Type::Ptr && (a.kind == Operand::Null || a.kind == Operand::Symbol)));
            if (constant && parameter.type.integer()) a = normalize_integer(a,parameter.type);
            if (!constant || (fact.state == 1 && !same_scalar(fact.value,a))) fact.state = 2;
            if (!fact.state) { fact.state = 1; fact.value = a; }
            ++work;
        }
    }
    // Decisions use the completed escape/mutation census, never a prefix of it.
    for (const auto& f : p.functions) {
        auto fn = p.symbols[f.symbol.index-1].entity; if (!eligible[fn]) continue;
        for (unsigned b = f.blocks.begin; b < f.blocks.end(); ++b) {
            auto r = p.blocks[p.block_order[b].index-1].instructions;
            for (unsigned n = r.begin; n < r.end(); ++n) {
                const auto& i = p.instructions[n];
                for (unsigned k = i.operands.begin; k < i.operands.end(); ++k) {
                    auto& a = p.operands[k]; ++work;
                    if (a.kind == Operand::Temporary && facts[a.ref].state == 1) a = facts[a.ref].value;
                }
            }
        }
    }
}
}
