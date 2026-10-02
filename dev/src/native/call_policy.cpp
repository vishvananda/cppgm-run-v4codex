#include "native/selection.h"
namespace native {
bool Selector::dynamic_copy(const lowir_model::Instruction& i, lowir_model::SignatureId id) const
{
    using namespace lowir_model;
    // Existing parallel ABI setup has already captured all three inputs. REP
    // replaces only the no-result memcpy contract, never memmove or a spelling
    // match to an ordinary function. The call's conservative clobbers remain.
    if (i.type != Type::Ptr || i.operands.count != 4 ||
        (i.destination && state(i.destination.index).uses) || arg(i,0).kind != lowir_model::Operand::Symbol) return false;
    const auto& symbol = p.symbols[arg(i,0).ref-1];
    if (symbol.kind != lowir_model::Symbol::FunctionSymbol || symbol.metadata.builtin != SymbolMetadata::Builtin::Memcpy ||
        symbol.metadata.role != SR_NONE || symbol.metadata.tls_for || p.functions[symbol.entity-1].signature.index != id.index) return false;
    const auto& s = p.signatures[id.index-1];
    if (s.result != Type::Ptr || s.parameters.count != 3 || s.boundary.arity != CAM_FIXED ||
        s.boundary.unwind != CUM_NO || s.boundary.effects != CFXM_DEFAULT ||
        s.boundary.returns != CRM_DEFAULT || s.boundary.query != CQM_DEFAULT) return false;
    for (unsigned n = 0; n < 3; ++n) {
        const auto& a = p.parameters[s.parameters.begin+n];
        if (a.type != (n == 2 ? Type(Type::I64) : Type(Type::Ptr)) || a.passing != PPM_DIRECT ||
            (n < 2 && a.alias != PALM_NOALIAS)) return false;
    }
    return true;
}
} // namespace native
