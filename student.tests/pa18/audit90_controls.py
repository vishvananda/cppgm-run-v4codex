#!/usr/bin/env python3
"""Nonfinal parameter packs: N3485 [temp.deduct.call]/1, [temp.deduct.type]/5,
[temp.arg.explicit]/9 and [temp.inst]/10. Run CC WORK; check actual execution.
"""
from pathlib import Path
import sys
import ordering_controls as runner

runner.GOOD = {
    'default_before_pack': 'template<class...T>int f(int x=7,T...v){return sizeof...(v)+x;}int main(){return f()!=7||f(3,1,2L)!=5;}',
    'defaults_surround_pack': 'template<class...T>int f(int x=3,T...v,int y=7){return sizeof...(v)+x+y;}int main(){return f()!=10||f<int,long>(3,1,2)!=12;}',
    'empty': 'template<class...T>int f(T...v,int x){return sizeof...(v)+x;}int main(){return f(3)!=3;}',
    'explicit': 'template<class...T>int f(T...v,int x){return sizeof...(v)+x;}int main(){return f<int,long>(1,2,3)!=5;}',
    'trailing_deduction': 'template<class...T,class U>int f(T...v,U x){return sizeof...(v)+x;}int main(){return f(3)!=3||f<int,long>(1,2,3L)!=5;}',
    'target': 'template<class...T>int f(T...v,int x){return sizeof...(v)+x;}int main(){int(*p)(long,int)=f<long>;return p(2,3)!=4;}',
    'empty_target': 'template<class...T>int f(T...v,int x){return sizeof...(v)+x;}int main(){int(*p)(int)=f;return p(3)!=3;}',
    'reference_target': 'template<class...T>int f(T...v,int x){return sizeof...(v)+x;}int main(){int(&p)(long,int)=f<long>;return p(2,3)!=4;}',
    'target_result_deduction': 'template<class R,class...T>R f(T...v,int x){return sizeof...(v)+x;}int main(){long(*p)(int)=f;return p(3)!=3;}',
    'converted_prefix': 'template<class...T>int f(T...v,int x){return sizeof...(v)+x;}int main(){return f<int*,float*>(0,0,3)!=5;}',
    'default_empty': 'template<class...T>int f(T...v,int x=7){return sizeof...(v)+x;}int main(){return f()!=7||f(3)!=3;}',
    'default_explicit': 'template<class...T>int f(T...v,int x=7){return sizeof...(v)+x;}int main(){return f<int,long>(1,2)!=9||f<int>(1)!=8;}',
    'default_effects': 'int calls;int next(){return ++calls;}template<class...T>int f(T...v,int x=next()){return sizeof...(v)+x;}int main(){int a=f<int,long>(1,2);int b=f();return a!=3||b!=2||calls!=2;}',
    'default_dependent': 'template<class U,class...T>int f(T...v,int x=sizeof(U)){return sizeof...(v)+x;}int main(){return f<long,int,int>(1,2)!=2+sizeof(long);}',
    'default_dormant': 'template<class U,class...T>int f(T...v,int x=sizeof(typename U::missing)){return sizeof...(v)+x;}int main(){return f<int,long>(1,4)!=5;}',
    'query': 'template<class...T>long f(T...,int);template<class U>auto g(U v)->decltype(f(v)){return 7;}int main(){return g(3)!=7;}',
    'constant_query': 'template<class...T>constexpr int f(T...v,int x=7){return sizeof...(v)+x;}static_assert(f()==7,"empty");static_assert(f<int,long>(1,2)==9,"expanded defaults");int main(){return f<int>(1,3)!=4;}',
    'member': 'struct A{template<class...T>int f(T...v,int x=7){return sizeof...(v)+x;}};int main(){A a;return a.f()!=7||a.f<int,long>(1,2)!=9;}',
    'constructor': 'struct A{int n;template<class...T>A(T...,int x):n(x){}};int main(){A a(7);return a.n!=7;}',
    'call_operator': 'struct A{template<class...T>int operator()(T...,int x){return x;}};int main(){A a;return a(7)!=7;}',
    'nested_target': 'int g(int x){return x;}template<class...T>int f(int(*p)(T...,int)){return p(7);}int main(){return f(g)!=7;}',
    'nested_explicit_target': 'int g(long a,int x){return a+x;}template<class...T>int f(int(*p)(T...,int)){return sizeof...(T);}int main(){return f<long>(g)!=1;}',
    'mixed_nondeduced': 'template<class A,class B>struct L{};template<class...T,class U>int f(L<T,U>...,U){return sizeof...(T);}int main(){return f<int>(L<int,long>(),2L)!=1;}',
    'distinct_packs': 'template<class...T,class...U>int f(T...a,int x,U...b){return sizeof...(a)+sizeof...(b)+x;}int main(){return f<int>(1,3,2L)!=5;}',
    'repeated_pack': 'template<class...T>int f(T...a,int x,T...b){return sizeof...(a)+sizeof...(b)+x;}int main(){return f<int>(1,3,2)!=5||f(7)!=7;}',
    'repeated_prefix_conversion': 'template<class...T>int f(T...,int,T...){return 1;}int main(){return f<int>(1,2,3L)!=1;}',
    'nontype_pack': 'template<int>struct Tag{};template<int...N>int f(Tag<N>...,int x){return sizeof...(N)+x;}int main(){return f<1,2>(Tag<1>(),Tag<2>(),3)!=5||f(3)!=3;}',
    'reference_prefix': 'template<class...T>int f(T&...v,int x){return sizeof...(v)+x;}int main(){int a=1;long b=2;return f<int,long>(a,b,3)!=5;}',
    'array_reference_prefix': 'template<class...T>int f(T&...v,int x){return sizeof...(v)+x;}int main(){int a[2]={};return f<int[2]>(a,3)!=4;}',
    'fallback': 'template<class...T>int f(T...,int){return 1;}int f(...){return 2;}int main(){return f(3)!=1||f(1,2)!=2;}',
    'retained_default': 'template<class...T>int f(T...v,int x=7){return sizeof...(v)+x;}template<class U>int g(U){return f<int,long>(1,2);}int main(){return g(1)!=9||g(2L)!=9;}',
}
runner.BAD = {
    'default_on_pack': 'template<class...T>int f(T...v=0){return sizeof...(v);}int main(){return f();}',
    'no_pack_deduction': 'template<class...T>int f(T...,int){return 1;}int main(){return f(1,2);}',
    'short_explicit': 'template<class...T>int f(T...,int){return 1;}int main(){return f<int,long>(1,2);}',
    'prefix_conversion': 'template<class...T>int f(T...,int){return 1;}int main(){return f<int*>(1,2);}',
    'target_mismatch': 'template<class...T>int f(T...,int){return 1;}int main(){int(*p)(float,int)=f<long>;return p(1,2);}',
    'target_no_pack_deduction': 'template<class...T>int f(T...,int){return 1;}int main(){int(*p)(long,int)=f;return p(1,2);}',
    'bad_default_demand': 'template<class U,class...T>int f(T...,int x=sizeof(typename U::missing)){return x;}int main(){return f<int,long>(1);}',
    'reference_temporary': 'template<class...T>int f(T&...,int){return 1;}int main(){return f<int>(1,2);}',
    'deduced_elsewhere_arity': 'template<class...T>struct L{};template<class...T>int f(L<T...>,T...,int){return 1;}int main(){return f(L<int,long>(),1,2L,3);}',
    'repeated_pack_conflict': 'template<class...T>int f(T...,int,T...){return 1;}int main(){return f<int>(1,2,3,4);}',
    'no_trailing_type': 'template<class...T,class U>int f(T...,U){return 1;}int main(){return f<int>(1);}',
}
if __name__ == '__main__':
    sys.exit(0 if runner.run(Path(sys.argv[1]).resolve(),Path(sys.argv[2])) else 1)
