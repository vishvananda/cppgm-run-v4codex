// Explicit audit adapter: hosted object reconstruction and the actual hosted
// MIR selector view. Each inspection invocation starts from external LowIR.
#include "lowir/model.h"
#include "native/encoding.h"
#include "toolchain/object.h"
#include <fstream>
#include <iostream>
int main(int argc, char** argv) {
    try {
        if (argc != 4) return 2;
        auto p = lowir_model::parse_lowir_program_files({argv[1]});
        native::Statistics stats;
        cppgm::toolchain::write_host_object(cppgm::toolchain::compile_object(p, stats, true), argv[2]);
        // A second independent inspection avoids preparing a mutated Program
        // twice. This is an explicit tool adapter, never production transport.
        auto view = lowir_model::parse_lowir_program_files({argv[1]});
        native::Statistics view_stats;
        native::legalize_extended_floats(view, view_stats);
        native::Image image(view.symbols.size());
        image.host = true;
        image.host_resume = image.runtime_begin;
        std::ofstream mir(argv[3]);
        native::compile_image(view, image, {}, &mir, view_stats);
        mir.close();
        return mir ? 0 : 1;
    } catch (const std::exception& e) {
        std::cerr << e.what() << '\n';
        return 1;
    }
}
