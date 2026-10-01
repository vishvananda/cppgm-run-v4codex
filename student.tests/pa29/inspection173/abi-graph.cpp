#include "abi/itanium/fact_writer.h"
#include <iostream>
int main(){
 using namespace abi_mangle;
 Graph g;auto context=g.make(Kind::RawContext,g.string("Z1fvE"));
 auto p=g.make(Kind::TemplateParameterDeclaration,0);
 auto h=g.make(Kind::TemplateHead,0,0,0,0,{p});
 auto t=g.make(Kind::Parameter);
 Target target;target.type=g.make(Kind::Lambda,context,1,0,0,{h,t});
 const char* expected="Z1fvEUlTyT_E_";
 if(mangle(g,target)!=expected){std::cerr<<mangle(g,target);return 1;}
 FactWriter writer(g);auto text=writer.write(target);auto parsed=parse_fact_text(text);
 if(parsed.cases.size()!=1 || mangle(parsed.graph,parsed.cases[0])!=expected)return 2;
 FactWriter again(parsed.graph);if(again.write(parsed.cases[0])!=text){std::cerr<<text<<"\nAGAIN\n"<<again.write(parsed.cases[0]);return 3;}
 Target automatic;automatic.type=g.builtin(ABI_BUILTIN_TYPE_AUTO);
 if(mangle(g,automatic)!="Da")return 4;
 std::cout<<text;
}
