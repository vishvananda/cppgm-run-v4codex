#include "semantic/analyzer.h"
#include "preprocess/preprocessor.h"
#include <cassert>
#include <iostream>
#include <string>
int main(int argc,char** argv) {
 using namespace cppgm;
 assert(argc==3);std::string mode=argv[2];bool ctor=mode.find("ctor")==0,failed=mode.find("failure")!=std::string::npos;
 Preprocessor pp(argv[1],"Sep 13 2026","00:00:00");PostTokenCursor post(pp,pp.identifiers());syntax::Ast ast(true);
 syntax::Cursor cursor(post,pp.identifiers(),ast);syntax::Parser parser(cursor,ast,pp.identifiers());auto root=parser.translation_unit();
 semantic::Analyzer sem(ast,pp.identifiers(),true,true);
 semantic::EntityId target=0,member=0;auto n=ast[root].first;
 for(;n;n=ast[n].next) {
  sem.consume(n);
  for(semantic::EntityId e=1;e<sem.entities.size();++e)if(sem.entities[e].class_info&&sem.entities[e].name) {
   auto spelling=pp.identifiers().spelling(sem.entities[e].name);
   if(std::string(spelling.data,spelling.size)=="Target")target=e;
  }
  if(target){n=ast[n].next;break;}
 }
 assert(target);
 for(semantic::EntityId e=1;e<sem.entities.size();++e)if(sem.entities[e].member_info&&sem.entities[e].owner==sem.entities[target].scope) {
  const auto& f=sem.member_fact(e);if(ctor?f.constructor:f.destructor)member=e;
 }
 assert(member);
 auto query=[&](){return ctor?sem.constructor_needed(member):sem.destructor_needed(member);};
 int before=-1;
 try {before=query();}catch(const std::exception& error){std::cout<<error.what()<<'\n';}
 bool rejected=false;
 try {for(;n;n=ast[n].next)sem.consume(n);sem.finish();}catch(const std::exception& error){rejected=true;std::cout<<error.what()<<'\n';}
 assert(rejected==failed);
 unsigned failures=0;int after=-1;auto nodes=ast.nodes.size(),entities=sem.entities.size();
 for(unsigned i=0;i<10000;++i) {
  try {after=query();}catch(const std::exception& error) {
   ++failures;
#ifdef EXPECT_ACTION_PUBLICATION
   assert(dynamic_cast<const semantic::FailedSemanticFact*>(&error));
#endif
  }
  assert(nodes==ast.nodes.size()&&entities==sem.entities.size());
 }
 std::cout<<"before "<<before<<" after "<<after<<" failures "<<failures<<'\n';
#ifdef EXPECT_ACTION_PUBLICATION
 assert(before==-1);
 assert(failed?failures==10000:(!failures&&after==1));
#endif
}
