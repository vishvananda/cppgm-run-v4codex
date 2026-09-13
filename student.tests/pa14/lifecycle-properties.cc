#include "semantic/analyzer.h"
#include "preprocess/preprocessor.h"
#include <iostream>
#include <string>
#include <cassert>
int main(int argc,char** argv) {
 using namespace cppgm;
 assert(argc==3);
 Preprocessor pp(argv[1],"Sep 13 2026","00:00:00");PostTokenCursor post(pp,pp.identifiers());syntax::Ast ast(true);
 syntax::Cursor cursor(post,pp.identifiers(),ast);syntax::Parser parser(cursor,ast,pp.identifiers());auto root=parser.translation_unit();
 semantic::Analyzer sem(ast,pp.identifiers(),true,true);
 std::string mode=argv[2];bool forward=mode=="forward";
 auto n=ast[root].first;
 if(forward){sem.consume(n);n=ast[n].next;}
 else {for(;n;n=ast[n].next)sem.consume(n);}
 semantic::TypeId target=0;
 for(const auto& e:sem.entities)if(e.name){auto spelling=pp.identifiers().spelling(e.name);if(std::string(spelling.data,spelling.size)=="Target")target=e.type;}
 assert(target);
 if(mode=="failure") {
  unsigned failures=0;std::size_t nodes=0,entities=0;
  for(unsigned i=0;i<10000;++i) {
   try {sem.trivial_destructor(target);}catch(const std::exception& error){
    ++failures;
#ifdef EXPECT_PROPERTY_COMPLETION
    if(i)assert(dynamic_cast<const semantic::FailedSemanticFact*>(&error));
#endif
   }
   if(!i){nodes=ast.nodes.size();entities=sem.entities.size();}
   assert(nodes==ast.nodes.size()&&entities==sem.entities.size());
  }
  std::cout<<"failures "<<failures<<" nodes "<<nodes<<" entities "<<entities<<"\n";
#ifdef EXPECT_PROPERTY_COMPLETION
  assert(failures==10000);
#endif
  return 0;
 }
 int before=-1;
 try {before=sem.trivial_destructor(target);}catch(const std::exception& e){std::cout<<e.what()<<'\n';}
 for(;n;n=ast[n].next)sem.consume(n);
 sem.finish();
 bool after=sem.trivial_destructor(target);
 std::cout<<"before "<<before<<" after "<<after<<"\n";
#ifdef EXPECT_PROPERTY_COMPLETION
 assert(before==(forward?-1:0));assert(!after);
#endif
}
