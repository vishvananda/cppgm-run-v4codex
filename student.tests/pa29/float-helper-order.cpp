// Inspect the same typed schedule that native emission consumes. Every helper
// introduced by binary16/binary128 legalization must have an emission entry.
#include "lowir/reader.h"
#include "lowir/validator.h"
#include "native/model.h"
#include "toolchain/object.h"
#include <stdexcept>
int main(int argc, char** argv) {
    if (argc != 3) return 2;
    auto p = lowir_model::parse_lowir_program_files({argv[1]});
    for (unsigned n = p.functions.size(); n; --n)
        p.function_order.push_back(lowir_model::FunctionId(n));
    auto original = p.function_order;
    native::Statistics stats;
    native::legalize_extended_floats(p,stats);
    if (!stats.extended_helpers || p.function_order.size() != p.functions.size())
        throw std::runtime_error("missing legalized helper schedule entries");
    for (unsigned n = 0; n < original.size(); ++n)
        if (original[n].index != p.function_order[n].index) throw std::runtime_error("prior function order changed");
    lowir_model::validate(p);
    auto object = cppgm::toolchain::compile_object(p,stats,true,3);
    cppgm::toolchain::write_host_object(std::move(object),argv[2]);
}
