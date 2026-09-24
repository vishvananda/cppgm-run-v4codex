#!/usr/bin/env python3
"""Partial heads, non-deduced lists and array signatures: CC WORK."""
from pathlib import Path
import sys
import ordering_controls as runner
runner.GOOD={
 'list_other_argument':'template<class T>int f(T x,T y){return x+y;}int main(){return f({},3)!=3||f(4,{})!=4||f({2},3)!=5;}',
 'list_default':'template<class T=int>int f(T x){return x;}int main(){return f({})!=0||f({3})!=3;}',
 'list_reference_default':'template<class T=int>int f(T&&x){return x;}int main(){return f({})!=0||f({3})!=3;}',
 'list_explicit':'template<class T>int f(T x){return x;}int main(){return f<int>({})!=0||f<int>({3})!=3;}',
 'list_array_bound':'template<class T>int f(T, const unsigned char(&a)[sizeof(T)]){return a[0];}int main(){return f(1,{})!=0||f(1,{7})!=7;}',
 'list_constexpr_default':'template<class T=int>constexpr int f(T&&x){return x;}static_assert(f({})==0, "");static_assert(f({3})==3, "");int main(){}',
 'list_constexpr_member':'struct A{template<class T=int>constexpr int f(T&&x)const{return x;}};constexpr A a{};static_assert(a.f({})==0, "");static_assert(a.f({4})==4, "");int main(){}',
 'partial_array_return':'template<class T,unsigned N>T(&f(T(&x)[N]))[N]{return x;}int main(){int a[3]={};return &f<int>(a)!=&a;}',
 'partial_bound':'template<class T,int N>int f(const T(&)[N]){return N;}int main(){return f<char>("abc")!=4;}',
 'partial_multidimensional':'template<class T,int M,int N>int f(T(&)[M][N]){return M*10+N;}int main(){int a[2][3];return f<int>(a)!=23;}',
 'partial_nontype_signature':'template<class T,T N>int f(T(&)[N]){return N;}int main(){int a[4];return f<int>(a)!=4;}',
 'partial_function_parameter':'template<class T,int N>T f(T(*g)(T(&)[N]),T(&a)[N]){return g(a);}int g(int(&)[3]){return 7;}int main(){int a[3];return f<int>(g,a)!=7;}',
 'explicit_before_deduction_default':'template<class T,class U=int>int f(U){return sizeof(U);}int main(){return f<void>(1.0)!=sizeof(double);}',
 'nontype_default_after_deduction':'template<class T,int N=1>int f(T(&)[N]){return N;}int main(){int a[3];return f<int>(a)!=3;}',
 'explicit_default_no_deduction':'template<class T,class U=int>int f(){return sizeof(U);}int main(){return f<void>()!=sizeof(int);}',
 'partial_address':'template<class T,int N>int f(T(&)[N]){return N;}int main(){int(*p)(int(&)[3])=f<int>;int a[3];return p(a)!=3;}',
 'braced_no_element_deduction':'template<class T>int f(T){return 1;}int f(int){return 2;}int main(){return f({3})!=2;}',
}
runner.BAD={
 'list_no_binding':'template<class T>int f(T);int main(){return f({1});}',
 'list_narrowing':'template<class T=int>int f(T);int main(){return f({1.5});}',
 'list_mutable_reference':'template<class T=int>int f(T&);int main(){return f({});}',
 'partial_bound_conflict':'template<class T,int N>int f(T(&)[N],T(&)[N]);int main(){int a[2],b[3];return f<int>(a,b);}',
 'partial_remaining_undeduced':'template<class T,int N>int f(T);int main(){return f<int>(1);}',
}
runner.GOOD.update({
 'base_same_primary':'template<int I,class T>struct A{};template<class T>struct A<0,T>:A<1,T>{};template<int I,class T>int f(A<I,T>&){return I;}int main(){A<0,int>a;return f(a)!=0||f<1>(a)!=1;}',
 'base_deep':'template<class T>struct A{};template<class T>struct B:A<T>{};struct C:B<int>{};template<class T>int f(const A<T>&){return sizeof(T);}int main(){C c;return f(c)!=sizeof(int);}',
 'base_pointer':'template<class T>struct A{};struct B:A<int>{};struct C:B{};template<class T>int f(A<T>*){return sizeof(T);}int main(){C c;return f(&c)!=sizeof(int);}',
 'base_direct_preferred':'template<class T>struct A{};template<>struct A<int>:A<char>{};template<class T>int f(A<T>&){return sizeof(T);}int main(){A<int>a;return f(a)!=sizeof(int);}',
 'base_failed_trial_isolation':'template<class T,class U>struct A{};struct B:A<char,double>{};struct C:B,A<int,int>{};template<class T>int f(A<T,T>&){return sizeof(T);}int main(){C c;return f(c)!=sizeof(int);}',
 'array_void_composite':'int main(){const int a[2]={};void*p=0;const void*q=true?&a:p;return q!=&a;}',
 'reference_unrelated_cv':'int f(float&&v){return v;}int main(){const int i=3;return f(i)!=3;}',
 'reference_array_decay':'int f(const char*&&p){return p[0];}int main(){return f("abc")!=97;}',
 'array_const_rvalue_ref':'template<class T>int f(const T(&&)[2]){return 7;}int main(){int a[2];return f(static_cast<int(&&)[2]>(a))!=7;}',
 'list_explicit_pack':'template<class...T>int f(T...){return sizeof...(T);}int main(){return f<int,int>({},{})!=2;}',
 'list_deduced_scalar_reference':'template<class T>constexpr int f(T,const T&v){return v;}static_assert(f(1,{})==0," ");static_assert(f(1,{7})==7," ");int main(){}',
})
runner.BAD.update({
 'base_ambiguous_types':'template<class T>struct A{};struct D:A<int>,A<char>{};template<class T>int f(A<T>&);int main(){D d;return f(d);}',
 'base_repeated_nonvirtual':'template<class T>struct A{};struct B:A<int>{};struct C:A<int>{};struct D:B,C{};template<class T>int f(A<T>&);int main(){D d;return f(d);}',
 'base_private_conversion':'template<class T>struct A{};class B:A<int>{};template<class T>int f(A<T>&);int main(){B b;return f(b);}',
 'reference_related_cv':'void f(int&&);int main(){const int i=3;f(i);}',
 'array_reference_drop_cv':'void f(int(&&)[2]);int main(){const int a[2]={};f(static_cast<const int(&&)[2]>(a));}',
 'base_pointer_depth':'template<class T>struct A{};struct B:A<int>{};template<class T>int f(A<T>**);int main(){B*b=0;return f(&b);}',
 'list_pack_undeduced':'template<class...T>int f(T...);int main(){return f(1,{});}',
})
for cv in ('','const','volatile','const volatile'):
 for target in ('','const','volatile','const volatile'):
  permitted=set(cv.split())<=set(target.split())
  name='array_void_'+cv.replace(' ','_')+'_to_'+target.replace(' ','_')
  runner.GOOD[name]=f'template<class A,class B>struct C{{static char f(B*);static long f(...);enum{{value=sizeof(f(static_cast<A*>(0)))==1}};}};int main(){{return C<{cv} int[2][3],{target} void>::value!={int(permitted)};}}'
if __name__=='__main__':sys.exit(0 if runner.run(Path(sys.argv[1]).resolve(),Path(sys.argv[2])) else 1)
