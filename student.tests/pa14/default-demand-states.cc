#include "semantic/analyzer.h"
#include "preprocess/preprocessor.h"
#include <cassert>
#include <iostream>
#include <string>
int main(int argc,char**argv){
 using namespace cppgm;
 assert(argc==3);std::string mode=argv[2];
 Preprocessor pp(argv[1],"Sep 13 2026","00:00:00");PostTokenCursor post(pp,pp.identifiers());syntax::Ast ast(true);
 syntax::Cursor cursor(post,pp.identifiers(),ast);syntax::Parser parser(cursor,ast,pp.identifiers());semantic::Analyzer sem(ast,pp.identifiers(),true,true);parser.translation_unit(&sem);
 std::vector<semantic::EntityId> functions;
 for(semantic::EntityId e=1;e<sem.entities.size();++e){const auto& item=sem.entities[e];if(item.kind!=semantic::EntityKind::Function||!item.defaults||item.template_pattern||item.template_info)continue;
  auto spelling=pp.identifiers().spelling(item.name);if(std::string(spelling.data,spelling.size)=="f")functions.push_back(e);
 }
 assert(functions.size()==(mode=="isolation"?2u:1u));
 unsigned failures=0;std::vector<semantic::NodeId> values(functions.size());std::size_t nodes=0,entities=0;
 for(unsigned i=0;i<10000;++i){
  for(unsigned j=0;j<functions.size();++j){
   try{auto value=sem.default_argument(functions[j],0);assert(!values[j]||value==values[j]);values[j]=value;}
   catch(const std::exception& error){++failures;if(i){auto fact=dynamic_cast<const semantic::FailedSemanticFact*>(&error);assert(fact&&fact->fact==semantic::SemanticFact::DefaultDemand);}}
  }
  if(!i&&mode=="body-failure"){
   bool failed=false;try{sem.finish();}catch(const std::exception&){failed=true;}assert(failed);
  }
  if(!i){nodes=ast.nodes.size();entities=sem.entities.size();}
  assert(nodes==ast.nodes.size()&&entities==sem.entities.size());
 }
 assert(failures==(mode=="dependency-failure"?10000u:0u));
 if(mode=="isolation")assert(values[0]!=values[1]);
 std::cout<<"failures "<<failures<<" nodes "<<nodes<<" entities "<<entities<<'\n';
 std::cout<<"{\"probe\":1";sem.telemetry(std::cout);std::cout<<"}\n";
}
