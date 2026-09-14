#!/usr/bin/env python3
"""Class pattern selection, aliases and abstract declarators: source to native."""
from pathlib import Path
import subprocess, tempfile, sys
ROOT = Path(__file__).resolve().parents[2]
CC = Path(sys.argv[1]).resolve() if len(sys.argv) > 1 else ROOT/'dev/cppgm++'
GOOD = {
 'dependent_bool_binding': '''template<class T,T V>struct Integral{static const T value=V;};
template<bool B>struct Box{static const bool value=B;};
template<class B>struct Use{typedef Box<bool(B::value)> type;};
static_assert(Use<Integral<bool,true>>::type::value,"true");
static_assert(!Use<Integral<bool,false>>::type::value,"false");int main(){return 0;}''',
 'injected_identity': '''template<class T>struct C{};
template<class T>struct C<T*>{typedef C self;};
int main(){C<int*> c;C<int*>::self* p=&c;return p!=&c;}''',
 'repeated_value_parameter': '''template<int A,int B>struct C{static const int value=0;};
template<int N>struct C<N,N>{static const int value=N;};
static_assert(C<7,7>::value==7,"value");
static_assert(C<7,8>::value==0,"mismatch");int main(){return 0;}''',
 'primary_then_partial': '''template<class T>struct C{static const int value=1;};
C<int>*p;static_assert(C<int>::value==1,"primary");
template<class T>struct C<T*>{static const int value=2;};
static_assert(C<char*>::value==2,"partial");int main(){return 0;}''',
 'partial_pack_member': '''template<class...T>struct C{};
template<class T,class...U>struct C<T*,U...>{int f(){return sizeof...(U);}};
int main(){C<int*,char,long> c;return c.f()-2;}''',
 'canonical_alias_defaults': '''template<class T,class U=int>struct P{};
template<class A,class B>struct Equal{static const bool value=false;};
template<class T>struct Equal<T,T>{static const bool value=true;};
using I=int;using A=P<char>;using B=P<char,I>;
static_assert(Equal<A,B>::value,"same");
static_assert(!Equal<const int,int>::value,"cv");
static_assert(Equal<const int,const int>::value,"cv match");
int main(){return 0;}''',
 'selected_member_reuse': '''template<class A,class B>struct C{int f(){return 1;}};
template<class X>struct C<X,X>{X x;int f(){return x+2;}int unused(){return X::missing;}};
int main(){C<int,int>a;a.x=5;C<long,long>b;b.x=7;C<int,long>c;
return a.f()+b.f()+c.f()-17;}''',
 'pointer_ordering': '''template<class T>struct C{static const int value=1;};
template<class T>struct C<T*>{static const int value=2;};
template<class T>struct C<const T*>{static const int value=3;};
static_assert(C<const int*>::value==3,"more specialized");
static_assert(C<int*>::value==2,"pointer");
static_assert(C<int>::value==1,"primary");int main(){return 0;}''',
 'integral_and_dependent_alias': '''template<bool B,class T>struct Choose{typedef char type;};
template<class T>struct Choose<true,T>{typedef T type;};
template<class T>struct Wrap{typedef typename Choose<true,T>::type type;};
static_assert(sizeof(Wrap<long>::type)==sizeof(long),"selected alias");
static_assert(sizeof(Choose<false,long>::type)==1,"primary alias");int main(){return 0;}''',
 'nested_patterns': '''template<class T>struct Box{};
template<class T>struct C{static const int value=1;};
template<class U>struct C<Box<U*>>{static const int value=2;};
static_assert(C<Box<int*>>::value==2,"nested pointer");
static_assert(C<Box<int>>::value==1,"nested mismatch");int main(){return 0;}''',
 'late_pointer_and_explicit': '''template<class T>struct C{static const int value=1;};
C<int*>* p;template<class T>struct C<T*>{static const int value=2;};
template<>struct C<long*>{static const int value=3;};
static_assert(C<int*>::value==2,"late selection");
static_assert(C<long*>::value==3,"explicit wins");int main(){return 0;}''',
 'partial_forward_renamed': '''template<class T>struct C;
template<class U>struct C<U*>;template<class V>struct C<V*>{typedef V type;};
static_assert(sizeof(C<long*>::type)==sizeof(long),"renamed head");int main(){return 0;}''',
 'pack_pattern': '''template<class...T>struct C{static const int value=0;};
template<class T,class...U>struct C<T*,U...>{static const int value=sizeof...(U)+1;};
static_assert(C<int*,char,long>::value==3,"pack");
static_assert(C<int*>::value==1,"empty tail");
static_assert(C<int,char>::value==0,"mismatch");int main(){return 0;}''',
 'alias_overload_and_cast': '''struct X{};
template<class T>struct P{typedef void* type;};
template<class T>struct I{typedef int type;};
template<class T>long f(void(*)(T),typename P<T>::type){return 4;}
template<class T>char f(void(*)(T),typename I<T>::type){return 7;}
static_assert(sizeof(f((void(*)(X))0,1))==sizeof(char),"alias candidate");
int main(){return f((void(*)(X))0,1)+f((void(*)(X))0,(void*)0)-11;}''',
 'abstract_cast_reference': '''int f(int n){return n+1;}
int main(){int (&r)(int)=(int(&)(int))f;return r(6)-7;}''',
}
BAD = {
 'unchanged_primary_pattern': 'template<class T>struct C{};template<class U>struct C<U>{};',
 'partial_default': 'template<class T>struct C{};template<class U=int>struct C<U*>{};',
 'ambiguous_patterns': '''template<class A,class B>struct C{};
template<class T>struct C<T,int>{};template<class T>struct C<int,T>{};
C<int,int> x;''',
 'duplicate_renamed_pattern': '''template<class T>struct C{};
template<class A>struct C<A*>{};template<class B>struct C<B*>{};''',
 'nondeducible_parameter': '''template<class T>struct C{};
template<class A,class B>struct C<A*>{};''',
 'cv_pattern_mismatch': '''template<class T>struct C{};
template<class T>struct C<const T*>{typedef T type;};C<int*>::type x;''',
 'missing_selected_alias': '''template<bool B,class T>struct C{};
template<class T>struct C<true,T>{typedef T type;};C<false,int>::type x;''',
}
failures=[]
with tempfile.TemporaryDirectory(prefix='pa15-class-patterns-') as tmp:
 for name,source in {**GOOD,**BAD}.items():
  src=Path(tmp)/(name+'.cpp');ir=src.with_suffix('.lowir');exe=Path(tmp)/name
  src.write_text(source)
  r=subprocess.run([CC,'--emit-lowir','-O0','--validate-lowir','-o',ir,src],capture_output=True,text=True)
  okay=(r.returncode==0)==(name in GOOD)
  if okay and name in GOOD:
   r=subprocess.run([ROOT/'dev/lowir2native-ref','-O0','-o',exe,ir],capture_output=True,text=True)
   okay=r.returncode==0
   if okay:
    r=subprocess.run([exe],capture_output=True,text=True,timeout=10);okay=r.returncode==0
  print(name,'PASS' if okay else 'FAIL',r.returncode,r.stderr.strip(),flush=True)
  if not okay:failures.append(name)
assert not failures,failures
print(f'{len(GOOD)} native and {len(BAD)} rejection class-pattern controls passed')
