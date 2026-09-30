#include "lowering/procedural.h"
#include "toolchain/object.h"
#include <fstream>
#include <iostream>
#include <iterator>
#include <sstream>
#include <stdexcept>
int main(int argc,char** argv)
{
    lowir_model::Program p;
    cppgm::lowering::build_program(p,{argv[1]},true,{},{});
    lowir_model::validate(p);
    std::ofstream ir(std::string(argv[2])+".lowir");lowir_model::write_program(p,ir);ir.close();
    std::ifstream input(std::string(argv[2])+".lowir");
    std::string text((std::istreambuf_iterator<char>(input)),{});
    lowir_model::Program roundtrip;lowir_model::read_program(roundtrip,text,"production-view");lowir_model::validate(roundtrip);
    std::ofstream view(std::string(argv[2])+".roundtrip");lowir_model::write_program(roundtrip,view);view.close();
    // First serialization can carry optional f/L suffixes. The reader records
    // the type separately and canonicalizes those spellings. Require stability
    // after canonicalization and identical encoded behavior, not suffix identity.
    std::ostringstream canonical;lowir_model::write_program(roundtrip,canonical);
    lowir_model::Program again;lowir_model::read_program(again,canonical.str(),"canonical-view");
    lowir_model::validate(again);std::ostringstream stable;lowir_model::write_program(again,stable);
    if(canonical.str()!=stable.str())throw std::runtime_error("unstable canonical LowIR");
    native::Statistics roundtrip_stats;
    auto roundtrip_obj=cppgm::toolchain::compile_object(roundtrip,roundtrip_stats);
    cppgm::toolchain::Linker roundtrip_linker;roundtrip_linker.add(std::move(roundtrip_obj));
    roundtrip_linker.finish(std::string(argv[2])+".roundtrip.elf");
    std::ofstream mir(std::string(argv[2])+".mir"); native::Statistics stats;native::Image image(p.symbols.size());
    native::compile_image(p,image,{},&mir,stats);
    auto obj=cppgm::toolchain::compile_object(p,stats);
    cppgm::toolchain::write_object(obj,std::string(argv[2])+".obj");
    cppgm::toolchain::Linker linker;linker.add(std::move(obj));linker.finish(std::string(argv[2])+".elf");
    std::cout << "typed production LowIR, adapter roundtrip, MIR and ELF passed\n";
}
