#!/usr/bin/env python3
"""Accumulated audit: discard effects, array plans and construction: CC WORK."""
from pathlib import Path
import sys
import ordering_controls as runner

runner.GOOD = {}
runner.BAD = {}
for name, declaration, expected in (
    ('copy', 'A(volatile A&);~A() noexcept;', False),
    ('destructor', 'A(volatile A&) noexcept;~A() noexcept(false);', False),
    ('default', 'A(volatile A&,int=maythrow()) noexcept;~A() noexcept;', False),
    ('quiet', 'A(volatile A&) noexcept;~A() noexcept;', True),
):
    prefix = 'int maythrow();struct A{' + declaration + '};extern volatile A a;'
    for form, expr in (('cast','(void)a'), ('functional','void(a)'), ('comma','(a,1)'), ('conditional','(void)(true?a:a)')):
        test = (' ' if expected else '!') + 'noexcept(' + expr + ')'
        runner.GOOD['effect_' + name + '_' + form] = prefix + 'static_assert(' + test + ',"");int main(){}'
        dependent = expr.replace('a','*p')
        runner.GOOD['query_' + name + '_' + form] = prefix + 'template<class T>constexpr bool test(T*p){return noexcept(' + dependent + ');}static_assert(' + ('' if expected else '!') + 'test(&a),"");int main(){}'
        runner.GOOD['signature_' + name + '_' + form] = prefix + 'template<class T,bool B=noexcept(' + expr.replace('a','*static_cast<T*>(0)') + ')>constexpr bool test(){return B;}static_assert(' + ('' if expected else '!') + 'test<volatile A>(),"");int main(){}'

runner.GOOD.update({
    'query_reference_call': 'struct A{A(volatile A&);};volatile A&get() noexcept;static_assert(noexcept((void)get()),"");int main(){}',
    'query_mixed_conditional': 'struct A{A(volatile A&);};extern volatile A a;volatile A&get() noexcept;static_assert(noexcept((void)(true?get():a)),"");int main(){}',
    'array_discard_lifetime': 'int copies,dtors;struct A{A(){}A(volatile A&){++copies;}~A(){++dtors;}};int main(){volatile A a;int values[]={((void)a,3),((void)a,5)};return copies!=2||dtors!=2||values[0]!=3||values[1]!=5;}',
    'array_temporary_lifetime': 'int hits;struct A{~A(){++hits;}};int main(){int values[]={(A(),3),(A(),5)};return hits!=2||values[0]!=3||values[1]!=5;}',
    'array_delegation': 'template<class T>struct A{T x;A():x(7){}A(int):A(){}};template<class T>struct B:A<T>{B():A<T>(1){}};int main(){B<int>a[]={B<int>(),B<int>()};return sizeof(a)!=2*sizeof(B<int>)||a[0].x!=7||a[1].x!=7;}',
    'branch_discard_cleanup': 'int copies,dtors;struct A{A(){}A(volatile A&){++copies;}~A(){++dtors;}};void f(bool b,volatile A&a){b?void(a):void();}int main(){volatile A a;f(false,a);f(true,a);return copies!=1||dtors!=1;}',
    'discard_default_temporary': 'int copies,dtors,defaults;struct D{D(){++defaults;}~D(){++dtors;}};struct A{A(){}A(volatile A&,const D& = D()){++copies;}};int main(){volatile A a;(void)a;return copies!=1||dtors!=1||defaults!=1;}',
    'template_discard_default_temporary': 'int copies,dtors,defaults;struct D{D(){++defaults;}~D(){++dtors;}};struct A{A(){}A(volatile A&,const D& = D()){++copies;}};volatile A a;template<class T>void f(){(void)a;}int main(){f<int>();f<long>();return copies!=2||dtors!=2||defaults!=2;}',
})

for name, expr in (
    ('braces','(A{},3)'), ('nested','((A(),1),3)'),
    ('conditional','(true?A():A(),3)'), ('conversion','A()'),
    ('member','A().n'),
):
    prefix = 'int hits;struct A{int n;constexpr A():n(3){}operator int()const{return n;}~A(){++hits;}};'
    runner.GOOD['array_lifetime_' + name] = prefix + 'int main(){int a[]={' + expr + '};return a[0]!=3||hits!=1;}'
    runner.GOOD['template_array_lifetime_' + name] = prefix + 'template<class T>int f(){int a[]={' + expr + '};return a[0]!=3||hits!=1;}int main(){return f<int>();}'

runner.GOOD.update({
    'array_unevaluated_lifetime': 'int hits;struct A{~A(){++hits;}};int main(){int a[]={sizeof((A(),1)),true?3:(A(),5)};return hits||a[0]!=sizeof(int)||a[1]!=3;}',
    'discard_body_not_demanded': 'template<class T>struct A{A(volatile A&) noexcept{T::missing();}~A() noexcept{T::missing();}};extern volatile A<int>a;static_assert(noexcept((void)a),"");int main(){}',
    'discard_exception_substitution': 'template<class T>struct A{A(volatile A&) noexcept(sizeof(T)==1);};template<class T,bool B=noexcept(void(*static_cast<T*>(0)))>constexpr bool f(){return B;}static_assert(f<volatile A<char>>(),"");static_assert(!f<volatile A<int>>(),"");int main(){}',
    'discard_default_conversion': 'struct D{operator int();};D get() noexcept;struct A{A(volatile A&,int=get()) noexcept;};extern volatile A a;static_assert(!noexcept((void)a),"");template<class T,bool B=noexcept(void(*static_cast<T*>(0)))>constexpr bool f(){return B;}static_assert(!f<volatile A>(),"");int main(){}',
    'array_list_temporary': 'int hits;struct A{~A(){++hits;}};constexpr int f(const A&){return 3;}int main(){int a[]={f({})};return a[0]!=3||hits!=1;}',
    'array_converting_temporary': 'int hits;struct A{constexpr A(int){}~A(){++hits;}};constexpr int f(const A&){return 3;}int main(){int a[]={f(0)};return a[0]!=3||hits!=1;}',
})
runner.BAD.update({
    'nonliteral_temporary_constant': 'struct A{~A(){}};constexpr int x=(A(),3);int main(){}',
    'nonliteral_template_constant': 'struct A{~A(){}};template<class T>int f(){constexpr int x=(A(),3);return x;}int main(){return f<int>();}',
})

if __name__ == '__main__':
    sys.exit(0 if runner.run(Path(sys.argv[1]).resolve(),Path(sys.argv[2])) else 1)
