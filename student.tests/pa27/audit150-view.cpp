// Explicit audit adapter: inspect the same typed host pipeline used by -c.
#include "lowering/procedural.h"
#include "toolchain/host_config.h"
#include "toolchain/object.h"
#include "native/encoding.h"
#include <fstream>
#include <iostream>
int main(int argc, char** argv) {
    if (argc != 3) return 2;
    std::string output = argv[2];
    std::vector<std::string> includes, macros;
    cppgm::toolchain::host_environment(includes,macros);
    lowir_model::Program program;
    cppgm::lowering::build_program(program,{argv[1]},true,includes,macros,false,true);
    lowir_model::validate(program);
    std::ofstream ir(output+".lowir"), mir(output+".mir");
    lowir_model::write_program(program,ir);
    native::Image image(program.symbols.size());
    image.host = true; image.host_resume = image.runtime_begin;
    native::Statistics stats, object_stats;
    native::compile_image(program,image,{},&mir,stats);
    // Re-encoding here is audit-only. Verify the view's image and ordinary -c
    // object are identical; production never constructs the text views.
    auto object = cppgm::toolchain::compile_object(program,object_stats,true);
    if (object.image.code != image.code || object.image.data != image.data ||
        object.image.tls != image.tls) return 3;
    cppgm::toolchain::write_host_object(std::move(object),output+".o");
    std::cout << "{\"functions\":" << stats.functions
              << ",\"instructions\":" << stats.instructions
              << ",\"frame_bytes\":" << stats.frame_bytes
              << ",\"text_bytes\":" << stats.text_bytes
              << ",\"value_visits\":" << stats.value_visits
              << ",\"parameter_flow_visits\":" << stats.parameter_flow_visits
              << ",\"carry_window_visits\":" << stats.carry_window_visits << "}\n";
    ir.close(); mir.close();
    return ir && mir ? 0:4;
}
