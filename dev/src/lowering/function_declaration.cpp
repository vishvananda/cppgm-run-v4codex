#include "lowering/procedural.h"
#include <stdexcept>
namespace cppgm { namespace lowering {
using namespace lowir_model;
using namespace ir_model;
void Procedural::declare_function(EntityId e)
{
    auto entity = sem.entities[e];
    bool defined = entity.body || sem.closure_adapter(e).function || ((sem.constructor_member(e) || sem.destructor_member(e) || sem.transfer_member(e)) && sem.synthetic_member(e));
    Function f; f.symbol = symbol(e); f.declaration = !defined;
    auto& existing = p.symbols[f.symbol.index-1];
    if (existing.kind == Symbol::FunctionSymbol) {
        auto& prior = p.functions[existing.entity-1];
        if (!entity.body) {
            // A later TU can establish the value ABI of an earlier opaque
            // declaration. Refresh that function identity before any call
            // consumes its signature; unrelated declarations stay warm.
            if (prior.declaration && linkage.incomplete_signatures.get(existing.entity))
                prior.signature = signature(sem.call_type(e),FunctionId(existing.entity));
            return;
        }
        if (!prior.declaration) {
            if (entity.inline_function) return;
            throw std::runtime_error("multiple function definitions");
        }
        prior.declaration = false;
        prior.signature = signature(sem.call_type(e), FunctionId(existing.entity));
        definitions.push_back(e);
        return;
    }
    FunctionId id(p.functions.size()+1);
    f.signature = signature(sem.call_type(e), id);
    if (entity.stable_prefix) p.signatures[f.signature.index-1].boundary.query = CQM_STABLE_PREFIX;
    if (sem.function_nonthrowing(e)) p.signatures[f.signature.index-1].boundary.unwind = CUM_NO;
    if (entity.member_info && !entity.is_static)
        p.parameters[p.signatures[f.signature.index-1].parameters.begin + sem.indirect_value(sem.types[entity.type].child)].object_bytes = sem.object_size(sem.entities[sem.scopes[entity.owner].entity].type);
    if (sem.constructor_member(e) && sem.transfer_member(e))
        for (unsigned j = 0; j < 2; ++j) p.parameters[p.signatures[f.signature.index-1].parameters.begin+j].alias = PALM_NOALIAS;
    if (entity.builtin != semantic::Entity::NoBuiltin) {
        auto& sig = p.signatures[f.signature.index-1]; sig.boundary.unwind = CUM_NO;
        if (entity.builtin == semantic::Entity::Strlen) sig.boundary.effects = CFXM_READONLY;
        if (entity.builtin == semantic::Entity::Memcpy)
            for (unsigned j = 0; j < 2; ++j) p.parameters[sig.parameters.begin+j].alias = PALM_NOALIAS;
    }
    p.functions.push_back(f);
    auto& sym = p.symbols[f.symbol.index-1]; sym.kind = Symbol::FunctionSymbol; sym.entity = id.index;
    if (defined) definitions.push_back(e);
}
} }
