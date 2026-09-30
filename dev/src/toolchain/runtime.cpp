#include "toolchain/runtime.h"
namespace cppgm { namespace toolchain {
using namespace lowir_model;
bool runtime_role(SymbolRole role)
{
    return role == SR_ALLOCATE_MEMORY || role == SR_FREE_MEMORY || role == SR_TERMINATE ||
        role == SR_PURE_VIRTUAL || role == SR_DYNAMIC_CAST ||
        (role >= SR_RTTI_CLASS && role <= SR_RTTI_VMI);
}
void Linker::supply_runtime()
{
    std::vector<RuntimeRequest> requests;
    for (unsigned i : runtime_demands_) {
        const auto& s = symbols_[i];
        auto name = names_.spelling(s.name);
        requests.push_back({std::string(name.data,name.size),s.role});
    }
    if (!requests.empty()) {
        auto obj = runtime_object(requests,runtime_stats);
        // All support definitions are demanded by this finite construction;
        // it has no weak alternatives or lazy GOT dependency graph to revisit.
        for (unsigned i = 1; i < obj.symbols.size(); ++i)
            if (obj.symbols[i].definition == i) ++definition_work;
        relocation_work += obj.image.code_fixups.size()+obj.image.data_fixups.size();
        add(std::move(obj));
    }
}
Object runtime_object(const std::vector<RuntimeRequest>& requests, native::Statistics& stats)
{
    RuntimeProgram r;
    struct Leaf { SymbolId symbol; SymbolRole role; };
    std::vector<Leaf> leaves;
    SymbolId cast;
    for (const auto& request : requests) {
        auto symbol = r.symbol(request.name,request.role);
        if (request.role >= SR_RTTI_CLASS) {
            r.table(symbol);
            if (request.role <= SR_RTTI_VMI) r.tables[request.role-SR_RTTI_CLASS] = symbol;
        } else if (request.role == SR_DYNAMIC_CAST) cast = symbol;
        else leaves.push_back({symbol,request.role});
    }
    if (cast) {
        for (unsigned i = 1; i < 3; ++i) if (!r.tables[i]) { r.tables[i] = r.symbol("",SR_NONE); r.table(r.tables[i]); }
        build_dynamic_cast(r,cast);
    }
    // The finite runtime program uses the ordinary typed backend. Primitive
    // process bodies are MIR because their Linux syscall boundary is native.
    auto obj = compile_object(r.p,stats);
    native::Encoder encoder(obj.image);
    for (const auto& leaf : leaves) {
        auto f = native::process_runtime(leaf.symbol,leaf.role); encoder.encode(f);
        ++stats.functions; stats.instructions += f.instructions.size();
        obj.symbols[leaf.symbol.index].definition = leaf.symbol.index;
    }
    stats.text_bytes = obj.image.code.size(); return obj;
}
} }
