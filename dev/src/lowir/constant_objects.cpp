#include "lowir/constant_objects.h"
namespace lowir_model {
ConstantObjects::ConstantObjects(const Program& program, std::uint64_t& work)
    : p(program), lengths(p.symbols.size()+1,~std::uint64_t(0))
{
    for (const auto& g : p.globals) {
        ++work;
        const auto& m = p.symbols[g.symbol.index-1].metadata;
        if (g.declaration || m.storage != GSM_READONLY || m.object ||
            (m.binding != SBM_INTERNAL && m.binding != SBM_STRONG)) continue;
        std::uint64_t offset = 0;
        for (unsigned n = g.data.begin; n < g.data.end(); ++n) {
            const auto& d = p.data[n]; ++work;
            if (d.kind == DataItem::Zero) {
                if (d.zero_bytes) { lengths[g.symbol.index] = offset; break; }
            } else if (d.kind == DataItem::Scalar && d.type.integer() && d.value.kind == Operand::Integer) {
                bool found = false;
                for (unsigned b = 0; b < d.type.bytes(); ++b) {
                    ++work;
                    auto bits = b < 8 ? d.value.data.integer : d.value.integer_high();
                    if (!((bits >> (8*(b%8)))&255)) { lengths[g.symbol.index] = offset+b; found = true; break; }
                }
                if (found) break;
                offset += d.type.bytes();
            } else break;
        }
    }
}
bool ConstantObjects::fold(const Instruction& i, const Operand* a, Operand& result) const
{
    if (i.opcode != Opcode::Call || i.operands.count != 2 || a[0].kind != Operand::Symbol ||
        a[1].kind != Operand::Symbol || i.type != Type::I64) return false;
    const auto& symbol = p.symbols[a[0].ref-1];
    if (symbol.kind != Symbol::FunctionSymbol || symbol.metadata.builtin != SymbolMetadata::Builtin::Strlen ||
        symbol.metadata.role != SR_NONE || symbol.metadata.tls_for) return false;
    const auto& f = p.functions[symbol.entity-1];
    const auto& s = p.signatures[f.signature.index-1];
    if (i.signature && i.signature.index != f.signature.index) return false;
    if (!f.declaration || s.result != Type::I64 || s.parameters.count != 1 ||
        s.boundary.arity != CAM_FIXED || s.boundary.unwind != CUM_NO ||
        s.boundary.effects != CFXM_READONLY || s.boundary.returns != CRM_DEFAULT || s.boundary.query != CQM_DEFAULT) return false;
    const auto& param = p.parameters[s.parameters.begin];
    if (param.type != Type::Ptr || param.passing != PPM_DIRECT) return false;
    auto length = lengths[a[1].ref];
    if (length == ~std::uint64_t(0)) return false;
    result = Operand::integer(length); return true;
}
}
