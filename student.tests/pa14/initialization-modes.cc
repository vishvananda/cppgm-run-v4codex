// Inspect the internal cache with unchanged layout and production object code.
// Load dependencies before exposing only Analyzer's private test surface.
#include "semantic/type_query.h"
#include "semantic/expression_store.h"
#include "semantic/fact_store.h"
#include "semantic/template_binding.h"
#include "syntax/parser.h"
#define private public
#include "semantic/analyzer.h"
#undef private
#include "preprocess/preprocessor.h"
#include <cassert>
#include <iostream>
#include <string>
int main(int argc,char** argv) {
 using namespace cppgm;
 assert(argc==3); bool reverse=std::string(argv[2])=="reverse";
 Preprocessor pp(argv[1],"Sep 13 2026","00:00:00"); PostTokenCursor post(pp,pp.identifiers()); syntax::Ast ast(true);
 syntax::Cursor cursor(post,pp.identifiers(),ast); syntax::Parser parser(cursor,ast,pp.identifiers());
 semantic::Analyzer sem(ast,pp.identifiers(),true,true); parser.translation_unit(&sem);
 semantic::EntityId cls=0;
 for(semantic::EntityId e=1;e<sem.entities.size();++e)
  if(sem.entities[e].class_info) cls=e;
 assert(cls); auto t=sem.entities[cls].type, scope=sem.entities[cls].owner;
 semantic::Expression value;value.type=t;value.category=semantic::ValueCategory::Lvalue;
 auto qualified=sem.types.qualify(t,1);
 auto transfer=sem.standard_conversion(value,qualified);
 assert(transfer.valid()&&transfer.target==qualified);
 unsigned failures=0; std::size_t nodes=0,entities=0,plans=0,conversions=0;
 std::uint32_t ids[2]={};
 for(unsigned i=0;i<10000;++i) {
  for(unsigned j=0;j<2;++j) {
   bool direct=bool(j)^reverse;
   auto c=sem.list_initialization(0,t,scope,direct);
   assert(!ids[direct]||ids[direct]==c.materialization);ids[direct]=c.materialization;
   bool failed=false;
   try {sem.validate_list_plan(c.materialization);}
   catch(const std::exception& error) {
    failed=true;++failures;
    if(i) {auto f=dynamic_cast<const semantic::FailedSemanticFact*>(&error);assert(f&&f->fact==semantic::SemanticFact::ListConversion);}
   }
   assert(failed==!direct);
  }
  assert(ids[0]!=ids[1]);
  if(!i){nodes=ast.nodes.size();entities=sem.entities.size();plans=sem.list_plans.size();conversions=sem.conversions.size();}
  assert(nodes==ast.nodes.size()&&entities==sem.entities.size()&&plans==sem.list_plans.size()&&conversions==sem.conversions.size());
 }
 assert(failures==10000);
 std::cout<<"10000 terminal copy failures; 10000 direct successes; stable nodes/entities/plans/conversions\n";
}
