#include "semantic/analyzer.h"
#include "preprocess/preprocessor.h"
#include <cassert>
#include <iostream>
#include <string>
int main(int argc,char**argv){
 using namespace cppgm;
 assert(argc==3);bool failure=std::string(argv[2])=="failure";
 Preprocessor pp(argv[1],"Sep 13 2026","00:00:00");PostTokenCursor post(pp,pp.identifiers());syntax::Ast ast(true);
 syntax::Cursor cursor(post,pp.identifiers(),ast);syntax::Parser parser(cursor,ast,pp.identifiers());semantic::Analyzer sem(ast,pp.identifiers(),true,true);parser.translation_unit(&sem);
 semantic::EntityId function=0;
 for(semantic::EntityId e=1;e<sem.entities.size();++e)if(sem.entities[e].specialization&&sem.entities[e].defaults&&sem.entities[e].kind==semantic::EntityKind::Function)function=e;
 assert(function);auto slot=sem.entities[function].defaults;auto source=sem.default_arguments[slot];
 unsigned failures=0;semantic::NodeId value=0;std::size_t nodes=0,entities=0;
 for(unsigned i=0;i<10000;++i){
  try{auto v=sem.default_argument(function,0);assert(!value||v==value);value=v;}
  catch(const std::exception& error){++failures;if(i){auto fact=dynamic_cast<const semantic::FailedSemanticFact*>(&error);assert(fact&&fact->fact==semantic::SemanticFact::DefaultArgument);}}
  if(!i){nodes=ast.nodes.size();entities=sem.entities.size();}
  assert(nodes==ast.nodes.size()&&entities==sem.entities.size());
  assert(sem.default_arguments[slot]==source);
 }
 assert(failures==(failure?10000u:0u));
 std::cout<<"failures "<<failures<<" nodes "<<nodes<<" entities "<<entities<<'\n';std::cout<<"{\"probe\":1";sem.telemetry(std::cout);std::cout<<"}\n";
}
