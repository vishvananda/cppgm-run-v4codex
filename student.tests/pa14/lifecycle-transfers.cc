#include "semantic/analyzer.h"
#include "preprocess/preprocessor.h"
#include <cassert>
#include <iostream>
#include <string>
int main(int argc,char** argv) {
 using namespace cppgm;
 assert(argc==3);bool failure=std::string(argv[2])=="failure";
 Preprocessor pp(argv[1],"Sep 13 2026","00:00:00");PostTokenCursor post(pp,pp.identifiers());syntax::Ast ast(true);
 syntax::Cursor cursor(post,pp.identifiers(),ast);syntax::Parser parser(cursor,ast,pp.identifiers());semantic::Analyzer sem(ast,pp.identifiers(),true,true);parser.translation_unit(&sem);
 semantic::EntityId member=0;
 for(semantic::EntityId e=1;e<sem.entities.size();++e)if(sem.entities[e].member_info&&sem.member_fact(e).transfer!=semantic::TransferKind::None)member=e;
 assert(member);
 unsigned failures=0;std::size_t nodes=0,entities=0;
 for(unsigned i=0;i<10000;++i) {
  try {assert(!sem.function_nonthrowing(member));}catch(const std::exception& error) {
   ++failures;
#ifdef EXPECT_TRANSFER_FACTS
   if(i){auto fact=dynamic_cast<const semantic::FailedSemanticFact*>(&error);assert(fact&&fact->fact==semantic::SemanticFact::Transfer);}
#endif
  }
  if(!i){nodes=ast.nodes.size();entities=sem.entities.size();}
  assert(nodes==ast.nodes.size()&&entities==sem.entities.size());
 }
 std::cout<<"failures "<<failures<<" deleted "<<sem.member_fact(member).deleted<<'\n';
#ifdef EXPECT_TRANSFER_FACTS
 assert(failures==(failure?10000u:0u));
 assert(sem.member_fact(member).transfer_state==(failure?semantic::FactState::Failure:semantic::FactState::Success));
 assert(sem.member_fact(member).deleted!=failure);
#endif
}
