#include "semantic/analyzer.h"
#include <cassert>
#include <iostream>
#include <sstream>
using namespace cppgm;
using namespace cppgm::semantic;
int main()
{
    IdentifierTable ids;
    syntax::Ast ast(true);
    Analyzer sem(ast, ids, true);
    SourceBuffer source(
        "int choose(int);long choose(long);int (*fp)(int)=choose;"
        "int twice(int a,int b){return a+b;}"
        "int f(short x){return twice(choose(x),choose(x));}"
        "struct C{void used(){int local=2;} void unused(){missing();}};"
        "using M=void(C::*)();M m=&C::used;"
        "template<class T>void target(T);template<class T>void consume(T);"
        "void a(){consume(static_cast<void(*)(int)>(&target<int>));}"
        "void b(){consume(static_cast<void(*)(int)>(&target<int>));}"
        "int (*make())(int); int invoke(){return make()(1);}"
        "int left();int right();int branch(bool c){return(c?left:right)();}"
        "void side();int comma(){return(side(),left)();}"
        "int arithmetic(short x,unsigned long u,long long l){short s=x;s+=x;return(true?s+1:s+2)+(u+l);}"
        "void variadic(int,...);void varargs(short x,float y){variadic(1,x,y);}"
        "int unevaluated(){return sizeof(&C::unused);}", 9);
    syntax::NodeId root;
    {
        PPTokenCursor pp(source, ids);
        PostTokenCursor post(pp, ids, true);
        syntax::Cursor cursor(post, ids, ast);
        syntax::Parser parser(cursor, ast, ids);
        root = parser.translation_unit(&sem);
    }
    sem.finish();
    unsigned selected=0, two_arguments=0, specialized=0, completed_members=0, indirect=0, operators=0, variadic=0;
    for (syntax::NodeId n=1;n<ast.nodes.size();++n) {
        const Expression& fact=sem.expression_fact(n);
        if (!fact.ready) continue;
        if (ast[n].kind==syntax::Kind::Parenthesized) assert(!fact.count && !fact.conversions);
        if (ast[n].kind==syntax::Kind::Binary && ast[n].op==OP_PLUS) {
            ++operators;
            assert(fact.count==2);
            unsigned i=0;
            for(syntax::NodeId c=ast[n].first;c;c=ast[c].next,++i) {
                assert(sem.expression_fact(c).incoming==fact.conversions+i);
                assert(sem.conversion_fact(fact.conversions+i).target==fact.type);
            }
        }
        if (ast[n].kind==syntax::Kind::Conditional) {
            assert(fact.count==3 && !fact.entity);
            unsigned i=0;
            for(syntax::NodeId c=ast[n].first;c;c=ast[c].next,++i)
                assert(sem.expression_fact(c).incoming==fact.conversions+i);
        }
        if (ast[n].kind==syntax::Kind::Assignment && ast[n].op==OP_PLUSASS) {
            assert(fact.count==3 && fact.category==ValueCategory::Lvalue);
            assert(sem.conversion_fact(fact.conversions).target==sem.types.fundamental(FT_INT));
            assert(sem.conversion_fact(fact.conversions+2).target==sem.types.fundamental(FT_SHORT_INT));
        }
        if (ast[n].kind != syntax::Kind::Call) continue;
        const Expression& expression=sem.expression_fact(n);
        if (!sem.facts[n].entity) {
            if(expression.form==ExpressionForm::Ordinary) { ++indirect; assert(!expression.entity); }
            continue;
        }
        assert(!expression.entity); // A call target is not the returned value.
        const Entity& e=sem.entities[sem.facts[n].entity];
        TextView name=ids.spelling(e.name);
        std::string spelling(name.data,name.size);
        if (spelling=="choose") {
            ++selected;
            assert(expression.count==1);
            const Conversion& c=sem.conversion_fact(expression.conversions);
            assert(c.rank==1 && c.target==sem.types.fundamental(FT_INT));
        }
        if (spelling=="twice") {
            ++two_arguments;
            assert(expression.count==2 && expression.incoming);
            for(unsigned i=0;i<2;++i) assert(sem.conversion_fact(expression.conversions+i).target==sem.types.fundamental(FT_INT));
        }
        if (spelling=="variadic") {
            ++variadic; assert(expression.count==3);
            assert(sem.conversion_fact(expression.conversions+1).target==sem.types.fundamental(FT_INT));
            assert(sem.conversion_fact(expression.conversions+2).target==sem.types.fundamental(FT_DOUBLE));
        }
    }
    for (const Entity& e:sem.entities) {
        if (e.specialization) ++specialized;
        if (e.member_info && e.definition) ++completed_members;
    }
    assert(selected==2 && two_arguments==1 && specialized==2 && completed_members==1);
    assert(indirect==3 && operators==5 && variadic==1);
    std::ostringstream first,second;
    sem.write_semantics(first,root); sem.finish(); sem.write_semantics(second,root);
    assert(first.str()==second.str());
    assert(first.str().find("C::unused")==std::string::npos);
    assert(first.str().find("function-definition C::used")!=std::string::npos);
    std::cout<<"PA7 selected conversions, canonical specialization reuse, demand and source graph API passed\n";
}
