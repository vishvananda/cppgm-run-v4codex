#!/usr/bin/env python3
"""Initialization/storage and retained body-query controls, run explicitly."""
from pathlib import Path
import subprocess, tempfile, sys
ROOT=Path(__file__).resolve().parents[2]
CC=Path(sys.argv[1]).resolve() if len(sys.argv)>1 else ROOT/'dev/cppgm++'
GOOD={
 'derived_value': '''struct B{int x;};template<class T>struct D:B{T y;};
int main(){D<int> d{};return d.x+d.y;}''',
 'derived_member_initialization': '''struct B{int x;};template<class T>struct D:B{T y=7;};
int main(){D<int> d{};return d.x+d.y-7;}''',
 'derived_nested': '''struct B{int x;};template<class T>struct D:B{T y;};
struct A{D<int> d[2];};int main(){A a{};return a.d[0].x+a.d[0].y+a.d[1].x+a.d[1].y;}''',
 'derived_user_constructor': '''struct B{int x;B():x(3){}};template<class T>struct D:B{T y;};
int main(){D<int> d{};return d.x+d.y-3;}''',
 'derived_member_pointer': '''struct X{int n;};struct B{int X::*p;};template<class T>struct D:B{T y;};
int main(){D<int> d{};const unsigned char* b=(const unsigned char*)&d;return b[0]!=255 || b[7]!=255 || d.y!=0;}''',
 'dependent_updates': '''template<class T>struct C{static int n;int f(){++n;--n;return ++n;}};
template<class T>int C<T>::n=3;int main(){C<int> c;return c.f()-4;}''',
 'volatile_updates': '''template<class T>int f(){volatile int n=3;++n;--n;return n;}
int main(){return f<int>()-3;}''',
 'postfix_updates': '''template<class T>struct C{static int n;int f(){n++;n--;return n++;}};
template<class T>int C<T>::n=3;int main(){C<int> c;return c.f()+C<int>::n-7;}''',
 'static_volatile_updates': '''template<class T>struct C{static volatile int n;int f(){++n;n--;return n;}};
template<class T>volatile int C<T>::n=3;int main(){C<int> c;return c.f()-3;}''',
 'array_volatile': '''int main(){constexpr volatile int a[2]={3,5};return a[0]+a[1]-8;}''',
 'array_aggregate': '''struct A{int n;double d;};int main(){constexpr A a[2]={{3,1.5},{5,2.5}};return a[0].n+a[1].n!=8 || a[0].d+a[1].d!=4.0;}''',
 'array_static_addresses': '''int n=7;int main(){constexpr int* a[2]={&n,nullptr};return *a[0]-7;}''',
 'array_static_reference': '''const int n=7;struct R{const int&r;};int main(){constexpr R a[1]={{n}};return a[0].r-7;}''',
 'array_qualified': '''namespace A{typedef unsigned word;}int main(){A::word constexpr x[4]={1,2,3,4};return x[2]-3;}''',
 'array_nested': '''int main(){constexpr int a[2][3]={{1,2},{3}};return a[0][0]+a[0][1]+a[0][2]+a[1][0]+a[1][1]+a[1][2]-6;}''',
 'array_deduced_bound': '''int main(){constexpr long a[]={1,2,3};return sizeof(a)!=24 || a[2]!=3;}''',
 'array_string': '''int main(){constexpr char s[]="abc";return sizeof(s)!=4 || s[2]!='c' || s[3]!=0;}''',
 'array_sparse': '''int main(){constexpr unsigned a[1024]={7};return a[0]!=7 || a[1023]!=0;}''',
 'array_template': '''template<int N>int f(int k){constexpr int a[3]={N,N+1,N+2};return a[k];}
int main(){return f<3>(1)+f<3>(2)+f<8>(0)-17;}''',
 'array_local_identity': '''int f(int n){constexpr int a[2]={3,4};constexpr int b[2]={3,4};return a==b ? 100:a[n]+b[n];}
int main(){return f(1)-8;}''',
 'array_static': '''int f(){static constexpr int a[2]={4,5};return a[1];}int main(){return f()-5;}''',
 'array_namespace': '''constexpr int a[2]={4,5};int main(){return a[1]-5;}''',
 'aggregate_relocation':(ROOT/'student.tests/pa15/aggregate_relocation.cpp').read_text(),
}
BAD={
 'postfix_const': 'template<class T>struct C{static const int n=3;int f(){return n++;}};int main(){return 0;}',
 'array_nonliteral':'struct A{int x;~A(){}};int main(){constexpr A a[2]={{1},{2}};}',
 'array_automatic_address':'int main(){int n=7;constexpr int* a[1]={&n};}',
 'array_temporary_reference':'struct R{const int&r;};int main(){constexpr R a[1]={{7}};}',
 'array_thread_address':'thread_local int n;int main(){constexpr int* a[1]={&n};}',
 'array_nonnull_integer':'int main(){constexpr int* a[1]={(int*)7};}',
 'array_dynamic':'int f();int main(){constexpr int a[2]={1,f()};}',
 'array_missing':'int main(){constexpr int a[2];}',
 'array_narrowing':'int main(){constexpr unsigned char a[2]={1,256};}',
 'update_const': 'template<class T>int f(){const int n=3;return ++n;}int main(){return 0;}',
 'update_prvalue':'template<class T>int f(){return ++3;}int main(){return 0;}',
 'update_enum':'enum E{a};template<class T>int f(){E e=a;return ++e;}int main(){return 0;}',
 'decrement_bool':'template<class T>int f(){bool n=true;return --n;}int main(){return 0;}',
}
fail=[]
with tempfile.TemporaryDirectory(prefix='pa15-initialization-') as td:
 for name,source in {**GOOD,**BAD}.items():
  src=Path(td)/(name+'.cpp');ir=src.with_suffix('.lowir');exe=src.with_suffix('.exe');src.write_text(source)
  r=subprocess.run([CC,'--emit-lowir','-O0','--validate-lowir','-o',ir,src],capture_output=True,text=True)
  okay=(r.returncode==0)==(name in GOOD)
  if okay and name in GOOD:
   r=subprocess.run([ROOT/'dev/lowir2native-ref','-O0','-o',exe,ir],capture_output=True,text=True);okay=r.returncode==0
   if okay:r=subprocess.run([exe],capture_output=True,text=True,timeout=10);okay=r.returncode==0
  print(name,'PASS' if okay else 'FAIL',r.returncode,r.stderr.strip(),flush=True)
  if not okay:fail.append(name)
assert not fail,fail
print(f'{len(GOOD)} native and {len(BAD)} rejection controls passed')

# Exercise the same global-address rule through the explicit LowIR adapter.
with tempfile.TemporaryDirectory(prefix='pa15-global-address-') as td:
 for valid,ret in [(True,'ptr'),(False,'i32')]:
  src=Path(td)/'source.lowir';out=Path(td)/'roundtrip.lowir'
  src.write_text('global @data : i32 = 7\nfunction @get() -> '+ret+' {block ^entry: return '+ret+' @data}\n')
  r=subprocess.run([ROOT/'dev/lowir','-o',out,src],capture_output=True,text=True)
  assert (r.returncode==0)==valid,(ret,r.returncode,r.stderr)
 src.write_text('global @data = {i32 7 i32 0}\nfunction @main() -> i32 {slot $x : obj<8x4> block ^entry: %p = addr $x copyobj 8x4 @data, %p %v = load i32 %p %r = binary sub i32 %v, 7 return i32 %r}\n')
 r=subprocess.run([ROOT/'dev/lowir','-o',out,src],capture_output=True,text=True);assert not r.returncode,r.stderr
 exe=Path(td)/'copy';r=subprocess.run([ROOT/'dev/lowir2native-ref','-O0','-o',exe,out],capture_output=True,text=True);assert not r.returncode,r.stderr
 assert not subprocess.run([exe]).returncode
print('3 global-address LowIR controls passed')
