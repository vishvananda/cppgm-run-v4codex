#include "abi/itanium/fact_writer.h"
#include <iostream>
int main() {
    using namespace abi_mangle;
    Graph graph;
    auto result=graph.builtin(ABI_BUILTIN_TYPE_VOID);
    auto a=graph.make(Kind::Parameter,0,1,0,0);
    auto b=graph.make(Kind::Parameter,0,1,1,0);
    if (a==b || a!=graph.make(Kind::Parameter,0,1,0,0)) return 1;
    Target target;
    target.type=graph.make(Kind::FunctionType,result,0,0,0,{a,b,b});
    FactWriter writer(graph); auto text=writer.write(target);
    auto parsed=parse_fact_text(text);
    if (mangle(graph,target)!=mangle(parsed.graph,parsed.cases[0])) return 2;
    FactWriter again(parsed.graph);
    if (text!=again.write(parsed.cases[0])) return 3;
    std::cout<<text;
}
