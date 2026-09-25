#!/usr/bin/env python3
"""Accumulated audit controls: CC WORK; validate and execute every accepted input."""
from pathlib import Path
import sys
import ordering_controls as r
S='template<class A,class B>struct Same{static const bool value=false;};template<class A>struct Same<A,A>{static const bool value=true;};'
D='template<class T>T&&dv();template<class...>using Void=void;'
r.GOOD={
'alias_const_adjust':S+'template<class T>using C=const T;template<class T>int f(C<T>){return 3;}template<class U>int f(U);int main(){return f(1)!=3;}',
'alias_ref_collapse':S+'template<class T>using R=T&;template<class T>R<T>&&f(T&x){return x;}int main(){int x=1;f(x)=3;return x!=3;}',
'using_result_distinct':'struct B{template<class T>char f(T){return 1;}};struct D:B{using B::f;template<class T>long f(T){return 2;}};int main(){D d;return d.f(0)!=2;}',
'prototype_noexcept_this':'struct X{int g()const noexcept{return 3;}auto f()const noexcept(noexcept(this->g()))->int{return g();}};int main(){X x;static_assert(noexcept(x.f()),"");return x.f()!=3;}',
'prototype_noexcept_implicit':'struct X{int g()const noexcept{return 3;}auto f()const noexcept(noexcept(g()))->int{return g();}};int main(){X x;static_assert(noexcept(x.f()),"");return x.f()!=3;}',
'parenthesized_member_query':S+'struct X{char g()const;auto f()const->decltype((g)()){return 1;}};int main(){X x;return sizeof(x.f())!=1;}',
'parenthesized_template_member_query':S+'template<class T>struct X{char g()const;template<class U>auto f(U)const->decltype((g)()){return 1;}};int main(){X<int>x;return sizeof(x.f(0))!=1;}',
'using_converted_pointer':D+'struct B{protected:operator int*(){return 0;}};class X:private B{public:using B::operator int*;};template<class T>auto f(T x)->decltype(static_cast<int*>(x)){return x;}int main(){X x;return f(x)!=nullptr;}',
'using_surrogate':D+'int f(int x){return x;}struct B{using P=int(*)(int);protected:operator P(){return f;}};class X:private B{public:using B::operator P;};int main(){X x;return __builtin_invoke(x,3)!=3;}',
'indirect_abstract_argument':D+'struct A{virtual void f()=0;};using P=void(*)(A);template<class F,class T,class=void>struct H{static const bool v=false;};template<class F,class T>struct H<F,T,Void<decltype(dv<F>()(dv<T>()))>>{static const bool v=true;};static_assert(!H<P,A&>::v,"");int main(){}',
'function_ref_parentheses':D+'constexpr int f(int n){return n+1;}template<class T>constexpr int call(T&&t){return (t)(3);}static_assert(call(f)==4,"");int main(){}',
'fixed_invoke_parentheses':'int f(int n){return n+1;}template<class T>int call(T,int(&p)(int)){return __builtin_invoke((p),3);}int main(){return call(0,f)!=4||call(0L,f)!=4;}',
'query_parenthesized_function':D+'constexpr int f(int n){return n+1;}template<class T>using R=decltype((f)(dv<T>()));int main(){return sizeof(R<int>)!=sizeof(int);}',
'correlated_empty':S+'template<class...>struct L{};template<class A,class B>struct P{};template<class...T>struct X{template<class...U>using Z=L<P<T,U>...>;};static_assert(Same<X<>::Z<>,L<>>::value,"");int main(){}',
'correlated_reversed':S+'template<class...>struct L{};template<class A,class B>struct P{};template<class...T>struct X{template<class...U>using Z=L<P<U,T>...>;};static_assert(Same<X<int,long>::Z<char,short>,L<P<char,int>,P<short,long>>>::value,"");int main(){}',
'alias_cv_pointer_fn':'template<class T>using F=T(int);template<class T>int call(F<T>*const p){return p(3);}int f(int n){return n;}int main(){return call(f)!=3;}',
'alias_outer_nonpack':S+'template<class T>struct X{template<class U>using A=T;template<class U>A<U>f(U){return 3;}};int main(){X<long>x;static_assert(Same<decltype(x.f(0)),long>::value,"");return x.f(0)!=3;}',
}
r.BAD={
'void_parameter_alias':'template<class T>using I=T;template<class T>int f(I<T>){return 0;}int main(){return f<void>();}',
'using_private_fn':'struct B{template<class T>int f(T){return 1;}};class D:B{using B::f;};template<class T>int call(T&t){return t.f(0);}int main(){D d;return call(d);}',
'alias_invalid_result':'template<class T>using A=T[2];template<class T>A<T>f();int main(){f<int>();}',
}

r.GOOD.update({
 'parenthesized_explicit_member':S+'struct X{template<class T>T g(T x)const{return x;}template<class U>auto f(U x)const->decltype((g<U>)(x)){return (g<U>)(x);}};int main(){X x;return x.f(3)!=3||x.f(4L)!=4;}',
 'parenthesized_object_member':D+'struct X{char f(int)const{return 3;}};template<class T>auto f(T&x)->decltype((x.f)(1)){return (x.f)(1);}int main(){X x;return f(x)!=3;}',
 'parenthesized_constexpr_member':D+'struct X{constexpr int f(int n)const{return n+1;}};template<int>struct Tag{};template<class T>using A=Tag<(T().f)(3)>;int main(){return sizeof(A<X>)!=1;}',
 'parenthesized_no_adl':D+'char g(...);namespace N{struct X{};long g(X);}template<class T>auto f(T x)->decltype((g)(x));int main(){return sizeof(f(N::X()))!=1;}',
 'parenthesized_prototype_cv':S+'struct X{char g()&;long g()const&;template<class T>auto f(T)const&->decltype((g)()){return 3;}};int main(){X x;static_assert(Same<decltype(x.f(0)),long>::value,"");return x.f(0)!=3;}',
 'noexcept_class_member':'template<class T>struct X{int g(T)const noexcept{return 3;}template<class U>auto f(U x)const noexcept(noexcept(this->g(x)))->int{return g(x);}};int main(){X<int>x;X<long>y;static_assert(noexcept(x.f(0))&&noexcept(y.f(0L)),"");return x.f(0)!=3||y.f(0L)!=3;}',
 'noexcept_member_only':'struct X{int g()const noexcept{return 3;}int f()const noexcept(noexcept(this->g())){return g();}};int main(){X x;static_assert(noexcept(x.f()),"");return x.f()!=3;}',
 'noexcept_later_member':'struct X{int f()const noexcept(noexcept(this->g())){return g();}int g()const noexcept{return 3;}};int main(){X x;static_assert(noexcept(x.f()),"");return x.f()!=3;}',
 'noexcept_cv':'struct X{int g()noexcept;int g()const;int f()const noexcept(noexcept(g())){return 3;}};int main(){X x;static_assert(!noexcept(x.f()),"");return x.f()!=3;}',
 'noexcept_pointer_parameter':'struct X{int f(int*p)const noexcept(sizeof(p)==sizeof(int*)){return *p;}};int main(){X x;int n=3;static_assert(noexcept(x.f(&n)),"");return x.f(&n)!=3;}',
 'noexcept_static':'struct X{static int g()noexcept{return 3;}static int f()noexcept(noexcept(g())){return g();}};int main(){static_assert(noexcept(X::f()),"");return X::f()!=3;}',
 'noexcept_outside':'struct X{int g()const noexcept{return 3;}int f()const noexcept(noexcept(this->g()));};int X::f()const noexcept(noexcept(this->g())){return g();}int main(){X x;static_assert(noexcept(x.f()),"");return x.f()!=3;}',
 'using_conversion_effects':'int n;struct B{protected:operator int(){++n;return 3;}};class X:private B{public:using B::operator int;};template<class T>int f(T&x){return x;}int main(){X x;return f(x)!=3||f(x)!=3||n!=2;}',
 'using_conversion_query':D+'struct B{protected:operator int();};class X:private B{public:using B::operator int;};int call(int);template<class T,class=void>struct H{static const bool v=false;};template<class T>struct H<T,Void<decltype(call(dv<T>()))>>{static const bool v=true;};static_assert(H<X>::v,"");int main(){}',
 'using_conversion_inaccessible_query':D+'struct B{operator int();};class X:private B{};int call(int);template<class T,class=void>struct H{static const bool v=false;};template<class T>struct H<T,Void<decltype(call(dv<T>()))>>{static const bool v=true;};static_assert(!H<X>::v,"");int main(){}',
 'using_conversion_public_template':'template<class T>struct B{protected:operator T(){return 3;}};template<class T>class X:private B<T>{public:using B<T>::operator T;};int main(){X<int>x;X<long>y;int a=x;long b=y;return a!=3||b!=3;}',
 'abstract_parameter_reference':D+'struct A{virtual void f()=0;};using P=void(*)(A&);template<class F,class T,class=void>struct H{static const bool v=false;};template<class F,class T>struct H<F,T,Void<decltype(dv<F>()(dv<T>()))>>{static const bool v=true;};static_assert(H<P,A&>::v,"");int main(){}',
 'abstract_surrogate':D+'struct A{virtual void f()=0;};struct F{using P=void(*)(A);operator P();};template<class F,class T,class=void>struct H{static const bool v=false;};template<class F,class T>struct H<F,T,Void<decltype(dv<F>()(dv<T>()))>>{static const bool v=true;};static_assert(!H<F,A&>::v,"");int main(){}',
 'abstract_return_decltype':S+D+'struct A{virtual void f()=0;};using P=A(*)();template<class F>using R=decltype(dv<F>()());static_assert(Same<R<P>,A>::value,"");int main(){}',
})
r.BAD.update({
 'noexcept_static_this':'struct X{static int f()noexcept(sizeof(this)>0);};int main(){}',
 'noexcept_static_member_call':'struct X{int g()noexcept;static int f()noexcept(noexcept(g()));};int main(){}',
 'using_conversion_template_specialization':'struct B{template<class T>operator T();};struct X:B{using B::operator int;};int main(){}',
 'using_private_conversion':'class B{operator int();};struct X:B{using B::operator int;};int main(){}',
 'private_base_conversion':'struct B{operator int(){return 3;}};class X:private B{};int main(){X x;int n=x;return n;}',
 'deleted_exposed_conversion':'struct B{operator int()=delete;};struct X:B{using B::operator int;};int main(){X x;int n=x;return n;}',
 'abstract_indirect_call':'struct A{virtual void f()=0;};void test(void(*p)(A),A&a){p(a);}int main(){}',
 'abstract_surrogate_call':'struct A{virtual void f()=0;};struct F{using P=void(*)(A);operator P();};void test(F&f,A&a){f(a);}int main(){}',
})
r.GOOD.update({
 'exception_pack':'int g(int,int)noexcept;template<class...T>int f(T...t)noexcept(noexcept(g(t...))){return 3;}int main(){static_assert(noexcept(f(1,2)),"");return f(1,2)!=3;}',
 'exception_pack_dormant':'template<class...>struct List{};template<class...T>int f(T...t)noexcept(sizeof...(t)==2){return sizeof(typename List<T...>::missing);}int main(){static_assert(noexcept(f(1,2))&&!noexcept(f(1)),"");}',
 'exception_member_pack':'template<int N>struct X{static int g(int,int)noexcept;template<class...T>int f(T...t)const noexcept(noexcept(this->g(t...))){return N;}};int main(){X<3>x;X<4>y;static_assert(noexcept(x.f(1,2))&&noexcept(y.f(1,2)),"");return x.f(1,2)!=3||y.f(1,2)!=4;}',
 'exception_outer_pack':'template<class...A>struct X{template<class...T>int f(T...t)const noexcept(sizeof...(A)==sizeof...(t)){return 3;}};int main(){X<int,long>x;X<char>y;static_assert(noexcept(x.f(1,2))&&!noexcept(y.f(1,2)),"");return x.f(1,2)!=3;}',
 'exception_empty_pack':'template<class...T>int f(T...t)noexcept(sizeof...(t)==0){return 3;}int main(){static_assert(noexcept(f())&&!noexcept(f(1)),"");return f()!=3;}',
 'exception_raw_cv':S+'template<class T>int f(const T t)noexcept(Same<decltype(t),const T>::value){return t;}int main(){static_assert(noexcept(f(3)),"");return f(3)!=3;}',
})
r.GOOD.update({
 'namespace_using_distinct_return':'namespace N{template<class T>char f(T){return 1;}}using N::f;template<class T>long f(T){return 2;}int main(){char(*p)(int)=f;long(*q)(int)=f;return p(0)!=1||q(0)!=2;}',
 'namespace_using_distinct_alias_return':'template<class T>using I=T;namespace N{template<class T>I<char>f(T){return 1;}}using N::f;template<class U>I<long>f(U){return 2;}int main(){char(*p)(int)=f;long(*q)(int)=f;return p(0)!=1||q(0)!=2;}',
})
r.BAD.update({
 'namespace_using_same_template':'namespace N{template<class T>int f(T);}using N::f;template<class U>int f(U);int main(){}',
 'namespace_using_same_plain':'namespace N{int f(int);}using N::f;long f(int);int main(){}',
})
r.GOOD.update({
 'exception_ordinary_raw_cv':S+'int f(const int t)noexcept(Same<decltype(t),const int>::value){return t;}int main(){static_assert(noexcept(f(3)),"");return f(3)!=3;}',
 'exception_ordinary_pointer_cv':S+'struct X{int f(int*const p)const noexcept(Same<decltype(p),int*const>::value){return *p;}};int main(){X x;int n=3;static_assert(noexcept(x.f(&n)),"");return x.f(&n)!=3;}',
 'exception_ordinary_array_adjust':S+'int f(const int p[2])noexcept(Same<decltype(p),const int*>::value){return p[0];}int main(){int n[2]={3,4};static_assert(noexcept(f(n)),"");return f(n)!=3;}',
})
sys.exit(0 if r.run(Path(sys.argv[1]).resolve(),Path(sys.argv[2])) else 1)
