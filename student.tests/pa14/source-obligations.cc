#include "semantic/analyzer.h"
#include "preprocess/preprocessor.h"
#include <cassert>
#include <iostream>
#include <sstream>
int main(int argc,char** argv) {
 using namespace cppgm;using namespace semantic;
 assert(argc==2);
 Preprocessor pp(argv[1],"Sep 13 2026","00:00:00");PostTokenCursor post(pp,pp.identifiers());syntax::Ast ast(true);
 syntax::Cursor cursor(post,pp.identifiers(),ast);syntax::Parser parser(cursor,ast,pp.identifiers());
 auto root=parser.translation_unit();Analyzer sem(ast,pp.identifiers(),true,true);
 auto main=ast[root].last;
 for(auto n=ast[root].first;n!=main;n=ast[n].next)sem.consume(n);
 EntityId ctor=0,dtor=0;
 for(EntityId e=1;e<sem.entities.size();++e)if(sem.entities[e].member_info) {
  const auto& m=sem.member_fact(e);
  if(m.constructor&&m.synthetic) {assert(!ctor);ctor=e;}
  if(m.destructor&&m.synthetic) {assert(!dtor);dtor=e;}
 }
 assert(ctor&&dtor);
 assert(sem.member_fact(ctor).default_properties==BooleanFact::False);
 assert(sem.member_fact(dtor).destructor_properties==FactState::Success);
 assert(!sem.member_fact(ctor).referenced&&!sem.member_fact(dtor).referenced);
 auto nodes=ast.nodes.size(),entities=sem.entities.size(),scopes=sem.scopes.size();
 for(unsigned i=0;i<10000;++i) {
  for(auto e:{ctor,dtor}) {
   bool unavailable=false;
   try {if(e==ctor)sem.constructor_needed(e);else sem.destructor_needed(e);}
   catch(const UnavailableSemanticFact&) {unavailable=true;}
   assert(unavailable&&sem.member_fact(e).actions_state==FactState::NotStarted);
  }
  assert(nodes==ast.nodes.size()&&entities==sem.entities.size()&&scopes==sem.scopes.size());
 }
 sem.consume(main);sem.finish();
 std::ostringstream telemetry;sem.telemetry(telemetry);
 assert(telemetry.str().find("\"semantic_default_initialization_work\":1,")!=std::string::npos);
 assert(telemetry.str().find("\"semantic_default_initialization_uses\":2,")!=std::string::npos);
 assert(sem.member_fact(ctor).actions_state==FactState::Success);
 assert(sem.member_fact(dtor).actions_state==FactState::Success);
 nodes=ast.nodes.size();entities=sem.entities.size();scopes=sem.scopes.size();
 auto conversions=sem.conversion_objects.size(),users=sem.user_conversions.size();
 for(unsigned i=0;i<10000;++i) {
  sem.finish();
  assert(nodes==ast.nodes.size()&&entities==sem.entities.size()&&scopes==sem.scopes.size());
  assert(conversions==sem.conversion_objects.size()&&users==sem.user_conversions.size());
 }
 std::cout<<"10000 source property queries without body/actions demand; 10000 stable concrete finish queries\n";
}
