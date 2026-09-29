#!/usr/bin/env python3
"""PA19 retained type/declaration composition. Run CC WORK; execute successes."""
from pathlib import Path
import sys
sys.path.insert(0, str(Path(__file__).resolve().parents[1] / 'pa18'))
import ordering_controls as runner

runner.GOOD = {
    'function_ellipsis': 'template<class>struct P{static const int n=0;};template<class R>struct P<R(*)()>{static const int n=1;};template<class R>struct P<R(*)(...)>{static const int n=2;};static_assert(P<void(...)>::n==0,"");static_assert(P<void(*)(...)>::n==2,"");static_assert(P<void()>::n==0,"");static_assert(P<void(*)()>::n==1,"");int main(){}',
    'function_reference': 'template<class>struct P;template<class R,class...A>struct P<R(A...)>{static const int n=sizeof...(A);};template<class T>using F=T(int,long);static_assert(P<F<char>>::n==2,"");int main(){}',
    'function_type_value_disambiguation': 'template<int>struct V{};template<class>struct P{};V<int(3)> v;P<void(...)> p;int main(){}',
    'friend_template_scope': 'template<class>struct Friend;template<class T>struct X{friend class Friend<T>;typedef Friend<T> type;};template<class T>struct Friend{};X<int>::type a;X<long>::type b;int main(){}',
    'friend_injected_scope': 'template<class>struct Ref;template<class T>struct X{friend class Ref<X>;typedef Ref<X> type;};template<class T>struct Ref{typedef typename T::type type;};X<int>::type a;int main(){}',
    'comma_optional_ellipsis': 'template<class T>struct X{static char f(T ...);};static_assert(sizeof(X<int>::f(1,2,3))==1,"");int main(){}',
    'ellipsis_and_pack': 'template<class...T>struct X{static char f(T...);static long g(int ...);};static_assert(sizeof(X<int,long>::f(1,2))==1,"");static_assert(sizeof(X<int>::g(1,2))==sizeof(long),"");int main(){}',
    'member_alias_identity': 'template<class>struct N{};template<template<class...>class G>using A=G<char>;template<template<class...>class F,class...>using B=A<F>;template<class>struct O{template<class T>using M=N<T>;using R=B<M>;};template<class,class>struct Same;template<class T>struct Same<T,T>{};Same<O<int>::R,N<char>> a;Same<O<long>::R,N<char>> b;int main(){}',
    'member_alias_outer_argument': 'template<class,class>struct Pair{};template<template<class...>class F>using Apply=F<char>;template<class T>struct O{template<class U>using M=Pair<T,U>;using R=Apply<M>;};template<class,class>struct Same;template<class T>struct Same<T,T>{};Same<O<int>::R,Pair<int,char>> a;Same<O<long>::R,Pair<long,char>> b;int main(){}',
    'member_function_type': 'template<class T>T&& val();template<class>struct Result;template<class F,class...A>struct Result<F(A...)>{typedef decltype(val<F>()(val<A>()...)) type;};struct H{long operator()(int=0);};template<class T>struct B{template<class...A>typename Result<T(A...)>::type operator()(A&&...){return 7;}};int main(){B<H>b;return b()!=7||b(2)!=7;}',
    'member_function_type_dormant': 'template<class T>T&& val();template<class>struct Result;template<class F,class...A>struct Result<F(A...)>{typedef decltype(val<F>()(val<A>()...)) type;};template<class T>struct B{template<class...A>typename Result<T(A...)>::type operator()(A&&...);};struct H{};int main(){B<H>b;}',
    'function_type_nested_argument': 'template<class>struct N{};template<class>struct Fn;template<class R,class A>struct Fn<R(A)>{typedef A type;};template<class T>struct Outer{typedef typename Fn<N<T>(long)>::type type;};static_assert(sizeof(Outer<int>::type)==sizeof(long),"");int main(){}',
    'fixed_head_pack_defaults': 'template<class A=int,class B=long,class C=char>struct L{};template<class A,class...T>int f(L<A,T...>){return sizeof...(T);}int main(){return f(L<>())!=2||f(L<int>())!=2||f(L<int,long,char>())!=2;}',
    'fixed_head_pack_nondefault': 'template<class A,class B,class C>struct L{};template<class A,class...T>int f(L<A,T...>){return sizeof...(T);}int main(){return f(L<int,long,char>())!=2;}',
    'fixed_head_pack_base': 'template<class A=int,class B=long,class C=char>struct L{};struct D:L<>{};template<class A,class...T>int f(const L<A,T...>&){return sizeof...(T);}int main(){D d;return f(d)!=2;}',
    'fixed_value_head_pack': 'template<int A=1,int B=2,int C=3>struct L{};template<int A,int...N>int f(L<A,N...>){return A+sizeof...(N);}int main(){return f(L<>())!=3||f(L<5>())!=7;}',
    'cast_preserves_object': 'int calls;struct X{int n;template<class T>operator T(){++calls;return T();}};template<class T>T&& f(T&x){return static_cast<T&&>(x);}int main(){X x;x.n=7;X&&r=f(x);r.n=9;return &r!=&x||x.n!=9||calls;}',
    'cast_preserves_base': 'int calls;struct B{int n;};struct D:B{template<class T>operator T(){++calls;return T();}};template<class T>B&& f(T&x){return static_cast<B&&>(x);}int main(){D d;d.n=7;B&&b=f(d);b.n=9;return d.n!=9||calls;}',
    'cast_unrelated_conversion': 'int calls;struct B{int n;};struct X{operator B(){++calls;B b;b.n=7;return b;}};int main(){X x;B&&b=static_cast<B&&>(x);return b.n!=7||calls!=1;}',
}
runner.BAD = {
    'raw_function_not_pointer': 'template<class>struct P;template<class R>struct P<R(*)()>{};P<void()> p;',
    'variadic_not_nonvariadic': 'template<class>struct P;template<class R>struct P<R(*)()>{};P<void(*)(...)> p;',
    'fixed_head_inconsistent_pack': 'template<class A=int,class B=long>struct L{};template<class...T>void f(L<T...>,L<T...>);int main(){f(L<>(),L<int,char>());}',
    'member_alias_kind_mismatch': 'template<template<int>class F>using A=F<1>;template<class>struct O{template<class T>using M=T;using R=A<M>;};O<int> o;',
    'cast_cannot_drop_const': 'struct X{template<class T>operator T(){return T();}};int main(){const X x{};X&&r=static_cast<X&&>(x);}',
    'cast_ambiguous_base': 'struct B{};struct L:B{};struct R:B{};struct D:L,R{};int main(){D d;B&&b=static_cast<B&&>(d);}',
}

if __name__ == '__main__':
    sys.exit(0 if runner.run(Path(sys.argv[1]).resolve(), Path(sys.argv[2])) else 1)
