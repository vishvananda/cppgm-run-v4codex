#!/usr/bin/env python3
"""Explicit selection, retained identity, lexical ownership and native outcomes."""
from pathlib import Path
import subprocess, tempfile, sys
ROOT=Path(__file__).resolve().parents[2]
CC=Path(sys.argv[1]).resolve() if len(sys.argv)>1 else ROOT/'dev/cppgm++'
CASES={
 'class_identity': '''template<int N>struct C{static const int v=1;};
using Early=C<3>;template<>struct C<1+2>{static const int v=7;};
static_assert(Early::v==7,"identity");int main(){return C<3>::v-7;}''',
 'class_lexical_scope': '''typedef long T;template<class T>struct C{};
template<>struct C<int>{T value;};static_assert(sizeof(C<int>)==8,"scope");int main(){return 0;}''',
 'class_forward': '''template<class T>struct C{int x;};template<>struct C<int>;
C<int>* p;template<>struct C<int>{int a[3];};static_assert(sizeof(C<int>)==12,"forward");int main(){return 0;}''',
 'class_self': '''template<class T>struct C;template<>struct C<int>{C* next;int n;};
int main(){C<int> a{};a.next=&a;a.n=9;return a.next->n-9;}''',
 'qualified_class': '''namespace N{template<class T>struct C;}
template<>struct N::C<int>{static const int v=5;};int main(){return N::C<int>::v-5;}''',
 'class_base_conversion': '''struct B{int n;};template<class T>struct M;
template<>struct M<int>:B{};template<class T>struct D:M<T>{};
template<class T>D<T>* nil(){return 0;}int f(B* p){return p?1:0;}
int main(){return f(nil<int>());}''',
 'class_virtual': '''template<class T>struct C;template<>struct C<int>{virtual int f(){return 9;}};
int call(C<int>* p){return p->f();}int main(){C<int> c;return call(&c)-9;}''',
 'function_renamed': '''template<class T>int f(T x){return 1;}
template<>int f<int>(int renamed){return renamed+2;}int main(){return f(5)-7;}''',
 'function_default': '''template<class T>int f(int n=sizeof(T)){return 1;}
template<>int f<int>(int renamed){return renamed;}int main(){return f<int>()-4;}''',
 'function_deduced': '''template<class T>int f(T x){return 1;}
template<>int f(long x){return x+2;}int main(){return f(5L)+f(3)-8;}''',
 'function_default_head': '''template<class T=int>int f(T){return 1;}
template<>int f(long x){return x;}int main(){return f(8L)-8;}''',
 'function_forward': '''template<class T>int f(T){return 1;}template<>int f<int>(int);
int caller(){return f(6);}template<>int f<int>(int arg){return arg;}int main(){return caller()-6;}''',
 'function_address': '''template<class T>int f(T x){return 1;}
template<>int f<int>(int x){return x;}int main(){int(*p)(int)=&f<int>;return p(8)-8;}''',
 'function_nontype': '''template<class T,T N>int f(){return 1;}
template<>int f<int,2+3>(){return 9;}int main(){return f<int,5>()+f<int,6>()-10;}''',
 'function_overload': '''template<class T>int f(T){return 1;}template<class T>int f(T*){return 2;}
template<>int f<int>(int*){return 7;}int main(){int x;return f(&x)+f(x)-8;}''',
 'member_definition': '''template<class T>struct C{int f(){return 1;}};
template<>int C<int>::f(){return 9;}int main(){C<int> c;return c.f()-9;}''',
 'member_static': '''template<class T>struct C{static int n;};template<class T>int C<T>::n=2;
template<>int C<int>::n=7;int main(){return C<int>::n+C<char>::n-9;}''',
 'member_forward': '''template<class T>struct C{static int n;};template<class T>int C<T>::n=2;
template<>int C<int>::n;int use(){return C<int>::n;}template<>int C<int>::n=9;int main(){return use()-9;}''',
 'member_destructor': '''int count;template<class T>struct C{~C();};template<class T>C<T>::~C(){count+=1;}
template<>C<int>::~C(){count+=3;}int main(){{C<int> a;C<char> b;}return count-4;}''',
 'variable_identity': '''template<class T>constexpr int v=sizeof(T);
template<>constexpr int v<int> = 7;static_assert(v<int> == 7 && v<char> == 1,"values");
int main(){return v<int>+v<char>-8;}''',
 'variable_query': '''template<class T>constexpr unsigned long v=sizeof(T);
template<class T>int f(){return v<T>;}int main(){return f<int>()+f<char>()-5;}''',
 'variable_partial': '''template<class T>constexpr int v=1;template<class T>constexpr int v<T&> = 2;
template<class T>constexpr int v<T&&> = 3;int main(){return v<int>+v<int&>+v<int&&>-6;}''',
 'variable_storage': '''template<class T>constexpr int v=sizeof(T)+3;
int main(){const int* a=&v<int>;const int* b=&v<char>;return *a+*b-11;}''',
 'variable_defaults': '''template<class T=int,T N=3>constexpr T v=N+sizeof(T);
static_assert(v<> == 7,"defaults");int main(){return 0;}''',
}
BAD={
 'duplicate_variable': 'template<class T>constexpr int v=0;template<>constexpr int v<int> = 1;template<>constexpr int v<int> = 2;',
 'duplicate_static_member': 'template<class T>struct C{static int n;};template<>int C<int>::n=1;template<>int C<int>::n=2;',
 'missing_member': 'template<class T>struct C{};template<>int C<int>::f(){return 0;}',
 'wrong_member_signature': 'template<class T>struct C{int f(int);};template<>int C<int>::f(char){return 0;}',
 'ordinary_member': 'struct C{int f();};template<>int C::f(){return 0;}',
 'duplicate_class': 'template<class T>struct C;template<>struct C<int>{};template<>struct C<int>{};',
 'late_class': 'template<class T>struct C{};static_assert(sizeof(C<int>)==1,"");template<>struct C<int>{};',
 'incomplete_specialization': 'template<class T>struct C{};template<>struct C<int>;C<int> c;',
 'specialization_assert': 'template<class T>struct C;template<>struct C<int>{static_assert(false,"check");};',
 'wrong_function_signature': 'template<class T>int f(T);template<>long f<int>(int);',
 'missing_function_primary': 'template<>int f<int>(int){return 1;}',
 'duplicate_function': 'template<class T>int f(T);template<>int f<int>(int){return 1;}template<>int f<int>(int){return 2;}',
 'no_inherited_primary_member': 'template<class T>struct C{static const int n=1;};template<>struct C<int>{};static_assert(C<int>::n==1,"");',
}
with tempfile.TemporaryDirectory(prefix='pa15-specialization-') as temp:
 for name,source in {**CASES,**BAD}.items():
  p=Path(temp);src=p/(name+'.cpp');ir=p/(name+'.lowir');exe=p/name;src.write_text(source)
  r=subprocess.run([CC,'--emit-lowir','-O0','--validate-lowir','-o',ir,src],capture_output=True,text=True)
  assert (r.returncode==0)==(name in CASES),(name,r.returncode,r.stderr)
  if name in CASES:
   r=subprocess.run([ROOT/'dev/lowir2native-ref','-O0','-o',exe,ir],capture_output=True,text=True)
   assert r.returncode==0,(name,'backend',r.stderr)
   r=subprocess.run([exe],timeout=5);assert r.returncode==0,(name,'runtime',r.returncode,ir.read_text())
  print(name,'pass',flush=True)
print(len(CASES),'native controls;',len(BAD),'rejection controls passed')
