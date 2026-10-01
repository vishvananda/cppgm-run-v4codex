#include "lowir/model.h"
#include "toolchain/object.h"
int main(int argc, char** argv) {
    if (argc != 3) return 1;
    auto program = lowir_model::parse_lowir_program_files({argv[1]});
    native::Statistics stats;
    cppgm::toolchain::write_host_object(cppgm::toolchain::compile_object(program,stats,true),argv[2]);
}
