#!/usr/bin/env python3
"""Immediate-context list/cast queries and their value/effect consumers: CC WORK."""
from pathlib import Path
import sys
import ordering_controls as runner
GOOD={};BAD={}
def detect(name,decl,expression,yes,no=''):
 source=decl+'template<class T>auto probe(int)->decltype('+expression+',char());template<class>long probe(...);'
 for t in yes.split(';'):
  if t:source+='static_assert(sizeof(probe<'+t+'>(0))==1,"accepted");'
 for t in no.split(';'):
  if t:source+='static_assert(sizeof(probe<'+t+'>(0))==sizeof(long),"discarded");'
 GOOD[name]=source+'int main(){}'
detect('fractional','struct X{X(int);};','T{0.5}','double;float','int;X')
detect('integer_ranges','','T{256}','int;long;double','unsigned char;char;bool')
detect('negative_to_unsigned','','T{-1}','int;long','unsigned;unsigned long;bool')
detect('bool_one','','T{1}','bool;char;int')
detect('floating_exact','','T{1.5}','float;double','int;bool')
detect('floating_inexact','','T{0.1}','double','float')
detect('nonconstant','template<class T>T dv();','T{dv<long>()}','long','int;float;double')
detect('nested_braces','','T{{1}}','int;const int&;int&&','int&')
detect('aggregate_fields','struct A{int x;};struct B{double x;};struct C{int x;double y;};','T{0.5}','B','A;C')
detect('aggregate_excess','struct A{int x;};struct B{int x;int y;};','T{1,2}','B','A;int')
detect('brace_elision','struct A{int x[2];int y;};struct B{int x[2];};','T{1,2,3}','A','B')
detect('reference_member','struct A{const int&r;};struct B{int&r;};','T{1}','A','B')
detect('deleted_constructor','struct A{A(int)=delete;};struct B{B(int);};','T{1}','B','A')
detect('private_constructor','class A{A(int);};struct B{B(int);};','T{1}','B','A')
detect('direct_explicit','struct A{explicit A(int);};','T{1}','A')
detect('private_destructor','class A{~A();};struct B{};','T{}','B','A')
detect('deleted_destructor','struct A{~A()=delete;};struct B{};','T{}','B','A')
detect('abstract','struct A{virtual void f()=0;};struct B{};','T{}','B','A')
detect('pack_list','template<int N>struct A{template<class...U>A(U...);};','T{1,2,3}','A<1>')
GOOD['empty_braced_argument']='template<class T>void accept(const T&);template<class T>auto f(int)->decltype(accept<T>({}),char());template<class>long f(...);int main(){return sizeof(f<int>(0))!=1;}'
GOOD['nested_argument']='struct A{int x[2];};template<class T>void accept(T);template<class T>auto f(int)->decltype(accept<T>({{1,2}}),char());template<class>long f(...);int main(){return sizeof(f<A>(0))!=1;}'
GOOD['copy_list_explicit']='struct A{explicit A(int);};template<class T>void accept(T);template<class T>auto f(int)->decltype(accept<T>({1}),char());template<class>long f(...);int main(){return sizeof(f<A>(0))!=sizeof(long);}'
GOOD['noexcept_lists']='struct A{A(int)noexcept;};struct B{B(int);};template<class T>constexpr bool f(){return noexcept(T{1});}static_assert(f<A>(),"");static_assert(!f<B>(),"");int main(){}'
GOOD['noexcept_fields']='struct A{A()noexcept;};struct B{B();};struct X{A a;};struct Y{B b;};template<class T>constexpr bool f(){return noexcept(T{});}static_assert(f<X>(),"");static_assert(!f<Y>(),"");int main(){}'
GOOD['constant_empty_tag']='struct Tag{};template<class T>constexpr bool f(T){return true;}template<class T>struct X{static const bool value=f(T{});};static_assert(X<Tag>::value,"");int main(){}'
GOOD['constant_aggregate']='struct A{int a;int b;};template<class T>constexpr int f(){return T{3,4}.a;}static_assert(f<A>()==3,"");int main(){return f<A>()!=3;}'
GOOD['constant_query_aggregate']='struct A{int a;int b;};template<class T>struct X{static const int value=T{3,4}.b;};static_assert(X<A>::value==4,"");int main(){}'
GOOD['constant_query_ctor']='struct A{int a;constexpr A(int x):a(x){}};template<class T>struct X{static const int value=T{3}.a;};static_assert(X<A>::value==3,"");int main(){}'
GOOD['qualified_type_tag']='struct Tag{};struct R{typedef Tag type;};template<class T>constexpr bool f(typename T::type){return true;}template<class T>struct X{static const bool value=f<T>(typename T::type{});};static_assert(X<R>::value,"");int main(){}'
GOOD['unused_poison_body']='struct A{template<class U>A(U){typename U::bad x;}};template<class T>auto f(int)->decltype(T{1},char());template<class>long f(...);static_assert(sizeof(f<A>(0))==1,"");int main(){}'
GOOD['selected_narrowing_no_fallback']='struct A{A(int);A(...);};template<class T>auto f(int)->decltype(T{0.5},char());template<class>long f(...);static_assert(sizeof(f<A>(0))==sizeof(long),"");int main(){}'
BAD['fixed_narrowing']='template<class T>auto f(T)->decltype(int{0.5});int main(){}'
BAD['ordinary_narrowing']='int main(){int x{0.5};return x;}'
BAD['fixed_braced_argument']='void accept(int);template<class T>auto f(T)->decltype(accept({0.5}));int main(){}'
GOOD['constexpr_variadic']='constexpr int f(int n,...){return n;}template<int N>struct X{};static_assert(f(3,1,2.0)==3,"");X<f(4,5,6.0)> x;int main(){return f(7,8,9.0)!=7;}'
GOOD['cast_virtual_sfinae']='struct B{};struct D:virtual B{};template<class,class>constexpr bool f(...){return true;}template<class T,class U,class=decltype((U*)((T*)0))>constexpr bool f(int){return false;}static_assert(f<B,D>(0),"");int main(){return !f<B,D>(0);}'
GOOD['cast_transitive_virtual']='struct B{};struct M:virtual B{};struct D:M{};template<class,class>char f(...);template<class T,class U,class=decltype((U*)((T*)0))>long f(int);static_assert(sizeof(f<B,D>(0))==1,"");int main(){}'
GOOD['cast_ambiguous']='struct B{};struct L:B{};struct R:B{};struct D:L,R{};template<class,class>char f(...);template<class T,class U,class=decltype((U*)((T*)0))>long f(int);static_assert(sizeof(f<B,D>(0))==1,"");int main(){}'
GOOD['cast_private_cstyle']='struct B{};class D:private B{};template<class T,class U>auto f(int)->decltype((U*)((T*)0),char());template<class,class>long f(...);static_assert(sizeof(f<B,D>(0))==1,"");int main(){}'
GOOD['cast_private_static']='struct B{};class D:private B{};template<class T,class U>auto f(int)->decltype(static_cast<U*>((T*)0),char());template<class,class>long f(...);static_assert(sizeof(f<B,D>(0))==sizeof(long),"");int main(){}'
BAD['variadic_argument_must_be_constant']='int side();constexpr int f(...){return 1;}static_assert(f(side())==1,"");int main(){}'
BAD['variadic_query_argument_must_be_constant']='int side();constexpr int f(...){return 1;}template<int>struct X{};X<f(side())> x;int main(){}'
BAD['cast_virtual_ordinary']='struct B{};struct D:virtual B{};int main(){B*b=0;D*d=(D*)b;return d!=0;}'
detect('implicit_deleted_default','struct A{int&r;A()=default;};struct B{int n;B()=default;};','T{}','B','A')
detect('default_deleted_field','struct A{A()=delete;A(int);};struct B{A a;B()=default;};struct C{};','T{}','C','B')
detect('default_ambiguous_field','struct A{A(int=0);A(double=0);};struct B{A a;B()=default;};struct C{};','T{}','C','B')
detect('string_array_field','struct A{char a[3];};struct B{char a[2];};','T{"ab"}','A','B')
detect('nested_string_array','struct A{char a[3];int n;};','T{{"ab"},4}','A')
detect('user_conversion_narrow','struct X{operator double()const;};template<class T>T dv();struct A{A(int);};struct B{B(double);};','T{dv<X>()}','double;B','int;A')
GOOD['cast_nonzero_base']='struct A{int a;};struct B{int b;};struct D:A,B{int c;};int main(){D d;B*b=&d;D*p=static_cast<D*>(b);D*q=(D*)b;D&r=static_cast<D&>(*b);B*z=0;return p!=&d||q!=&d||&r!=&d||static_cast<D*>(z)!=0;}'
GOOD['query_string_value']='struct A{char a[5];};template<class T>struct X{static const int n=T{"ab"}.a[1]+T{"ab"}.a[4];};static_assert(X<A>::n==98,"");int main(){}'
GOOD['typed_list_abi']='template<class T>auto f(T x)->decltype(T{x}){return T{x};}int main(){return f(3)!=3;}'
GOOD['cast_private_upcast']='struct B{};class D:private B{};template<class T,class U>auto f(int)->decltype(static_cast<U*>((T*)0),char());template<class,class>long f(...);static_assert(sizeof(f<D,B>(0))==sizeof(long),"");int main(){}'
GOOD['cast_private_constructor']='class A{A(int);};template<class T>auto f(int)->decltype(static_cast<T>(1),char());template<class>long f(...);static_assert(sizeof(f<A>(0))==sizeof(long),"");int main(){}'
GOOD['cast_virtual_reference']='struct B{};struct D:virtual B{};template<class T>T&dv();template<class T,class U>auto f(int)->decltype(static_cast<U&>(dv<T>()),char());template<class,class>long f(...);static_assert(sizeof(f<B,D>(0))==sizeof(long),"");int main(){}'
GOOD['cast_invalid_reference']='template<class T>T dv();template<class T>auto f(int)->decltype(const_cast<T&>(dv<T>()),char());template<class>long f(...);static_assert(sizeof(f<int>(0))==sizeof(long),"");int main(){}'
GOOD['class_ctor_default_query']='struct A{int n;constexpr A(int x,int y=2):n(x+y){}};constexpr int f(A a){return a.n;}template<class T>struct X{static const int n=f(T{3});};static_assert(X<A>::n==5,"");int main(){}'

BAD['ordinary_user_narrowing']=(Path(__file__).parent/'list75-ordinary-user-narrow.cpp').read_text()
BAD['ordinary_scalar_brace_cast']='int main(){return int{0.5};}'
BAD['ordinary_ctor_user_narrowing']='struct X{operator double()const{return 0.5;}};struct A{A(int){}};int main(){A a{X()};}'
BAD['fixed_user_narrowing']='struct X{operator double()const{return 0.5;}};template<class T>void f(){int x{X()};}int main(){}'
BAD['fixed_user_brace_cast']='struct X{operator double()const{return 0.5;}};template<class T>void f(){int x=int{X()};}int main(){}'
BAD['ordinary_user_integer_narrow']='struct X{constexpr operator int()const{return 300;}};int main(){char x{X()};return x;}'
GOOD['ordinary_user_integer_exact']='struct X{constexpr operator int()const{return 3;}};int main(){char x{X()};return x!=3;}'
GOOD['query_user_integer_exact']='struct X{constexpr operator int()const{return 3;}};template<class T>auto f(int)->decltype(T{X()},char());template<class>long f(...);static_assert(sizeof(f<char>(0))==1,"");int main(){}'
GOOD['query_user_integer_narrow']='struct X{constexpr operator int()const{return 300;}};template<class T>auto f(int)->decltype(T{X()},char());template<class>long f(...);static_assert(sizeof(f<char>(0))==sizeof(long),"");int main(){}'
GOOD['braced_reference_ranking']='struct A{int n;};template<class T>char select(T&&);template<class T>long select(const T&);template<class T>auto f(int)->decltype(select<T>({1}));static_assert(sizeof(f<A>(0))==1,"");int main(){}'
GOOD['braced_ellipsis_failure']='template<class T>char select(...);template<class T>auto f(int)->decltype(select<T>({1}));template<class>long f(...);static_assert(sizeof(f<int>(0))==sizeof(long),"");int main(){}'
GOOD['cast_ambiguous_up_reference']='struct B{};struct L:B{};struct R:B{};struct D:L,R{};template<class T>T&dv();template<class T,class U>auto f(int)->decltype(static_cast<U&>(dv<T>()),char());template<class,class>long f(...);static_assert(sizeof(f<D,B>(0))==sizeof(long),"");int main(){}'
GOOD['cast_ambiguous_up_cstyle']='struct B{};struct L:B{};struct R:B{};struct D:L,R{};template<class T,class U>auto f(int)->decltype((U*)((T*)0),char());template<class,class>long f(...);static_assert(sizeof(f<D,B>(0))==sizeof(long),"");int main(){}'
GOOD['lazy_parameter_boundary']='template<class T>struct Box{T val;static T bad(){return T::missing_name;}};void f(Box<int>);int main(){}'
runner.GOOD=GOOD;runner.BAD=BAD
if __name__=='__main__':sys.exit(0 if runner.run(Path(sys.argv[1]).resolve(),Path(sys.argv[2])) else 1)
