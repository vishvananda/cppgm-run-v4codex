#include "native/encoding.h"
#include "native/selection.h"
#include <algorithm>
#include <chrono>
namespace native {
Function builtin_tls(const lowir_model::Program&,const lowir_model::Function&);
Function builtin_strlen(const lowir_model::Program&,const lowir_model::Function&);
Function builtin_fill(const lowir_model::Program&,const lowir_model::Function&);
std::vector<bool> object_demand(const lowir_model::Program&);
using Clock = std::chrono::steady_clock;
static double ms(Clock::time_point begin) { return std::chrono::duration<double,std::milli>(Clock::now()-begin).count(); }
void compile_image(lowir_model::Program& p, Image& image, const std::vector<Instruction>& start, std::ostream* mir, Statistics& stats, unsigned level)
{
    // Both source and explicit LowIR enter object preparation here. The source
    // view retains original bodies/attributes, so a text adapter cannot spend
    // a fresh budget on calls already expanded by source lowering.
    auto preparation = Clock::now();
    lowir_model::expand_forced_calls(p);
    stats.preparation_ms += ms(preparation);
    stats.inline_calls += p.stats.inline_calls;
    stats.inline_work += p.stats.inline_work;
    stats.inline_declined += p.stats.inline_declined;
    stats.inline_budget_work += p.stats.inline_budget_work;
    stats.inline_max_function_work = std::max(stats.inline_max_function_work,p.stats.inline_max_function_work);
    stats.prepared_instructions += p.instructions.size();
    stats.prepared_operands += p.operands.size();
    Encoder encoder(image);
    Workspace workspace(p);
    if (image.host) {
        image.indirect_addresses.resize(image.symbols.size());
        for (const auto& f : p.functions)
            image.indirect_addresses[f.symbol.index] = p.symbols[f.symbol.index-1].metadata.binding != ir_model::SBM_INTERNAL;
        for (const auto& g : p.globals)
            image.indirect_addresses[g.symbol.index] = (g.declaration || p.symbols[g.symbol.index-1].metadata.binding == ir_model::SBM_WEAK) &&
                p.symbols[g.symbol.index-1].metadata.storage != ir_model::GSM_THREAD_LOCAL;
    }
    auto live = image.host ? object_demand(p) : std::vector<bool>();
    auto time = Clock::now(); encode_data(p,image,image.host ? &live : nullptr); encoder.startup(start); stats.encoding_ms += ms(time);
    if (mir) dump_header(p,start,*mir,image.defined[image.runtime_begin]);
    for (unsigned ordinal = 0; ordinal < p.functions.size(); ++ordinal) {
        const auto& source = p.functions[p.function_order.empty() ? ordinal : p.function_order[ordinal].index-1];
        if (source.declaration) continue;
        if (image.host && !live[source.symbol.index]) continue;
        time = Clock::now();
        Function f = Selector(p,source,workspace,stats,image.host,level).run();
        stats.selection_ms += ms(time);
        if (mir) dump_function(p,f,*mir);
        time = Clock::now(); encoder.encode(f); stats.encoding_ms += ms(time);
        // f and all selection temporaries die here, before the next function.
    }
    std::vector<bool> demanded(image.symbols.size());
    for (const auto& fix : image.code_fixups) demanded[fix.symbol] = true;
    for (const auto& fix : image.data_fixups) demanded[fix.symbol] = true;
    for (const auto& fix : image.tls_fixups) demanded[fix.symbol] = true;
    for (const auto& source : p.functions) if (source.declaration && demanded[source.symbol.index]) {
        const auto& metadata = p.symbols[source.symbol.index-1].metadata;
        using Builtin = lowir_model::SymbolMetadata::Builtin;
        if (metadata.builtin == Builtin::Strlen) {
            const auto& sig = p.signatures[source.signature.index-1];
            if (image.host || sig.result != Type::I64 || sig.parameters.count != 1 ||
                sig.boundary.arity != ir_model::CAM_FIXED ||
                p.parameters[sig.parameters.begin].type != Type::Ptr ||
                p.parameters[sig.parameters.begin].passing != ir_model::PPM_DIRECT) continue;
        }
        if (!metadata.tls_for && metadata.builtin != Builtin::Strlen && metadata.builtin != Builtin::FillBytes) continue;
        time = Clock::now();
        Function f = metadata.tls_for ? builtin_tls(p,source) : metadata.builtin == Builtin::Strlen ?
            builtin_strlen(p,source) : builtin_fill(p,source);
        stats.selection_ms += ms(time);
        if (mir) dump_function(p,f,*mir);
        time = Clock::now(); encoder.encode(f); stats.encoding_ms += ms(time); ++stats.functions; stats.instructions += f.instructions.size();
    }
    stats.text_bytes = image.code.size();
}
void compile(lowir_model::Program& p, const std::string& output, std::ostream* mir, Statistics& stats, unsigned level)
{
    native::legalize_extended_floats(p,stats);
    auto start = startup(p);
    lowir_model::require(output.empty() || !start.empty(), "executable requires an entry function");
    Image image(p.symbols.size());
    compile_image(p,image,start,mir,stats,level);
    if (!output.empty()) {
        auto time = Clock::now(); write_executable(image,output); stats.encoding_ms += ms(time);
    }
}
} // namespace native
