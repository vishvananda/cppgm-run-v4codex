#!/usr/bin/env python3
"""Source-name obligations: N3485 [temp.names], [temp.res], [temp.dep.type]."""
from pathlib import Path
import sys
import entity_controls as harness

harness.GOOD = {
 'qualified_type': 'struct P{typedef int type;};template<class T>int f(){typename T::type x=7;return x;}int main(){return f<P>()-7;}',
 'current_primary': 'template<class T>struct A{typedef T type;A<T>::type n;};int main(){A<int>a={7};return a.n-7;}',
 'current_rooted': 'template<class T>struct A{typedef T type;::A<T>::type n;};int main(){A<int>a={7};return a.n-7;}',
 'current_alias': 'template<class T>struct A;template<class T>using Id=A<T>;template<class T>struct A{typedef T type;Id<T>::type n;};int main(){A<int>a={7};return a.n-7;}',
 'current_partial': 'template<class T>struct A;template<class T>struct A<T*>{typedef T type;A<T*>::type n;};int main(){A<int*>a={7};return a.n-7;}',
 'current_partial_renamed_definition': 'template<class T>struct A;template<class T>struct A<T*>{typedef T type;int f(type);};template<class U>int A<U*>::f(A<U*>::type n){return n;}int main(){A<int*>a;return a.f(7)-7;}',
 'current_partial_pack_definition': 'template<class...T>struct L{};template<class>struct A;template<class...T>struct A<L<T...>>{typedef int type;int f(type);};template<class...U>int A<L<U...>>::f(A<L<U...>>::type n){return n;}int main(){A<L<int,char>>a;return a.f(7)-7;}',
 'current_nested': 'template<class T>struct A{typedef T type;struct B{typedef T inner;A<T>::B::inner n;A<T>::type m;};};int main(){A<int>::B b={3,4};return b.n+b.m-7;}',
 'fixed_base': 'struct B{typedef int type;};template<class T>struct A:B{A<T>::type n;};int main(){A<int>a;a.n=7;return a.n-7;}',
 'dependent_base_explicit': 'struct B{typedef int type;};template<class T>struct A:T{typename A<T>::type n;};int main(){A<B>a;a.n=7;return a.n-7;}',
 'current_type_parameter_alias': 'template<class T>struct A{typedef T U;typedef int type;A<U>::type n;};int main(){A<char>a={7};return a.n-7;}',
 'current_value_parameter': 'template<int N>struct A{typedef int type;A<N>::type n;};int main(){A<3>a={7};return a.n-7;}',
 'current_value_alias': 'template<int N>struct A{static const int I=N;static const int J=I;typedef int type;A<J>::type n;};int main(){A<3>a={7};return a.n-7;}',
 'current_pack': 'template<class...T>struct A{typedef int type;A<T...>::type n;};int main(){A<int,char>a={7};return a.n-7;}',
 'noncurrent_explicit': 'template<class T>struct A{typedef int type;template<class U>static int f(){typename A<U>::type n=7;return n;}};int main(){return A<char>::f<int>()-7;}',
 'return_parameter_boundary': 'template<class T>struct A{typedef int type;static type f(type);};template<class U>typename A<U>::type A<U>::f(A<U>::type n){return n;}int main(){return A<int>::f(7)-7;}',
 'trailing_return': 'template<class T>struct A{typedef int type;static type f();};template<class U>auto A<U>::f()->A<U>::type{return 7;}int main(){return A<int>::f()-7;}',
 'member_template_type': 'struct P{template<class T>struct Rebind{typedef T type;};};template<class T>int f(){typename T::template Rebind<int>::type n=7;return n;}int main(){return f<P>()-7;}',
 'template_template_argument': 'struct P{template<class T>struct Rebind{T n;};};template<template<class>class C>struct Use{C<int> n;};template<class T>int f(){Use<T::template Rebind> a={{7}};return a.n.n;}int main(){return f<P>()-7;}',
 'current_member_template': 'template<class T>struct A{template<class U>struct B{U n;};A<T>::B<int> n;};int main(){A<char>a={{7}};return a.n.n-7;}',
 'dependent_dot': 'struct P{template<class T>T f(T n){return n;}};template<class T>int g(T&x){return x.template f<int>(7);}int main(){P p;return g(p)-7;}',
 'dependent_arrow': 'struct P{template<class T>T f(T n){return n;}};template<class T>int g(T*x){return x->template f<int>(7);}int main(){P p;return g(&p)-7;}',
 'fixed_receiver': 'struct P{template<class T>T f(T n){return n;}};template<class T>int g(T n){P p;return p.f<int>(n);}int main(){return g(7)-7;}',
 'current_this': 'template<class T>struct A{template<class U>int f(U n){return n;}int g(){return this->f<int>(7);}};int main(){A<char>a;return a.g()-7;}',
 'current_receiver': 'template<class T>struct A{template<class U>int f(U n){return n;}int g(A<T>&a){return a.f<int>(7);}};int main(){A<char>a;return a.g(a)-7;}',
 'dependent_base_member_call': 'template<class T>struct B{template<class U>int f(U n){return n;}};template<class T>struct A:B<T>{int g(){return this->B<T>::template f<int>(7);}};int main(){A<char>a;return a.g()-7;}',
 'current_qualified_member_call': 'template<class T>struct A{template<class U>int f(U n){return n;}int g(){return this->A<T>::f<int>(7);}};int main(){A<char>a;return a.g()-7;}',
 'qualified_call': 'struct P{template<class T>static T f(T n){return n;}};template<class T>int g(){return T::template f<int>(7);}int main(){return g<P>()-7;}',
 'pack_type': 'template<unsigned long N>struct P{typedef int type;};template<class...T>int f(){typename P<sizeof...(T)>::type n=7;return n;}int main(){return f<int,char>()-7;}',
 'base_type_context': 'struct P{struct Base{int n;};};template<class T>struct A:T::Base{};int main(){A<P>a;a.n=7;return a.n-7;}',
 'elaborated_type_context': 'struct P{struct B{int n;};};template<class T>int f(){struct T::B b={7};return b.n;}int main(){return f<P>()-7;}',
 'direct_initializer_value': 'template<class T>struct A{static const int n=7;};template<class T>int f(){int n(A<T>::n);return n;}int main(){return f<int>()-7;}',
 'direct_initializer_specialized_value': 'template<class T>struct A{typedef int type;};template<class T>int f(){int n(A<T>::type);return n;}template<>struct A<int>{static const int type=7;};int main(){return f<int>()-7;}',
 'direct_initializer_current_function': 'template<class T>struct A{typedef T type;int f(A<T>::type);};template<class T>int A<T>::f(T n){return n;}int main(){A<int>a;return a.f(7)-7;}',
 'varargs_shape': 'template<class>struct F;template<class R,class A>struct F<R(A...)>{static const int n=7;};int main(){return F<int(char...)>::n-7;}',
}
harness.BAD = {
 'dormant_type': 'template<class T>struct A{typedef T::type bad;};',
 'dormant_member_template_type': 'template<class T>struct A{typedef T::template Rebind<int> bad;};',
 'type_missing_template': 'template<class T>struct A{typedef typename T::Rebind<int>::type bad;};',
 'noncurrent_body': 'template<class T>struct A{typedef int type;template<class U>static void f(){A<U>::type n;}};',
 'noncurrent_partial': 'template<class T>struct A{typedef int type;};template<class T>struct A<T*>{A<T>::type n;};',
 'value_expression_not_current': 'template<int N>struct A{typedef int type;A<N+0>::type n;};',
 'value_alias_expression_not_current': 'template<int N>struct A{static const int I=N+0;typedef int type;A<I>::type n;};',
 'pack_expression': 'template<unsigned long N>struct P{typedef int type;};template<class...T>struct A{typedef P<sizeof...(T)>::type bad;};',
 'leading_return': 'template<class T>struct A{typedef int type;static type f();};template<class T>A<T>::type A<T>::f(){return 0;}',
 'leading_return_member_template': 'template<class T>struct A{typedef int type;template<class U>static type f();};template<class T>template<class U>A<T>::type A<T>::f(){return 0;}',
 'dependent_base_unknown': 'template<class T>struct A:T{A<T>::type n;};',
 'dependent_dot_missing': 'template<class T>int g(T&x){return x.f<int>(7);}',
 'dependent_arrow_missing': 'template<class T>int g(T*x){return x->f<int>(7);}',
 'dependent_qualified_missing': 'template<class T>int g(){return T::f<int>(7);}',
 'dependent_qualified_chain_missing': 'template<class T>int g(){return T::template B<int>::f<int>(7);}',
 'dependent_base_member_missing': 'template<class T>struct B{template<class U>int f(U n){return n;}};template<class T>struct A:B<T>{int g(){return this->B<T>::f<int>(7);}};',
 'noncurrent_receiver_missing': 'template<class T>struct A{template<class U>int f(U n){return n;}};template<class T>int g(A<T>&a){return a.f<int>(7);}',
 'typename_does_not_override_value': 'struct P{static int type;};template<class T>void f(){typename T::type n;}int main(){f<P>();}',
 'dependent_direct_initializer_type': 'template<class T>struct A{typedef int type;};template<class T>int f(){int n(A<T>::type);return n;}int main(){return f<int>();}',
}
if __name__ == '__main__':
 cc=Path(sys.argv[1]).resolve() if len(sys.argv)>1 else harness.ROOT/'dev/cppgm++'
 work=Path(sys.argv[2]) if len(sys.argv)>2 else Path('/tmp/pa17-name-controls')
 sys.exit(0 if harness.run(cc,work) else 1)
