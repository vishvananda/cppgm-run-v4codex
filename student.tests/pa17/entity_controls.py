#!/usr/bin/env python3
"""Template entity controls; source -> student LowIR -> supplied backend -> execution."""
from pathlib import Path
import json, subprocess, sys
ROOT=Path(__file__).resolve().parents[2]
GOOD={
'array_bounds': '''template<class>struct A{static const int n=0;};template<class T,unsigned long N>struct A<T[N]>{static const int n=N;};template<class T>struct A<T[3]>{static const int n=30;};static_assert(A<int[4]>::n==4 && A<const int[3]>::n==30 && A<int*>::n==0, "array shape");int main(){return A<double[8]>::n-8;}''',
'function_shapes': '''template<class>struct F{static const int n=0;};template<class R,class...A>struct F<R(A...)>{static const int n=sizeof...(A)+1;};template<class R,class...A>struct F<R(A...) const &>{static const int n=sizeof...(A)+10;};static_assert(F<int()>::n==1 && F<int(char,double)>::n==3 && F<int() const &>::n==10 && F<int() volatile>::n==0, "function shape");int main(){return 0;}''',
'function_adjustments': '''template<class>struct F{static const int n=0;};template<class R,class T>struct F<R(T*)>{static const int n=sizeof(T);};static_assert(F<int(const char[9])>::n==1, "array adjustment");int main(){return 0;}''',
'function_cv_argument': '''template<class>struct A{static const int n=0;};template<class T>struct A<T const>{static const int n=1;};static_assert(A<int const>::n==1 && A<int() const>::n==0 && A<int const()>::n==0, "cv classification");int main(){return 0;}''',
'repeated_packs': '''template<class...>struct L{};template<class,class>struct S{static const int n=0;};template<class...T>struct S<L<T...>,L<T...>>{static const int n=1;};static_assert(S<L<>,L<>>::n==1 && S<L<int,char>,L<int,char>>::n==1 && S<L<int>,L<char>>::n==0 && S<L<int>,L<>>::n==0, "repeat equality");int main(){return 0;}''',
'nested_expansion': '''template<class...>struct L{};template<class>struct B{};template<class>struct S;template<template<class>class C,class...T>struct S<L<C<T>...>>{static const int n=sizeof...(T);};static_assert(S<L<B<char>,B<int>>>::n==2, "nested expansion");int main(){return 0;}''',
'namespace_identity': '''namespace a{template<class>struct X{static const int n=1;};}namespace b{template<class>struct X{static const int n=2;};}template<template<class>class C>struct S{static const int n=C<int>::n;};static_assert(S<a::X>::n==1 && S<b::X>::n==2, "entity identity");int main(){return 0;}''',
'head_kinds': '''template<class,int>struct I{};template<class,unsigned long>struct U{};template<class>struct S{static const int n=0;};template<template<class,unsigned long>class C,class T,unsigned long N>struct S<C<T,N>>{static const int n=1;};static_assert(S<I<int,1>>::n==0 && S<U<int,1>>::n==1, "head kinds");int main(){return 0;}''',
'alias_identity': '''template<class T>using P=T*;template<class A,class B>struct S{static const bool v=false;};template<class T>struct S<T,T>{static const bool v=true;};static_assert(S<P<const int>,const int*>::v && !S<P<const int>,int* const>::v, "alias cv identity");int main(){return 0;}''',
'alias_template_argument': '''template<class>struct B{static const int n=7;};template<class T>using A=B<T>;template<template<class>class C>struct S:C<int>{};static_assert(S<A>::n==7, "alias template argument");int main(){return 0;}''',
'alias_value_pack': '''template<unsigned long...N>struct Seq{static const int n=sizeof...(N);};template<unsigned long...N>using Alias=Seq<N...>;template<class>struct S;template<unsigned long...N>struct S<Alias<N...>>{static const int n=sizeof...(N);};static_assert(S<Alias<>>::n==0 && S<Alias<1,2,3>>::n==3, "alias value pack");int main(){return 0;}''',
'base_relay': '''template<class>struct F;template<class R,class...T>struct F<R(T...)>{static const int n=sizeof...(T);};template<class>struct G;template<class R,class...T>struct G<R(T...)>:F<R(T...)>{};static_assert(G<int(char,double)>::n==2, "relay");int main(){return 0;}''',
'fixed_pack_order': '''template<class...>struct L{};template<class>struct S;template<template<class...>class C,class...T>struct S<C<T...>>{static const int n=1;};template<template<class...>class C,class T,class...U>struct S<C<T,U...>>{static const int n=2;};static_assert(S<L<>>::n==1 && S<L<int>>::n==2, "fixed prefix");int main(){return 0;}''',
'head_rename': '''template<template<class,int>class C>struct S;template<template<class T,int N>class D>struct S{static const int n=4;};template<class,int>struct X{};int main(){return S<X>::n-4;}''',
'cv_nested_alias': '''template<class T>struct B{};template<class T>using A=B<T>;template<class>struct S{static const int n=0;};template<class T>struct S<const A<T>>{static const int n=1;};static_assert(S<const B<char>>::n==1 && S<B<char>>::n==0, "nested cv");int main(){return 0;}''',
'array_bound_call': '''template<class T,unsigned long N>int count(T(&)[N]){return N;}int main(){int a[7];return count(a)-7;}''',
}
BAD={
'head_arity': 'template<class,class>struct X{};template<template<class>class C>struct S{};S<X> s;',
'head_redeclaration': 'template<template<class>class C>struct S;template<template<int>class C>struct S{};',
'head_value_type': 'template<long>struct X{};template<template<int>class C>struct S{};S<X> s;',
'ordinary_type_as_template': 'template<template<class>class C>struct S{};S<int> s;',
'bare_template_as_type': 'template<class>struct X{};template<class T>struct S{};S<X> s;',
'array_ambiguity': 'template<class,class>struct A;template<class T>struct A<T,int>{};template<class T>struct A<int,T>{};A<int,int> a;',
'partial_redefinition': 'template<class>struct A;template<class T>struct A<T*>{};template<class U>struct A<U*>{};',
'head_kind_redeclaration': 'template<class>struct S;template<template<class>class C>struct S{};',
}
def run(cc,work):
 work.mkdir(parents=True,exist_ok=True); rows=[]
 for good,cases in [(True,GOOD),(False,BAD)]:
  for name,source in cases.items():
   src=work/(name+'.cpp');src.write_text(source);ir=work/(name+'.lowir');exe=work/(name+'.exe')
   p=subprocess.run([str(cc),'--emit-lowir','-O0','--validate-lowir','-o',str(ir),str(src)],capture_output=True,text=True)
   row=dict(name=name,expected='native' if good else 'reject',compiler_exit=p.returncode,diagnostic=p.stderr,passed=(p.returncode==0)==good)
   if good and not p.returncode:
    b=subprocess.run([str(ROOT/'reference-binaries/lowir2native'),'-O0','-o',str(exe),str(ir)],capture_output=True,text=True)
    row['backend_exit']=b.returncode;row['backend_diagnostic']=b.stderr
    row['native_exit']=subprocess.run([str(exe)]).returncode if not b.returncode else None
    row['passed']=not b.returncode and row['native_exit']==0
   rows.append(row);print(name,'PASS' if row['passed'] else 'FAIL',p.stderr.strip())
 (work/'results.json').write_text(json.dumps(rows,indent=2)+'\n');return all(r['passed'] for r in rows)
if __name__=='__main__':sys.exit(0 if run(Path(sys.argv[1]).resolve() if len(sys.argv)>1 else ROOT/'dev/cppgm++',Path(sys.argv[2]) if len(sys.argv)>2 else Path('/tmp/pa17-controls')) else 1)
