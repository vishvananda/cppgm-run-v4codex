#include "abi/itanium/fact_writer.h"
#include <iostream>
int main(){
 using namespace abi_mangle;
 Graph g;auto i=g.builtin(ABI_BUILTIN_TYPE_INT);
 auto function=g.make(Kind::FunctionType,i,0,0,0,{i});
 Target target;target.type=g.make(Kind::Vendor,function,g.string("block_pointer"));
 const char* expected="U13block_pointerFiiE";
 if(mangle(g,target)!=expected)return 1;
 FactWriter writer(g);auto text=writer.write(target);auto parsed=parse_fact_text(text);
 if(parsed.cases.size()!=1 || mangle(parsed.graph,parsed.cases[0])!=expected)return 2;
 FactWriter again(parsed.graph);if(again.write(parsed.cases[0])!=text)return 3;
 std::cout<<text;
}
