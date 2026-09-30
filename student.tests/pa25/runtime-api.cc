#include "toolchain/runtime.h"
#include <fstream>
#include <iostream>
using namespace lowir_model;
int main(int argc,char** argv)
{
    cppgm::toolchain::RuntimeProgram r;
    for (auto& table : r.tables) { table = r.symbol("",SR_NONE); r.table(table); }
    auto symbol = r.symbol("__dynamic_cast",SR_DYNAMIC_CAST);
    cppgm::toolchain::build_dynamic_cast(r,symbol);
    cppgm::toolchain::build_exceptions(r,{{"__cxa_allocate_exception",SR_EH_ALLOCATE_EXCEPTION},
        {"__cxa_throw",SR_EH_THROW},{"__cxa_begin_catch",SR_EH_BEGIN_CATCH},
        {"__cxa_end_catch",SR_EH_END_CATCH},{"__cxa_rethrow",SR_EH_RETHROW},
        {"__cxa_free_exception",SR_EH_FREE_EXCEPTION},{"__cxa_bad_cast",SR_BAD_CAST},
        {"__cxa_bad_typeid",SR_BAD_TYPEID},{"allocate",SR_ALLOCATE_MEMORY}});
    validate(r.p); // explicit audit boundary, not production serialization
    std::ofstream lowir(std::string(argv[1])+".lowir"); write_program(r.p,lowir);
    std::ofstream mir(std::string(argv[1])+".mir");
    native::Statistics stats; native::Image image(r.p.symbols.size());
    native::compile_image(r.p,image,{},&mir,stats);
    for (auto role : {SR_ALLOCATE_MEMORY,SR_FREE_MEMORY,SR_PURE_VIRTUAL}) {
        auto f = native::process_runtime(symbol,role);
        native::dump_function(r.p,f,mir);
    }
    auto object=cppgm::toolchain::runtime_object({{"allocate",SR_ALLOCATE_MEMORY},{"free",SR_FREE_MEMORY}},stats);
    cppgm::toolchain::write_object(object,std::string(argv[1])+".obj");
    auto read=cppgm::toolchain::read_object(std::string(argv[1])+".obj");
    if(read.image.code!=object.image.code || read.symbols.size()!=object.symbols.size())return 1;
    std::cout<<"runtime IR validation, MIR and object roundtrip passed\n";
}
