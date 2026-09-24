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
if __name__=='__main__':sys.exit(0 if runner.run(Path(sys.argv[1]).resolve(),Path(sys.argv[2])) else 1)
