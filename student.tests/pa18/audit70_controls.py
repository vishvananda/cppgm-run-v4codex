#!/usr/bin/env python3
"""Audit 70 operator selection and retained receiver effects: CC WORK.
N3485 [over.built]/18,21,22, [expr.ass]/7, [expr.unary.noexcept]/3.
Results check overload participation, cv/ref identity, single evaluation and
observable values; every accepted case validates and executes its LowIR.
"""
from pathlib import Path
import sys
import ordering_controls as runner
runner.GOOD = {
    'volatile_ref': 'template<class A,class B>struct Same{static const bool value=false;};template<class A>struct Same<A,A>{static const bool value=true;};struct X{volatile int n;operator volatile int&(){return n;}};template<class T>auto f(T&t)->decltype(t+=1){return t+=1;}int main(){X x{3};static_assert(Same<decltype(f(x)),volatile int&>::value,"");volatile int&r=f(x);return x.n!=4||&r!=&x.n;}',
    'rank_arithmetic': 'struct A{operator double&();};int operator+=(A&,float);template<class T>T&v();template<class T,class=decltype(v<T>()+=1)>char f(int);template<class>long f(...);static_assert(sizeof(f<A>(0))==sizeof(long),"");int main(){}',
    'rank_pointer': 'int n;struct A{int*p;operator int*&(){return p;}};int operator+=(A&,long){return ++n;}template<class T>auto f(T&t)->decltype(t+=1){return t+=1;}int main(){A a{0};return f(a)!=1||n!=1;}',
    'arrow_scalar': 'using I=int;struct P{I*operator->();};template<class T>T&v()noexcept;template<class T>struct F{static const bool value=noexcept(v<T>()->~I());};static_assert(!F<P>::value,"");int main(){}',
    'arrow_signature': 'using I=int;struct P{I*operator->();};template<class T>T&v()noexcept;template<bool B>struct Tag{};template<class T>auto f(int)->Tag<noexcept(v<T>()->~I())>;template<class T>struct Same;template<>struct Same<Tag<false>>{static const bool value=true;};static_assert(Same<decltype(f<P>(0))>::value,"");int main(){}',
    'arrow_throws': 'struct X{};struct P{X*operator->();};template<class T>T&v()noexcept;template<class T>struct F{static const bool value=noexcept(v<T>()->~X());};static_assert(!F<P>::value,"");int main(){}',
    'arrow_nothrow': 'struct X{};struct P{X*operator->()noexcept;};template<class T>T&v()noexcept;template<class T>struct F{static const bool value=noexcept(v<T>()->~X());};static_assert(F<P>::value,"");int main(){}',
    'arrow_temporary': 'struct X{};struct Q{X*operator->()noexcept;~Q()noexcept(false);};struct P{Q operator->()noexcept;};template<class T>T&v()noexcept;template<class T>struct F{static const bool value=noexcept(v<T>()->~X());};static_assert(!F<P>::value,"");int main(){}',
    'unscoped_assign': 'enum E{e};template<class T>T&v();template<class T,class=decltype(v<T>()=v<T>())>char f(int);template<class>long f(...);static_assert(sizeof(f<E>(0))==1,"");int main(){}',
}
runner.BAD = {}

for op,initial,right,expected in [('+=',7,3,10),('-=',7,3,4),('*=',7,3,21),('/=',7,3,2),('%=',7,3,1),('&=',7,3,3),('|=',4,3,7),('^=',7,3,4),('<<=',7,2,28),('>>=',7,2,1)]:
    for cv in ('','volatile '):
        name = ('volatile_' if cv else 'plain_')+str(len(runner.GOOD))
        runner.GOOD[name]='int calls;struct X{'+cv+'int n;operator '+cv+'int&(){++calls;return n;}};template<class T>auto f(T&t)->decltype(t'+op+str(right)+'){return t'+op+str(right)+';}int main(){X x{'+str(initial)+'};'+cv+'int&r=f(x);return calls!=1||x.n!='+str(expected)+'||&r!=&x.n;}'
runner.GOOD.update({
 'rank_arithmetic_runtime':'int calls;struct A{double n;operator double&(){++calls;return n;}};template<class T>auto f(T&t)->decltype(t+=2){return t+=2;}int main(){A a{0.5};double&r=f(a);return a.n!=2.5||calls!=1||&r!=&a.n;}',
 'volatile_pointer':'int calls;struct A{int*volatile p;operator int*volatile&(){++calls;return p;}};template<class T>auto f(T&t)->decltype(t+=2){return t+=2;}int main(){int a[4];A x{a};int*volatile&r=f(x);return x.p!=a+2||&r!=&x.p||calls!=1;}',
 'ambiguous_rhs':'struct A{operator double&();};struct B{operator int();operator long();};template<class T,class U>auto f(int)->decltype(*((T*)0)+=*((U*)0),char());template<class,class>long f(...);static_assert(sizeof(f<A,B>(0))==sizeof(long),"");int main(){}',
 'const_reference_rejected':'struct A{operator const int&();};template<class T>auto f(int)->decltype(*((T*)0)+=1,char());template<class>long f(...);static_assert(sizeof(f<A>(0))==sizeof(long),"");int main(){}',
 'value_conversion_rejected':'struct A{operator int();};template<class T>auto f(int)->decltype(*((T*)0)+=1,char());template<class>long f(...);static_assert(sizeof(f<A>(0))==sizeof(long),"");int main(){}',
 'volatile_fixed_body':'int calls;struct A{volatile short n;operator volatile short&(){++calls;return n;}};template<class T>void f(A&a){a*=2.5;}int main(){A a{3};f<int>(a);return calls!=1||a.n!=7;}',
})
for label,decl,expected in [
 ('quiet','struct P{I*operator->()noexcept;};',True),
 ('throws','struct P{I*operator->();};',False),
 ('nested','struct Q{I*operator->();};struct P{Q operator->()noexcept;};',False),
 ('temporary','struct Q{I*operator->()noexcept;~Q()noexcept(false);};struct P{Q operator->()noexcept;};',False),
 ('reference','struct Q{I*operator->()noexcept;~Q()noexcept(false);};struct P{Q&operator->()noexcept;};',True),
 ('quiet_temporary','struct Q{I*operator->()noexcept;~Q()noexcept;};struct P{Q operator->()noexcept;};',True)]:
    runner.GOOD['scalar_arrow_'+label]='using I=int;'+decl+'template<class T>T&v()noexcept;template<class T>struct F{static const bool value=noexcept(v<T>()->~I());};static_assert(F<P>::value=='+str(expected).lower()+',"");int main(){}'
runner.GOOD.update({
 'scalar_arrow_runtime':'using I=int;int calls;struct P{int*n;int*operator->(){++calls;return n;}};template<class T>void f(T&p){p->~I();}int main(){int n=3;P p{&n};f(p);return calls!=1||n!=3;}',
 'scalar_arrow_temporary_runtime':'using I=int;int calls,dtors;struct Q{I*p;I*operator->(){++calls;return p;}~Q(){++dtors;}};struct P{I*p;Q operator->(){++calls;return Q{p};}};template<class T>void f(T&p){p->~I();}int main(){int n=1;P p{&n};f(p);return calls!=2||dtors!=1;}',
})
for label,decl,valid in [
 ('missing','struct P{};',False),
 ('private','class P{I*operator->();};',False),
 ('deleted','struct P{I*operator->()=delete;};',False),
 ('nonpointer','struct P{I operator->();};',False),
 ('recursive','struct P{P&operator->();};',False),
 ('deleted_temporary','struct Q{I*operator->();~Q()=delete;};struct P{Q operator->();};',False),
 ('public','struct P{I*operator->();};',True)]:
    runner.GOOD['arrow_sfinae_'+label]='using I=int;'+decl+'template<class T>T&v();template<class T,class=decltype(v<T>()->~I())>char f(int);template<class>long f(...);static_assert((sizeof(f<P>(0))==1)=='+str(valid).lower()+',"");int main(){}'
runner.GOOD['arrow_completion']='using I=int;struct P;template<class T>T&v();template<class T,class=decltype(v<T>()->~I())>char f(int);template<class>long f(...);static_assert(sizeof(f<P>(0))==sizeof(long),"");struct P{I*operator->();};static_assert(sizeof(f<P>(0))==1,"");int main(){}'
runner.GOOD['class_bool_pointer']='struct B{operator bool&();};template<class T>T&v();template<class T,class=decltype(v<T>()+=v<int*>())>char f(int);template<class>long f(...);static_assert(sizeof(f<B>(0))==sizeof(long),"");int main(){}'
runner.BAD['property_context']='class Private{~Private();friend struct Context;};struct Victim{Private p;};struct Context{static char f();};decltype(((Victim*)0)->~Victim(),char()) Context::f(){return 0;}int main(){}'
runner.GOOD['property_context_own_friend']='class Private{~Private();friend struct Victim;};struct Victim{Private p;};struct Context{static char f();};decltype(((Victim*)0)->~Victim(),char()) Context::f(){return 0;}int main(){}'
runner.BAD['property_template_context']='class Private{~Private();template<class>friend struct Context;};struct Victim{Private p;};template<class T>struct Context{static char f();};template<class T>decltype(((Victim*)0)->~Victim(),char()) Context<T>::f(){return 0;}int main(){}'
runner.GOOD['property_template_context_own_friend']='class Private{~Private();friend struct Victim;};struct Victim{Private p;};template<class T>struct Context{static char f();};template<class T>decltype(((Victim*)0)->~Victim(),char()) Context<T>::f(){return 0;}int main(){}'
if __name__=='__main__':
    sys.exit(0 if runner.run(Path(sys.argv[1]).resolve(),Path(sys.argv[2])) else 1)
