#!/usr/bin/env python3
"""Dependent type/template category, candidate failure, and handoff interactions.

Proof: N3485 [temp.res]/3-4; [temp.arg.template]/1;
[temp.class.spec.match]/1-2 and [temp.deduct]/8. Course extensions are exercised
only as neighbors of the same canonical lookup/substitution owners.
"""
from pathlib import Path
import sys
import entity_controls as runner

runner.GOOD = {
 'type_member_pointer': 'struct X{using A=int;};template<class T>typename T::A* f(){return 0;}int main(){return f<X>()!=0;}',
 'applied_alias_pointer': 'struct X{template<class U>using A=U;};template<class T>typename T::template A<int>* f(){return 0;}int main(){return f<X>()!=0;}',
 'applied_class_pointer': 'struct X{template<class U>struct A{};};template<class T>typename T::template A<int>* f(){return 0;}int main(){return f<X>()!=0;}',
 'alias_template_argument': 'struct X{template<class U>using A=U;};template<template<class>class F>struct S{using type=F<int>;};template<class T>using V=typename S<T::template A>::type;int main(){V<X>x=7;return x-7;}',
 'class_template_argument': 'struct X{template<class U>struct A{static const int n=sizeof(U);};};template<template<class>class F>struct S{static const int n=F<int>::n;};template<class T>int f(){return S<T::template A>::n;}int main(){return f<X>()-4;}',
 'inherited_template_argument': 'template<class T>struct B{template<class U>using A=U;};template<class T>struct D:B<T>{};template<template<class>class F>struct S{using type=F<int>;};template<class T>using V=typename S<T::template A>::type;int main(){V<D<char>>x=7;return x-7;}',
 'partial_nonclass_qualifier': 'template<class T,class=void>struct S{static const int n=0;};template<class T>struct S<T,typename T::type>{static const int n=1;};struct X{using type=void;};static_assert(S<int>::n==0 && S<X>::n==1,"candidate");int main(){return 0;}',
 'partial_nontemplate_member': 'template<class T,class=void>struct S{static const int n=0;};template<class T>struct S<T,typename T::template F<int>>{static const int n=1;};struct X{using F=int;};struct Y{template<class>using F=void;};static_assert(S<X>::n==0 && S<Y>::n==1,"candidate");int main(){return 0;}',
 'partial_template_as_type': 'template<class T,class=void>struct S{static const int n=0;};template<class T>struct S<T,typename T::F>{static const int n=1;};struct X{template<class>using F=int;};struct Y{using F=void;};static_assert(S<X>::n==0 && S<Y>::n==1,"candidate");int main(){return 0;}',
 'partial_const_failure': 'template<bool,class=void>struct E{};template<class T>struct E<true,T>{using type=T;};template<class T>using A=const typename E<(sizeof(T)>1),T>::type;template<class T,class=void>struct S{static const int n=0;};template<class T>struct S<T,A<T>>{static const int n=1;};static_assert(S<char,const int>::n==0 && S<int,const int>::n==1,"failed type has no cv");int main(){return 0;}',
 'partial_volatile_failure': 'template<bool,class=void>struct E{};template<class T>struct E<true,T>{using type=T;};template<class T>using A=volatile typename E<(sizeof(T)>1),T>::type;template<class T,class=void>struct S{static const int n=0;};template<class T>struct S<T,A<T>>{static const int n=1;};static_assert(S<char,volatile int>::n==0 && S<int,volatile int>::n==1,"failed type has no cv");int main(){return 0;}',
 'partial_cv_direct_failure': 'template<class T,class=void>struct S{static const int n=0;};template<class T>struct S<T,const volatile typename T::type>{static const int n=1;};struct X{};struct Y{using type=int;};static_assert(S<X,const volatile int>::n==0 && S<Y,const volatile int>::n==1,"failed type");int main(){return 0;}',
 'partial_repeated_failure': 'template<class T>using A=const typename T::type;template<class T,class=void>struct S{static const int n=0;};template<class T>struct S<T,A<T>>{static const int n=1;};template<class T,class=void>struct Q{static const int n=0;};template<class T>struct Q<T,A<T>>{static const int n=2;};struct X{};struct Y{using type=int;};static_assert(S<X,const int>::n==0 && Q<X,const int>::n==0 && S<Y,const int>::n==1 && Q<Y,const int>::n==2,"cached failure");int main(){return 0;}',
 'fixed_pack_suffix': 'template<class A,class B>struct S{using type=B;};template<class...T>using X=typename S<T...,int>::type;int main(){X<char>x=7;return x-7;}',
 'fixed_pack_defaults': 'template<class A=int,class B=long>struct S{using type=B;};template<class...T>using X=typename S<T...>::type;static_assert(sizeof(X<>)==8 && sizeof(X<char,int>)==4,"defaults");int main(){return 0;}',
 'conditional_empty_pack': 'struct X{template<class...T>explicit(sizeof...(T)>0)X(T...){}};int main(){X x={};return 0;}',
 'conditional_candidate_filter': 'struct X{int n;template<class T>explicit(sizeof(T)>4)X(T):n(1){} X(int):n(2){}};int f(X x){return x.n;}int main(){return f(3L)-2;}',
 'variable_member_partial_outer': 'template<class X>struct O{template<class T>static const int v=sizeof(X);template<class T>static const int v<T*> =sizeof(X)+sizeof(T);};static_assert(O<char>::v<int*> ==5,"outer");int main(){return 0;}',
}
runner.BAD = {
 'bare_alias_pointer': 'struct X{template<class>using A=int;};template<class T>typename T::A* f(){return 0;}int main(){return f<X>()!=0;}',
 'bare_class_pointer': 'struct X{template<class>struct A{};};template<class T>typename T::A* f(){return 0;}int main(){return f<X>()!=0;}',
 'bare_alias_typedef': 'struct X{template<class>using A=int;};template<class T>struct S{using type=typename T::A;};using Bad=S<X>::type;int main(){return 0;}',
 'bare_alias_const_pointer': 'struct X{template<class>using A=int;};template<class T>const typename T::A* f(){return 0;}int main(){return f<X>()!=0;}',
 'ordinary_type_as_dependent_template': 'struct X{using A=int;};template<template<class>class F>struct S{using type=F<int>;};template<class T>using V=typename S<T::template A>::type;V<X>x;',
 'hard_missing_const_type': 'struct X{};template<class T>using A=const typename T::type;A<X>*p;',
 'hard_nonclass_qualifier': 'template<class T>using A=typename T::type;A<int>*p;',
 'template_use_then_type_use': 'struct X{template<class U>using A=U;};template<template<class>class F>struct S{using type=F<int>;};template<class T>struct Q{using good=typename S<T::template A>::type;using bad=typename T::A;};Q<X>q;',
}
if __name__ == '__main__':
 sys.exit(0 if runner.run(Path(sys.argv[1]).resolve(),Path(sys.argv[2])) else 1)
