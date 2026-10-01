#include "semantic/analyzer.h"
#include "preprocess/preprocessor.h"
#include "abi/itanium/abi_mangle.h"
#include <iostream>
#include <set>
#include <tuple>
int main(int argc,char**argv) {
 try {
  if(argc!=2)return 2;
  cppgm::Preprocessor pp(argv[1],"Oct  1 2026","00:00:00",true,true);
  cppgm::PostTokenCursor post(pp,pp.identifiers(),false,0,true,true);
  cppgm::syntax::Ast ast(true); cppgm::syntax::Cursor cursor(post,pp.identifiers(),ast);
  cppgm::syntax::Parser parser(cursor,ast,pp.identifiers());
  cppgm::semantic::Analyzer sem(ast,pp.identifiers(),true,true,true);
  parser.translation_unit(&sem);sem.finish();
  unsigned concrete=0,dependent=0,patterns=0;std::set<unsigned> widths;
  for(unsigned n=1;n<ast.nodes.parsed_size();++n) patterns+=ast[n].kind==cppgm::syntax::Kind::BitIntType;
  for(auto t:sem.types.records) {
   if(t.kind==cppgm::semantic::TypeKind::DependentBitInt) { ++dependent; if(!t.bound)return 3; }
   if(t.kind==cppgm::semantic::TypeKind::Fundamental && cppgm::semantic::bit_integer_kind(t.fundamental)) {
    ++concrete;widths.insert(t.bound);
    if(t.bound<1 || t.bound>128 || (t.fundamental==cppgm::FT_BITINT && t.bound==1))return 4;
   }
  }
  abi_mangle::AbiFactFile file; auto& g=file.graph;
  for(bool unsign:{false,true}) for(unsigned width:widths) {
   if(!unsign && width==1)continue;
   abi_mangle::Target target;target.kind=abi_mangle::TargetKind::Type;
   target.type=g.make(abi_mangle::Kind::BitInt,0,unsign,0,width);file.cases.push_back(target);
  }
  abi_mangle::Target dep;dep.kind=abi_mangle::TargetKind::Type;
  dep.type=g.make(abi_mangle::Kind::BitInt,g.make(abi_mangle::Kind::ExprParameter,0,0,0,0),false);
  file.cases.push_back(dep);
  auto text=abi_mangle::serialize_fact_file(file);auto parsed=abi_mangle::parse_fact_text(text);
  if(abi_mangle::mangle_fact_file(file)!=abi_mangle::mangle_fact_file(parsed))return 5;
  std::cout<<"{\"parsed_nodes\":"<<ast.nodes.parsed_size()<<",\"source_widths\":"<<patterns
    <<",\"concrete_types\":"<<concrete<<",\"dependent_types\":"<<dependent
    <<",\"abi_adapter_cases\":"<<file.cases.size()<<"}\n";
 }catch(const std::exception&e){std::cerr<<e.what()<<'\n';return 1;}
}
