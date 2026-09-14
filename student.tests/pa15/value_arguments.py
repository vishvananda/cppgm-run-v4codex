#!/usr/bin/env python3
"""PA15 typed integral argument identity, environment, and rejection controls."""
import pathlib, subprocess, tempfile, sys
root = pathlib.Path(__file__).resolve().parents[2]
compiler = pathlib.Path(sys.argv[1]).resolve() if len(sys.argv)>1 else root/'dev/cppgm++'
common = 'template<class T, T V> struct C { static const T value = V; };\n'
cases = {
 'overload_conversion': (True, '''template<bool N> int f() { return N; }
template<int N> int f() { return N; } int main(){return f<2>()-2;}'''),
 'member_alias': (True, '''template<class T,T N> struct C { typedef T result; result f(); };
template<class U,U M> typename C<U,M>::result C<U,M>::f() { return M; }
int main(){C<int,7> c;return c.f()-7;}'''),
 'class_redeclaration': (True, '''template<class T,T N> struct C;
template<class U,U M=7> struct C { static const U value=M; };
static_assert(C<int>::value==7,"head"); int main(){return 0;}'''),
 'wrong_redeclaration': (False, 'template<int N> struct C; template<long N> struct C {};'),
 'kind_redeclaration': (False, 'template<class T> struct C; template<int T> struct C {};'),
 'dependent_cast': (True, '''struct S{static const int value=1;};
template<bool N>struct C{static const bool value=N;};
template<class T>struct D{typedef C<bool(T::value)> type;};
static_assert(D<S>::type::value,"cast");int main(){return 0;}'''),
 'enum_type': (True, '''enum class E{a=3};template<E N>struct C{static const E value=N;};
static_assert(C<E::a>::value==E::a,"enum identity");int main(){return 0;}'''),
 'enum_conversion': (False, 'enum class E{a=3};template<int N>struct C{};C<E::a> c;'),
 'canonical': (True, '''template<int N> int f() { return N; }
int main() { return f<3>() + f<1+2>() + f<static_cast<int>(3L)>(); }'''),
 'defaults': (True, common+'''template<class T, T V, T W=V+1> struct D { int a[W]; };
static_assert(sizeof(D<int,3>)==16,"default substitution");
static_assert(C<unsigned long long,18446744073709551615ULL>::value == ~0ULL,"bits");
static_assert(C<long long,-9223372036854775807LL-1>::value < 0,"sign"); int main(){return 0;}'''),
 'function_defaults': (True, '''template<int N, int M=N+2> int f() { return M; }
int main(){return f<5>()-7;}'''),
 'out_of_line': (True, '''template<class T,T N> struct C { T f(); static const T value; };
template<class U,U M> U C<U,M>::f() { return M; }
template<class U,U M> const U C<U,M>::value = M;
int main(){C<int,9> c; return c.f()+C<int,9>::value-18;}'''),
 'array_signature': (True, '''template<int N> int f(int (&p)[N]) { return sizeof(p); }
int main(){int p[5]; return f<5>(p)-20;}'''),
 'head_shadow': (True, '''typedef long number;
template<number N> struct C { static const long value=N; };
int main(){typedef char number; return C<257>::value-257;}'''),
 'narrow_bool': (False, common+'C<bool,2> x;'),
 'narrow_char': (False, common+'C<signed char,256> x;'),
 'narrow_sign': (False, common+'C<unsigned,-1> x;'),
 'mutable': (False, common+'int n=2; C<int,n> x;'),
 'volatile': (False, common+'const volatile int n=2; C<int,n> x;'),
 'overflow': (False, common+'C<int,2147483647+1> x;'),
 'function_kind': (False, 'template<class T> int f(T x) { return 1; } int main(){return f<3>(2);}'),
 'overload_kinds': (True, '''template<class T> int f() { return sizeof(T); }
template<int N> int f() { return N; }
int main(){return f<int>()+f<3>()-7;}'''),
 'value_prvalue': (False, 'template<int N> int f() { N=3; return N; } int main(){return f<2>();}'),
 'wrong_kind': (False, 'template<class T> struct C {}; C<3> x;'),
 'dependent_bad': (False, common+'template<class T,T N> struct D { C<bool,N> x; }; D<int,2> x;'),
 'partial_explicit': (True, '''template<int N,class T> int f(T x) { return N+x; }
int main(){return f<7>(3)-10;}'''),
}
with tempfile.TemporaryDirectory(prefix='pa15-values-') as td:
 for name,(success,source) in cases.items():
  src,out=pathlib.Path(td)/f'{name}.cpp',pathlib.Path(td)/f'{name}.lowir';src.write_text(source)
  p=subprocess.run([str(compiler),'--emit-lowir','-O0','-o',str(out),str(src)],capture_output=True)
  assert (p.returncode==0)==success,(name,p.returncode,p.stderr.decode())
  if name=='canonical':
   assert out.read_text().count('function @f(')==1,out.read_text()
  print(name,'pass')
print(len(cases),'value argument groups passed')
