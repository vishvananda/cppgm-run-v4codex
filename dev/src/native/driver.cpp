#include "native/encoding.h"
#include "native/selection.h"
#include <chrono>
namespace native {
Function builtin_strlen(const lowir_model::Program&,const lowir_model::Function&);
using Clock = std::chrono::steady_clock;
static double ms(Clock::time_point begin) { return std::chrono::duration<double,std::milli>(Clock::now()-begin).count(); }
void compile(const lowir_model::Program& p, const std::string& output, std::ostream* mir, Statistics& stats)
{
    auto start = startup(p);
    lowir_model::require(output.empty() || !start.empty(), "executable requires an entry function");
    Image image(p.symbols.size());
    Encoder encoder(image);
    Workspace workspace(p);
    if (mir) dump_header(p,start,*mir);
    auto time = Clock::now(); encoder.startup(start); encode_data(p,image); stats.encoding_ms += ms(time);
    for (const auto& source : p.functions) if (!source.declaration) {
        time = Clock::now();
        Function f = Selector(p,source,workspace,stats).run();
        stats.selection_ms += ms(time);
        if (mir) dump_function(p,f,*mir);
        time = Clock::now(); encoder.encode(f); stats.encoding_ms += ms(time);
        // f and all selection temporaries die here, before the next function.
    }
    std::vector<bool> demanded(p.symbols.size()+1);
    for (const auto& fix : image.code_fixups) demanded[fix.symbol] = true;
    for (const auto& fix : image.data_fixups) demanded[fix.symbol] = true;
    for (const auto& source : p.functions) if (source.declaration && demanded[source.symbol.index]) {
        const auto& metadata = p.symbols[source.symbol.index-1].metadata;
        if (p.name(metadata.object) != "cppgm_builtin_strlen") continue;
        time = Clock::now();
        Function f = builtin_strlen(p,source);
        stats.selection_ms += ms(time);
        if (mir) dump_function(p,f,*mir);
        time = Clock::now(); encoder.encode(f); stats.encoding_ms += ms(time); ++stats.functions; stats.instructions += f.instructions.size();
    }
    stats.text_bytes = image.code.size();
    if (!output.empty()) {
        time = Clock::now(); write_executable(image,output); stats.encoding_ms += ms(time);
    }
}
} // namespace native
