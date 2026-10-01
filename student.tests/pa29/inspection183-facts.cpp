#include "semantic/analyzer.h"
#include "preprocess/preprocessor.h"
#include <iostream>
int main(int argc,char**argv) {
    try {
        if (argc!=2) return 2;
        cppgm::Preprocessor pp(argv[1],"Oct  1 2026","00:00:00",true,true);
        cppgm::PostTokenCursor post(pp,pp.identifiers(),false,0,true,true);
        cppgm::syntax::Ast ast(true);
        cppgm::syntax::Cursor cursor(post,pp.identifiers(),ast);
        cppgm::syntax::Parser parser(cursor,ast,pp.identifiers());
        cppgm::semantic::Analyzer sem(ast,pp.identifiers(),true,true,true);
        parser.translation_unit(&sem); sem.finish();
        std::cout << "{\"parsed_nodes\":" << ast.nodes.parsed_size() << ",\"guides\":[";
        for (unsigned i=1;i<sem.deduction_guides.size();++i) {
            const auto& g=sem.deduction_guides[i];
            auto result=sem.types[sem.types[g.signature].child].entity;
            auto text=pp.identifiers().spelling(sem.entities[g.primary].name);
            if(i>1)std::cout<<',';
            std::cout << "{\"primary\":" << g.primary << ",\"name\":\"" << std::string(text.data,text.size)
                << "\",\"signature\":" << g.signature << ",\"environment\":" << g.environment
                << ",\"source\":" << g.source << ",\"head\":" << g.head
                << ",\"explicit\":" << g.explicit_guide << ",\"explicit_query\":" << g.explicit_condition
                << ",\"noexcept\":" << g.nonthrowing << ",\"exception_query\":" << g.exception_condition
                << ",\"result_complete\":" << sem.entities[result].complete << '}';
        }
        std::cout << "]}" << '\n';
    } catch(const std::exception& e) {std::cerr<<e.what()<<'\n';return 1;}
}
