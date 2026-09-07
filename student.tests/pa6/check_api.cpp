#include "semantic/analyzer.h"
#include <cassert>
#include <sstream>
#include <iostream>
using namespace cppgm;
using namespace cppgm::semantic;

int main()
{
    IdentifierTable ids;
    syntax::Ast ast(true);
    Analyzer sem(ast, ids);
    SourceBuffer source(
        "void f(const int); void f(volatile int);"
        "void g(int[3]); void g(int*);"
        "using F=int(const int); F* fp;"
        "extern int a[];int a[3]; using U=int[]; U* p;"
        "using R=int&;using RR=R&&;"
        "namespace A{using T=int;} namespace B{using T=int;}"
        "template<class T>struct Box{T member;};"
        "struct C{void method(){Later x;} using Later=int;};", 11);
    {
        PPTokenCursor pp(source, ids);
        PostTokenCursor post(pp, ids, true);
        syntax::Cursor cursor(post, ids, ast);
        syntax::Parser parser(cursor, ast, ids);
        parser.translation_unit(&sem);
    }
    sem.finish();
    auto name = [&](IdentifierId id) { TextView t=ids.spelling(id);return std::string(t.data,t.size); };
    EntityId f=0,g=0,r=0,array=0;
    unsigned fdecls=0,gdecls=0;
    for (std::size_t i=1;i<sem.declarations.size();++i) {
        const Declaration& d=sem.declarations[i];const Entity& e=sem.entities[d.entity];
        if (!e.name) continue;
        std::string n=name(e.name);
        assert(d.source < ast.nodes.size());
        if(n=="f") {
            if(f) assert(f==d.entity);
            f=d.entity; ++fdecls;
            Type t=sem.types[e.type];assert(t.count==1);
            assert(sem.types[sem.types.parameters[t.offset]].cv==0);
            assert(sem.types[sem.types.parameters[sem.types[d.type].offset]].cv!=0);
        }
        if(n=="g") {if(g) assert(g==d.entity);g=d.entity;++gdecls;}
        if(n=="a") {if(array) assert(array==d.entity);array=d.entity;assert(sem.types[e.type].bound==3);}
        if(n=="U") assert(sem.types[e.type].bound==0);
        if(n=="R") r=d.entity;
        if(n=="RR") assert(sem.entities[r].type==e.type);
        if(n=="method") {
            assert(e.definition && e.scope);
            assert(sem.facts[e.definition].entity == d.entity);
            assert(sem.facts[e.definition].scope == e.scope);
        }
        if(n=="member") assert(sem.types[e.type].kind==TypeKind::Named);
    }
    assert(fdecls==2 && gdecls==2 && array);
    std::size_t transitions=sem.types.signature_work;
    for(int i=0;i<10000;++i) sem.types.signature(sem.entities[f].type);
    assert(sem.types.signature_work<=transitions+1);
    // Interning and signature facts survive arena growth and share semantic IDs.
    TypeId original=sem.types.fundamental(FT_INT);
    TypeId deep=original;
    for(int i=0;i<1000;++i) deep=sem.types.compound(TypeKind::Pointer,deep);
    assert(sem.types.signature(deep)==deep);
    transitions=sem.types.signature_work;
    for(int i=0;i<1000;++i) assert(sem.types.signature(deep)==deep);
    assert(sem.types.signature_work==transitions);
    assert(sem.types.fundamental(FT_INT)==original);
    std::ostringstream out;sem.write(out);
    assert(out.str().find("variable x int")!=std::string::npos);
    std::cout<<"PA6 semantic identity/lifetime API passed\n";
}
