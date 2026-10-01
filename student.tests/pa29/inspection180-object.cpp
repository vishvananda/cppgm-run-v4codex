#include "lowir/model.h"
#include "toolchain/object.h"
#include <fstream>
#include <iostream>
int main(int argc, char** argv) {
    try {
        if (argc != 4) return 1;
        auto p = lowir_model::parse_lowir_program_files({argv[1]});
        lowir_model::expand_forced_calls(p);
        lowir_model::validate(p);
        std::ofstream out(argv[2]); lowir_model::write_program(p,out); out.close();
        native::Statistics stats;
        cppgm::toolchain::write_host_object(cppgm::toolchain::compile_object(p,stats,true),argv[3]);
        std::cout << p.stats.inline_calls << ' ' << p.stats.inline_work << '\n';
    } catch (const std::exception& e) { std::cerr << e.what() << '\n'; return 1; }
}
