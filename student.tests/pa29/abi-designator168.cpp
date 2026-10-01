#include "abi/itanium/fact_writer.h"
#include <iostream>
int main(){
 using namespace abi_mangle;
 Graph g;auto t=g.make(Kind::Parameter);auto i=g.builtin(ABI_BUILTIN_TYPE_INT);
 auto value=g.make(Kind::Value,i,0,0,3);
 auto designation=g.make(Kind::DesignatedInit,value,g.string("member"));
 Target target;target.type=g.make(Kind::Decltype,g.make(Kind::InitList,t,0,0,0,{designation}));
 const char* expected="DTtlT_di6memberLi3EEE";
 if(mangle(g,target)!=expected){std::cerr<<mangle(g,target)<<'\n';return 1;}
 FactWriter writer(g);auto text=writer.write(target);auto parsed=parse_fact_text(text);
 if(parsed.cases.size()!=1 || mangle(parsed.graph,parsed.cases[0])!=expected)return 2;
 FactWriter again(parsed.graph);if(again.write(parsed.cases[0])!=text)return 3;
 std::cout<<text;
}
