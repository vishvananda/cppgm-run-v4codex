#include "lowir/reader.h"
#include "toolchain/object.h"
#include "toolchain/elf_model.h"
#include <iostream>
int main(int argc,char**argv){try{if(argc!=3)return 2;auto p=lowir_model::parse_lowir_program_files({argv[1]});native::Statistics stats;auto object=cppgm::toolchain::compile_object(p,stats,true);cppgm::toolchain::write_host_object(std::move(object),argv[2]);return 0;}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
