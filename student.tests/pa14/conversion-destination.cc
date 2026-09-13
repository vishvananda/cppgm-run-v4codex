#include "semantic/analyzer.h"
#include "preprocess/preprocessor.h"
#include <cassert>
#include <iostream>
#include <string>
int main(int argc,char** argv) {
 using namespace cppgm; using namespace semantic;
 assert(argc==2||argc==3);bool user=argc==3;
 Preprocessor pp(argv[1],"Sep 13 2026","00:00:00");PostTokenCursor post(pp,pp.identifiers());syntax::Ast ast(true);
 syntax::Cursor cursor(post,pp.identifiers(),ast);syntax::Parser parser(cursor,ast,pp.identifiers());Analyzer sem(ast,pp.identifiers(),true,true);
 parser.translation_unit(&sem);sem.finish();
 unsigned destination=0,temporary=0,recipes=0;
 for(unsigned i=1;i<sem.conversion_objects.size();++i) {
  const auto& c=sem.conversion_objects[i];
  if(c.use==ConversionUse::Destination){++destination;assert(!c.temporary);}
  else if(c.use==ConversionUse::Temporary){++temporary;assert(c.temporary&&sem.entities[c.temporary].kind==EntityKind::Variable);}
  else {++recipes;assert(!c.temporary);}
 }
 if(user) assert(destination==2&&temporary==0&&recipes==1);
 else assert(destination==4&&temporary==2&&recipes==3);
 unsigned user_destinations=0;
 for(unsigned i=1;i<sem.user_conversions.size();++i) {
  const auto& c=sem.user_conversions[i];
  if(c.use==ConversionUse::Destination){++user_destinations;assert(c.prepared&&!c.temporary&&!c.source_temporary);}
 }
 assert(user_destinations==(user?2u:0u));
 auto nodes=ast.nodes.size(),entities=sem.entities.size(),objects=sem.conversion_objects.size();
 for(unsigned i=0;i<10000;++i){sem.finish();assert(nodes==ast.nodes.size()&&entities==sem.entities.size()&&objects==sem.conversion_objects.size());}
 std::cout<<destination<<" destinations, "<<temporary<<" temporaries, "<<recipes<<" recipes; 10000 stable finish queries\n";
}
