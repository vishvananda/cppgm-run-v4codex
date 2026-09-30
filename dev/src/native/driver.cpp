#include "native/encoding.h"
#include "native/selection.h"
#include <chrono>
namespace native {
Function builtin_tls(const lowir_model::Program&,const lowir_model::Function&);
Function builtin_strlen(const lowir_model::Program&,const lowir_model::Function&);
using Clock = std::chrono::steady_clock;
static double ms(Clock::time_point begin) { return std::chrono::duration<double,std::milli>(Clock::now()-begin).count(); }
void compile_image(const lowir_model::Program& p, Image& image, const std::vector<Instruction>& start, std::ostream* mir, Statistics& stats)
{
    Encoder encoder(image);
    Workspace workspace(p);
    auto time = Clock::now(); encode_data(p,image); encoder.startup(start); stats.encoding_ms += ms(time);
    if (mir) dump_header(p,start,*mir,image.defined[image.runtime_begin]);
    for (const auto& source : p.functions) if (!source.declaration) {
        time = Clock::now();
        Function f = Selector(p,source,workspace,stats,image.host).run();
        stats.selection_ms += ms(time);
        if (mir) dump_function(p,f,*mir);
        time = Clock::now(); encoder.encode(f); stats.encoding_ms += ms(time);
        // f and all selection temporaries die here, before the next function.
    }
    std::vector<bool> demanded(image.symbols.size());
    for (const auto& fix : image.code_fixups) demanded[fix.symbol] = true;
    for (const auto& fix : image.data_fixups) demanded[fix.symbol] = true;
    for (const auto& source : p.functions) if (source.declaration && demanded[source.symbol.index]) {
        const auto& metadata = p.symbols[source.symbol.index-1].metadata;
        if (image.host && metadata.builtin != lowir_model::SymbolMetadata::Builtin::None) continue;
        if (!metadata.tls_for && metadata.builtin != lowir_model::SymbolMetadata::Builtin::Strlen) continue;
        time = Clock::now();
        Function f = metadata.tls_for ? builtin_tls(p,source) : builtin_strlen(p,source);
        stats.selection_ms += ms(time);
        if (mir) dump_function(p,f,*mir);
        time = Clock::now(); encoder.encode(f); stats.encoding_ms += ms(time); ++stats.functions; stats.instructions += f.instructions.size();
    }
    stats.text_bytes = image.code.size();
}
void compile(const lowir_model::Program& p, const std::string& output, std::ostream* mir, Statistics& stats)
{
    auto start = startup(p);
    lowir_model::require(output.empty() || !start.empty(), "executable requires an entry function");
    Image image(p.symbols.size());
    compile_image(p,image,start,mir,stats);
    if (!output.empty()) {
        auto time = Clock::now(); write_executable(image,output); stats.encoding_ms += ms(time);
    }
}
} // namespace native
