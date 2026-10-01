// Explicit PA9 adapter roundtrip; expected spelling follows the vendor vector grammar.
#include "abi/itanium/fact_writer.h"
#include <iostream>
int main(){
    using namespace abi_mangle;
    Graph graph;
    auto lane=graph.builtin(ABI_BUILTIN_TYPE_INT);
    auto count=graph.make(Kind::ExprParameter,0,0,0,0);
    Target target;target.type=graph.make(Kind::Vector,lane,count);
    if(mangle(graph,target)!="DvT__i")return 1;
    FactWriter writer(graph);
    auto text=writer.write(target);
    if(text.find("vector-expression")==std::string::npos)return 2;
    auto parsed=parse_fact_text(text);
    if(parsed.cases.size()!=1 || mangle(parsed.graph,parsed.cases[0])!="DvT__i")return 3;
    FactWriter roundtrip(parsed.graph);
    if(roundtrip.write(parsed.cases[0])!=text)return 4;
    std::cout<<text;
}
