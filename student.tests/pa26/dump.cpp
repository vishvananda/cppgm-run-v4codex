#include "lowering/procedural.h"
#include "lowir/writer.h"
#include <iostream>
#include "native/encoding.h"
int main(int argc, char** argv) {
    if (argc < 2 || argc > 3) return 1;
    lowir_model::Program p;
    cppgm::lowering::build_program(p,{argv[1]},false,{},{},false,true);
    if (argc == 2) lowir_model::write_program(p,std::cout);
    else {
        native::Image image(p.symbols.size()); image.host = true; image.host_resume = image.runtime_begin;
        native::Statistics stats; native::compile_image(p,image,{},&std::cout,stats);
    }
}
