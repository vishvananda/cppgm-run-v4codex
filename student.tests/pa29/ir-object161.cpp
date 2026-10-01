// Explicit external-LowIR test adapter using the production native/object phases.
#include "lowir/model.h"
#include "native/encoding.h"
#include "toolchain/object.h"
#include <exception>
#include <iostream>
int main(int argc,char** argv) {
    try {
        if(argc!=3)return 2;
        auto program=lowir_model::parse_lowir_program_files({argv[1]});
        native::Statistics stats;
        cppgm::toolchain::write_host_object(cppgm::toolchain::compile_object(program,stats,true),argv[2]);
        return 0;
    } catch(const std::exception& e) {std::cerr<<e.what()<<'\n';return 1;}
}
