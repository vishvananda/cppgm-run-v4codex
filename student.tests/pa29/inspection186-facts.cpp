#include "semantic/analyzer.h"
#include "preprocess/preprocessor.h"
#include <iostream>
#include <fstream>
#include "toolchain/host_config.h"
int main(int argc,char**argv) {
    try {
        if(argc!=3)return 2;
        cppgm::Preprocessor pp(argv[1],"Oct  1 2026","00:00:00",true,true);
        std::vector<std::string> includes, macros;
        cppgm::toolchain::host_environment(includes,macros);
        pp.include_paths(includes);pp.command_options(macros);
        cppgm::PostTokenCursor post(pp,pp.identifiers(),false,0,true,true);
        cppgm::syntax::Ast ast(true);
        cppgm::syntax::Cursor cursor(post,pp.identifiers(),ast);
        cppgm::syntax::Parser parser(cursor,ast,pp.identifiers());
        cppgm::semantic::Analyzer sem(ast,pp.identifiers(),true,true,true);
        auto root=parser.translation_unit(&sem); sem.finish();
        std::ofstream view(argv[2]);cppgm::syntax::write_ast(view,ast,root,pp.identifiers());
        std::cout<<"{\"parsed_nodes\":"<<ast.nodes.parsed_size()<<",\"guides\":"<<sem.deduction_guides.size()-1<<",\"rtti_uses\":"<<sem.rtti_expressions.size()-1<<",\"casts\":[";
        bool comma=false;
        for(unsigned n=1;n<ast.nodes.size();++n) {
            if(ast[n].kind!=cppgm::syntax::Kind::Cast)continue;
            auto x=sem.expression_fact(n);
            if(!x.ready || !x.count)continue;
            auto c=sem.conversion_fact(x.conversions);
            if(comma)std::cout<<',';comma=true;
            std::cout<<"{\"node\":"<<n<<",\"source\":"<<ast.nodes.occurrences[n].source
                <<",\"context\":"<<ast.nodes.occurrences[n].context
                <<",\"type\":"<<x.type<<",\"target\":"<<c.target
                <<",\"reference\":"<<c.reference<<",\"derived\":"<<c.derived
                <<",\"adjustment\":"<<c.adjustment
                <<",\"offset\":"<<(c.adjustment?sem.base_adjustments[c.adjustment].total:0)
                <<",\"constant_forbidden\":"<<c.constant_forbidden<<'}';
        }
        std::cout<<"]}\n";
    }catch(const std::exception&e){std::cerr<<e.what()<<'\n';return 1;}
}
