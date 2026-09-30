#include "toolchain/runtime.h"
namespace cppgm { namespace toolchain {
using namespace lowir_model;
bool runtime_role(SymbolRole role)
{
    return role == SR_EH_FREE_EXCEPTION || role == SR_BAD_CAST || role == SR_BAD_TYPEID || role == SR_MALLOC || role == SR_ALLOCATE_MEMORY || role == SR_FREE_MEMORY || role == SR_TERMINATE ||
        role == SR_PURE_VIRTUAL || role == SR_DYNAMIC_CAST ||
        (role >= SR_RTTI_CLASS && role <= SR_RTTI_DATA) ||
        (role >= SR_EH_ALLOCATE_EXCEPTION && role <= SR_EH_THROW);
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
    bool exceptions = false;
    for (const auto& request : requests) {
        if (request.role == SR_EH_FREE_EXCEPTION || (request.role >= SR_EH_ALLOCATE_EXCEPTION && request.role <= SR_EH_THROW) || request.role == SR_BAD_CAST || request.role == SR_BAD_TYPEID) { exceptions = true; continue; }
        if (request.role == SR_ALLOCATE_MEMORY) exceptions = true;
        auto symbol = r.symbol(request.name,request.role);
        if (request.role >= SR_RTTI_CLASS) {
            r.table(symbol);
            if (request.role <= SR_RTTI_VMI) r.tables[request.role-SR_RTTI_CLASS] = symbol;
        } else if (request.role == SR_DYNAMIC_CAST) cast = symbol;
        else leaves.push_back({symbol,request.role});
    }
    if (exceptions) {
        const char* tables[] = {"_ZTVN10__cxxabiv117__class_type_infoE", "_ZTVN10__cxxabiv120__si_class_type_infoE",
            "_ZTVN10__cxxabiv121__vmi_class_type_infoE", "_ZTVN10__cxxabiv119__pointer_type_infoE", "_ZTVN10__cxxabiv120__function_type_infoE"};
        for (unsigned i = 0; i < 5; ++i) if (!r.tables[i]) {
            // The pointer table may already be a requested RTTI data symbol.
            for (unsigned s = 0; s < r.p.symbols.size(); ++s)
                if (r.p.symbols[s].metadata.object && r.p.name(r.p.symbols[s].metadata.object) == tables[i]) r.tables[i] = SymbolId(s+1);
            if (!r.tables[i]) { r.tables[i] = r.symbol(tables[i],SR_NONE); r.table(r.tables[i]); }
        }
        build_exceptions(r,requests);
    }
    if (cast) {
        for (unsigned i = 1; i < 3; ++i) if (!r.tables[i]) { r.tables[i] = r.symbol("",SR_NONE); r.table(r.tables[i]); }
        build_dynamic_cast(r,cast);
    }
    for (const auto& primitive : r.primitives) leaves.push_back({primitive.symbol,primitive.role});
    // The finite runtime program uses the ordinary typed backend. Primitive
    // process bodies are MIR because their Linux syscall boundary is native.
    auto obj = compile_object(r.p,stats);
    // Runtime state references are explicit adapter identities, not names.
    for (auto* fixes : {&obj.image.code_fixups,&obj.image.data_fixups}) for (auto& fix : *fixes)
        for (unsigned i = 0; i < unsigned(native::RuntimeEntity::Count); ++i)
            if (r.states[i] && fix.symbol == r.states[i].index) { fix.symbol = obj.image.runtime_begin+i; break; }
    for (unsigned i = 0; i < unsigned(native::RuntimeEntity::Count); ++i) if (r.states[i]) {
        auto id=obj.image.runtime_begin+i;
        if (obj.image.defined[id]) continue;
        obj.image.data.resize((obj.image.data.size()+15)&~std::size_t(15),0);
        obj.image.symbols[id]=obj.image.data.size();obj.image.data.resize(obj.image.data.size()+16,0);
        obj.image.defined[id]=obj.image.data_symbols[id]=true;obj.symbols[id].definition=id;
    }
    native::Encoder encoder(obj.image);
    for (const auto& leaf : leaves) {
        auto f = native::process_runtime(leaf.symbol,leaf.role,leaf.role == SR_ALLOCATE_MEMORY && r.p.symbols[leaf.symbol.index-1].metadata.binding != SBM_INTERNAL ? r.allocation_failure : SymbolId()); encoder.encode(f);
        ++stats.functions; stats.instructions += f.instructions.size();
        obj.symbols[leaf.symbol.index].definition = leaf.symbol.index;
    }
    stats.text_bytes = obj.image.code.size(); return obj;
}
} }
