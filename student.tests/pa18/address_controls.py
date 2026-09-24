#!/usr/bin/env python3
"""C++11 [temp.arg.nontype]/1,5 identity/conversion and rejection controls."""
from pathlib import Path
import sys
import ordering_controls as runner
GOOD={
 'pointer':'int x=1;template<int*P>struct A{static void f(){*P=7;}};int main(){A<&x>::f();return x!=7;}',
 'reference':'int x=1;template<int&R>struct A{static void f(){++R;}};int main(){A<x>::f();return x!=2;}',
 'const_reference':'const int x=7;template<const int&R>struct A{static int f(){return R;}};int main(){return A<x>::f()!=7;}',
 'reference_const_added':'int x=7;template<const int&R>struct A{static int f(){return R;}};int main(){return A<x>::f()!=7;}',
 'static_distinct':'struct X{static int n;};struct Y{static int n;};int X::n=3;int Y::n=5;template<int*P>struct A{static int f(){return *P;}};int main(){return A<&X::n>::f()!=3||A<&Y::n>::f()!=5;}',
 'internal':'static int x=7;template<int*P>int f(){return *P;}int main(){return f<&x>()!=7;}',
 'qualification':'int x=7;template<const int*P>int f(){return *P;}int main(){return f<&x>()!=7;}',
 'null_cast':'template<int*P=(int*)0>struct A{static bool f(){return P==nullptr;}};int main(){return !A<>::f();}',
 'null_keyword':'template<int*P=nullptr>struct A{static bool f(){return P==nullptr;}};int main(){return !A<>::f();}',
 'array':'int x[2]={7,3};template<const int*P>int f(){return P[0]+P[1];}int main(){return f<x>()!=10;}',
 'array_reference':'int x[2]={7,3};template<int(&R)[2]>int f(){++R[1];return R[0]+R[1];}int main(){return f<x>()!=11;}',
 'nested_forward':'int x=7;template<int*P>struct A{static int f(){return *P;}};template<int*P>struct B{template<class T>struct C:A<P>{};};int main(){return B<&x>::C<int>::f()!=7;}',
 'reference_forward':'int x=7;template<int&R>struct A{static int f(){return R;}};template<int&R>struct B:A<R>{};int main(){return B<x>::f()!=7;}',
 'function':'int f(int x){return x+2;}template<int(*P)(int)>int g(){return P(5);}int main(){return g<f>()!=7||g<&f>()!=7;}',
 'function_reference':'int f(int x){return x+2;}template<int(&R)(int)>int g(){return R(5);}int main(){return g<f>()!=7;}',
 'function_overload':'int f(int x){return x+2;}long f(long){return 9;}template<int(*P)(int)>int g(){return P(5);}int main(){return g<&f>()!=7;}',
 'function_template':'template<class T>T f(T x){return x+2;}template<int(*P)(int)>int g(){return P(5);}int main(){return g<&f>()!=7;}',
 'function_static_template':'struct A{template<class T>static T f(T x){return x+2;}};template<int(*P)(int)>int g(){return P(5);}int main(){return g<&A::f>()!=7;}',
 'parentheses':'int x=7;template<int*P>int f(){return *P;}int main(){return f<(&x)>()!=7;}',
 'identity':'int x=7;template<int*P>struct A{};template<class T,class U>struct Same{static const bool value=false;};template<class T>struct Same<T,T>{static const bool value=true;};template<int*P>struct B{typedef A<P> type;};static_assert(Same<A<&x>,B<&x>::type>::value,"");int main(){}',
}
BAD={
 'integral_zero':'template<int*P>struct A{};A<0>a;',
 'reference_temporary':'template<const int&R>struct A{};A<1>a;',
 'local':'template<int*P>struct A{};int main(){int x;A<&x>a;}',
 'local_static':'template<int*P>struct A{};int main(){static int x;A<&x>a;}',
 'thread_local':'thread_local int x;template<int*P>struct A{};A<&x>a;',
 'subobject':'int x[2];template<int*P>struct A{};A<&x[0]>a;',
 'nonstatic_member':'struct X{int n;};X x;template<int*P>struct A{};A<&x.n>a;',
 'static_through_object':'struct X{static int n;};int X::n;X x;template<int*P>struct A{};A<&x.n>a;',
 'string':'template<const char*P>struct A{};A<"x">a;',
 'drop_const':'const int x=7;template<int*P>struct A{};A<&x>a;',
 'void_conversion':'int x;template<void*P>struct A{};A<&x>a;',
 'base_conversion':'struct B{};struct D:B{};D x;template<B*P>struct A{};A<&x>a;',
 'pointer_variable':'int x;constexpr int*p=&x;template<int*P>struct A{};A<p>a;',
 'pointer_arithmetic':'int x;template<int*P>struct A{};A<&x+0>a;',
 'reference_address':'int x;template<int&R>struct A{};A<&x>a;',
 'reference_cv':'const int x=1;template<int&R>struct A{};A<x>a;',
 'deleted_function':'int f(int)=delete;template<int(*P)(int)>struct A{};A<f>a;',
}
GOOD.update({
 'explicit_function':'template<class T,class U>int f(U){return sizeof(T);}template<int(*P)(int)>int g(){return P(0);}int main(){return g<&f<char>>()!=1;}',
 'dependent_pointer_pack':'int a=3,b=4;template<int*...P>struct A{static int size(){return sizeof...(P);}};template<int*...P>struct B:A<P...>{};int main(){return B<&a,&b>::size()!=2;}',
 'dependent_reference_array_pack':'int a[2],b[2];template<int(&...R)[2]>struct A{static int size(){return sizeof...(R);}};template<int(&...R)[2]>struct B:A<R...>{};int main(){return B<a,b>::size()!=2;}',
 'function_parameter_adjustment':'int f(int x){return x+2;}template<int P(int)>int g(){return P(5);}int main(){return g<f>()!=7;}',
 'array_parameter_adjustment':'int a[2]={3,4};template<int P[2]>int f(){return P[0]+P[1];}int main(){return f<a>()!=7;}',
 'static_demand':'template<class T>struct A{static int x;};template<class T>int A<T>::x=7;template<int*P>int f(){return *P;}int main(){return f<&A<int>::x>()!=7;}',
 'pointer_sfinae':'template<int*P>struct A{};template<class T>auto f(T*)->decltype(A<&T::x>(),int()){return 7;}int f(...){return 3;}struct S{static int x;};int S::x;struct B{int x;};int main(){S s;B b;return f(&s)!=7||f(&b)!=3;}',
 'reference_decltype':'int x=7;template<int&R>struct A{typedef decltype(R) type;static type f(){return R;}};int main(){A<x>::f()=3;return x!=3;}',
 'pointer_const_read':'int x=7;template<int*const P>int f(){return *P;}int main(){return f<&x>()!=7;}',
 'pointer_forward_cv':'int x=7;template<const int*P>int f(){return *P;}template<int*P>int g(){return f<P>();}int main(){return g<&x>()!=7;}',
 'address_reference_identity':'int x=7;template<int&R>int*f(){return &R;}int main(){return f<x>()!=&x;}',
 'null_identity':'template<int*P>struct A{};template<class T,class U>struct Same{static const bool value=false;};template<class T>struct Same<T,T>{static const bool value=true;};static_assert(Same<A<nullptr>,A<(int*)0>>::value,"");int main(){}',
 'null_pointer_variable':'constexpr int*p=nullptr;template<int*P>bool f(){return P==nullptr;}int main(){return !f<p>();}',
 'outer_parameter_type':'int x=7;template<class T>struct A{template<T P>struct B{static int f(){return *P;}};};int main(){return A<int*>::B<&x>::f()!=7;}',
})
BAD.update({
 'address_body_demand':'template<void(*)()>struct A{};template<class T>struct B{static void f(){T::bad();}typedef A<&B::f> type;};template<class T>struct D:B<T>{typedef int type;};template<class T>typename D<T>::type g(T*,long*);template<class T>void g(T*,int){}int main(){g((int*)0,0);}',
 'explicit_wrong_function':'template<class T>int f(T){return 1;}template<int(*P)(int)>struct A{};A<&f<char>>a;',
 'reference_pointer_conversion':'int x;int*p=&x;template<const int*&R>struct A{};A<p>a;',
 'reference_rvalue_parameter':'template<int&&R>struct A{};',
})
if __name__=='__main__':
 runner.GOOD=GOOD;runner.BAD=BAD
 sys.exit(0 if runner.run(Path(sys.argv[1]).resolve(),Path(sys.argv[2])) else 1)
