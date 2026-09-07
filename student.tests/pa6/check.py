#!/usr/bin/env python3
"""Independent scope/type interactions, with exact snippets or rejection oracles."""
import pathlib
import subprocess
import sys
import tempfile

root = pathlib.Path(__file__).resolve().parents[2]
compiler = pathlib.Path(sys.argv[1]).resolve() if len(sys.argv) > 1 else root/'dev/cppgm++'
cases = [
 ('qualified-class-owner', 'namespace A{struct X;}namespace B{struct A::X{int y;};}', None),
 ('qualified-enum-owner', 'namespace A{enum class E:int;}namespace B{enum class A::E:int{a};}', None),
 ('qualified-class-enclosing', 'namespace A{struct X;using T=int;}struct A::X{T member;};', ['variable member int']),
 ('qualified-class-parser-owner', 'namespace A{using word=int;struct X;}struct A::X{void f(){word x;}};', ['variable x int']),
 ('qualified-nested-class-parser-owner', 'struct O{struct X;using word=int;};struct O::X{void f(){word x;}};', ['variable x int']),
 ('qualified-enum-base-owner', 'namespace A{using word=int;enum class E:word;}enum class A::E:word{v};', ['enumerator v enum class A::E 0']),
 ('using-alias-conflict', 'typedef int U;using U=char;', None),
 ('using-alias-repeat', 'using U=int;using U=int;U x;', ['variable x int']),
 ('function-pointer-redeclare', 'int(*p)(const int);int(*p)(int);', ['variable p pointer to function of (const int) returning int','variable p pointer to function of (int) returning int']),
 ('function-alias-redeclare', 'using F=int(const int);typedef int F(int);F *p;', ['variable p pointer to function of (const int) returning int']),
 ('alias-source-form', 'namespace N{using F=int(const int);}using N::F;F *p;', ['type-alias F function of (const int) returning int','variable p pointer to function of (const int) returning int']),
 ('inline-root-hit', 'namespace A{using T=char;}namespace N{using T=int;inline namespace I{using namespace A;}}N::T x;', ['variable x int']),
 ('inline-child-hit', 'namespace A{using T=char;}namespace N{using namespace A;inline namespace I{using T=int;}}N::T x;', ['variable x int']),
 ('inline-directives', 'namespace A{using T=int;}namespace N{inline namespace I{using namespace A;}}N::T x;', ['variable x int']),
 ('inline-conflict', 'namespace N{using T=int;inline namespace I{using T=char;}}N::T x;', None),
 ('constructor-try-body', 'struct S{S()try {T body;}catch(...){T handler;}using T=int;};', ['variable body int','variable handler int']),
 ('for-lifetime', 'void f(){for(const int n=3;;){int a[n];}int b[n];}', None),
 ('for-body-lifetime', 'using T=int;void f(){for(;;)using T=char;T x;}', ['variable x int']),
 ('branch-lifetime', 'using T=int;void f(){if(1)using T=char;else{T x;}T y;}', ['variable x int','variable y int']),
 ('condition-lifetime', 'const int n=5;void f(){if(const int n=3){int a[n];}int b[n];}', ['variable a array of 3 int','variable b array of 5 int']),
 ('do-body-lifetime', 'using T=int;void f(){do using T=char;while(false);T x;}', ['variable x int']),
 ('layout-overflow', 'struct X{char a[18446744073709551615ULL];char b;};int n[sizeof(X)];', None),
 ('layout-tail-overflow', 'struct X{long x;char a[18446744073709551615ULL-8];};int n[sizeof(X)];', None),
 ('layout-max-array', 'struct X{char a[18446744073709551615ULL];};static_assert(sizeof(X)==18446744073709551615ULL,"size");', ['type X struct X']),
 ('anonymous-object-not-qualifier', 'static struct{using T=int;}object;object::T x;', None),
 ('function-point', 'using T=int;namespace N{void f(){T x;}using T=char;}', ['variable x int']),
 ('class-point', 'using T=int;namespace N{struct C{void f(){T x;}};using T=char;}', ['variable x int']),
 ('local-class-point', 'using T=int;void f(){struct C{void f(){T x;}};using T=char;}', ['variable x int']),
 ('nested-complete-class', 'struct O{struct I{void f(){T x;}};using T=int;};', ['variable x int']),
 ('layout', 'struct S{char x;int y;static int z;};union U{char x;long y;};int a[sizeof(S)];int b[alignof(S)];int c[sizeof(U)];', ['variable a array of 8 int','variable b array of 4 int','variable c array of 8 int']),
 ('qualifier-shadow', 'namespace N{using T=int;}namespace M{int N;N::T x;}', ['variable x int']),
 ('injected-name', 'struct C{C(){} C* next;};int C(int);', ['variable next pointer to struct C']),
 ('anchor-shadow', 'namespace A{using T=int;} namespace B{using T=char; namespace C{using namespace A; T x;}}', ['variable x char']),
 ('anchor-ambiguity', 'namespace A{using T=int;} namespace B{using T=char;} using namespace A; using namespace B; T x;', None),
 ('transitive-anchor', 'namespace A{using T=int;} namespace B{using namespace A;} namespace D{using T=char; namespace E{using namespace B;T x;}}', ['variable x char']),
 ('using-cycle', 'namespace A{} namespace B{using namespace A;} namespace A{using namespace B;using T=int;} using namespace B;T x;', ['variable x int']),
 ('duplicate-paths', 'namespace A{using T=int;} namespace B{using namespace A;} namespace C{using namespace A;} using namespace B;using namespace C;T x;', ['variable x int']),
 ('qualified-direct', 'namespace A{using T=int;} namespace B{using namespace A;using T=char;} B::T x;', ['variable x char']),
 ('enum-shadow', 'enum E { a }; namespace N{enum E{b}; E x;} E y;', ['variable x enum E', 'variable y enum E']),
 ('qualified-enum', 'struct S{enum class E:int;};enum class S::E:int{a=3,b=a+1};int x[static_cast<int>(S::E::b)];', ['variable x array of 4 int']),
 ('enum-cv', 'enum class E{v=2};constexpr E x=E::v;static_assert(x==E::v,"ok");', ['variable x const enum class E']),
 ('enum-to-int', 'enum class E{v};int x=E::v;', None),
 ('int-to-enum', 'enum class E{v};E x=1;', None),
 ('alias-redeclare', 'using T=int;typedef int T;T x;', ['variable x int']),
 ('alias-conflict', 'using T=int;typedef char T;', None),
 ('unsigned-wrap', 'constexpr unsigned n=4294967295u+2u;int x[n];', ['variable x array of 1 int']),
 ('promotions', 'constexpr unsigned char n=255;int x[n+1];static_assert(-1<1u?0:1,"conversion");', ['variable x array of 256 int']),
 ('negative-bound', 'int x[-1];', None),
 ('short-circuit', 'static_assert(1 || (1/0),"or");static_assert(!(0 && (1/0)),"and");int x[1?3:1/0];', ['variable x array of 3 int']),
 ('unsigned-product', 'constexpr unsigned long long n=18446744073709551615ULL*18446744073709551615ULL;int x[n];', ['variable x array of 1 int']),
 ('nested-reference', 'using R=const int&;constexpr int n=4;R r=n;R r2=r;int x[r2];', ['variable x array of 4 int']),
 ('array-function', 'using F=int(const int);F *x;void f(int a[3],F cb);void f(int*,int(*)(int));', ['variable x pointer to function of (const int) returning int']),
 ('later-typedef', 'struct S{void f(){U u;V v;}typedef int U,V;};', ['variable u int','variable v int']),
 ('later-pointer-typedef', 'struct S{void f(){P p;A a;}typedef int *P,A[3];};', ['variable p pointer to int','variable a array of 3 int']),
 ('template-isolation', 'template<template<class I>class T>struct S{I x;};', None),
 ('anonymous-ranges', 'static union{int a;}; static union{int b;long c;};', ['scope class __anonymous_union_type__1_7','scope class __anonymous_union_type__9_18']),
 ('unnamed-parameters', 'void f(const int, int[3]){}', ['parameter  const int','parameter  array of 3 int']),
]
with tempfile.TemporaryDirectory(prefix='pa6-personal-') as tmp:
    tmp = pathlib.Path(tmp)
    for name, source, expected in cases:
        src, out = tmp/(name+'.cpp'), tmp/'out'
        src.write_text(source)
        result = subprocess.run([compiler,'--emit-types','-o',out,src],capture_output=True,text=True,timeout=15)
        assert 'Sanitizer' not in result.stderr and 'runtime error:' not in result.stderr, result.stderr
        assert result.returncode == (1 if expected is None else 0), (name,result.returncode,result.stderr)
        if expected is not None:
            text = out.read_text()
            for line in expected:
                assert line in text, (name,line,text)
    print(f'PA6 personal behavior: {len(cases)} passed')
