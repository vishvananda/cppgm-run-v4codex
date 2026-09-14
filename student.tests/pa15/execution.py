#!/usr/bin/env python3
"""PA15 checked constant execution, body validation and storage controls."""
from pathlib import Path
import subprocess,sys,tempfile
ROOT=Path(__file__).resolve().parents[2]
CC=Path(sys.argv[1]).resolve() if len(sys.argv)>1 else ROOT/'dev/cppgm++'
GOOD={
'scalar_frames':'''constexpr int add(int x,int y){return x+y;}template<int N>struct C{static const int n=N;};
static_assert(C<add(2,3)>::n==5,"");static_assert(C<add(7,9)>::n==16,"");
int main(){constexpr int a=add(3,4);constexpr int b=add(9,5);return a+b-21;}''',
'recursive_frames':'''constexpr unsigned f(unsigned n){return n ? n*f(n-1):1;}
static_assert(f(6)==720,"");static_assert(f(4)==24,"");int main(){return f(5)-120;}''',
'short_circuit':'''constexpr bool f(int n){return n && 8/n==4;}
static_assert(!f(0),"");static_assert(f(2),"");int main(){return 0;}''',
'conditional':'''constexpr int f(int n){return n ? 12/n:7;}static_assert(f(0)==7,"");
static_assert(f(3)==4,"");int main(){return 0;}''',
'nested_frames':'''constexpr int f(int x){return x*3;}constexpr int g(int x){return f(x)+f(x+1);}
static_assert(g(2)==15,"");static_assert(g(5)==33,"");int main(){return 0;}''',
'default_argument':'''constexpr int f(int x,int y=7){return x+y;}template<int N>struct C{static const int n=N;};
static_assert(C<f(3)>::n==10,"");int main(){constexpr int x=f(9);return x-16;}''',
'member_static':'''struct A{static constexpr unsigned f(unsigned n){return n<64?n:64;}};
struct B{static constexpr unsigned n=A::f(80);};int main(){return B::n-64;}''',
'out_of_class':'''struct A{static constexpr int f(int);};constexpr int A::f(int n){using I=int;return I(n*2);}
struct B{static constexpr int n=A::f(9);};int main(){return B::n-18;}''',
'function_template':'''template<class T>constexpr T f(T n){return n+1;}template<int N>struct C{static const int n=N;};
static_assert(C<f<int>(3)>::n==4,"");int main(){constexpr int n=f<int>(8);return n-9;}''',
'conversion_frames':'''template<class T,T V>struct K{static const T n=V;constexpr operator T()const{return n;}};
template<bool B>struct C{static const bool n=B;};template<class T>struct Use{typedef C<T{}> type;};
static_assert(Use<K<bool,true>>::type::n,"");static_assert(!Use<K<bool,false>>::type::n,"");int main(){return 0;}''',
'conversion_cast':'''struct K{constexpr operator int()const{return 7;}};
template<int N>struct C{static const int n=N;};static_assert(C<static_cast<int>(K{})>::n==7,"");int main(){return 0;}''',
'conversion_explicit_cast':'''struct K{explicit constexpr operator int()const{return 7;}};
template<int N>struct C{static const int n=N;};static_assert(C<static_cast<int>(K{})>::n==7,"");int main(){return 0;}''',
'conversion_functional_cast':'''struct K{explicit constexpr operator int()const{return 7;}};
template<int N>struct C{static const int n=N;};static_assert(C<int(K{})>::n==7,"");int main(){return 0;}''',
'conversion_direct':'''struct K{explicit constexpr operator int()const{return 7;}};
int main(){constexpr int a=static_cast<int>(K{});constexpr int b=int(K{});return a+b-14;}''',
'conversion_return':'''struct K{constexpr operator int()const{return 7;}};constexpr int f(){return K{};}
int main(){constexpr int n=f();return n-7;}''',
'conversion_parameter':'''struct K{constexpr operator int()const{return 7;}};constexpr int f(int n){return n+2;}
template<int>struct C{};C<f(K{})> c;int main(){constexpr int n=f(K{});return n-9;}''',
'stateless_member_call':'''struct K{constexpr int f(int n)const{return n+3;}};
template<int N>struct C{static const int n=N;};static_assert(C<K{}.f(4)>::n==7,"");
int main(){constexpr int n=K{}.f(5);return n-8;}''',
'limit_then_shallow':'''constexpr int f(int n){return n?f(n-1):0;}const int runtime=f(520);
constexpr int shallow=f(4);int main(){return runtime+shallow;}''',
'definition_then_retry':'''extern const int n;constexpr int f(){return n;}const int runtime=f();
const int n=3;constexpr int later=f();int main(){return later+runtime-6;}''',
'unused_ordinary_complete_class':'''struct A{int f(){static_assert(sizeof(x)==sizeof(int),"");return x;}int x;};int main(){return 0;}''',
'unused_template_dependent':'''template<class T>struct A{int f(){static_assert(sizeof(T)==0,"");return 0;}};A<int> a;int main(){return 0;}''',
'unused_explicit_class':'''template<class T>struct A;template<>struct A<int>{int f(){static_assert(sizeof(int)==4,"");return 3;}};int main(){return 0;}''',
'storage_two_classes':'''template<class T,int N>struct A{static const int value=N;int f(){return T::missing;}};
template<class T,int N>const int A<T,N>::value;A<int,3> a,b;A<char,7> c;int main(){return 0;}''',
'storage_later_definition':'''template<class T>struct A{static const int value=9;};A<int> a;template<class T>const int A<T>::value;int main(){return 0;}''',
}
BAD={
'ordinary_false':'struct A{int f(){static_assert(false,"");return 0;}};int main(){return 0;}',
'ordinary_return':'struct A{int f(){return "bad";}};int main(){return 0;}',
'ordinary_jump':'struct A{int f(){break;return 0;}};int main(){return 0;}',
'explicit_false':'template<class T>struct A;template<>struct A<int>{int f(){static_assert(false,"");return 0;}};int main(){return 0;}',
'called_template_false':'template<class T>struct A{int f(){static_assert(sizeof(T)==0,"");return 0;}};int main(){A<int>a;return a.f();}',
'nonconstexpr_call':'int f(){return 3;}constexpr int n=f();int main(){return n;}',
'mutable_global':'int x=3;constexpr int f(){return x;}constexpr int n=f();int main(){return n;}',
'division_zero':'constexpr int f(int n){return 8/n;}constexpr int n=f(0);int main(){return n;}',
'overflow':'constexpr int f(int n){return n+1;}constexpr int n=f(2147483647);int main(){return n;}',
'same_frame_recursion':'constexpr int f(int n){return f(n);}constexpr int n=f(0);int main(){return n;}',
'growing_recursion':'constexpr int f(int n){return f(n+1);}constexpr int n=f(0);int main(){return n;}',
'nonconstexpr_conversion':'struct K{operator int()const{return 3;}};template<int>struct C{};C<K{}> c;int main(){return 0;}',
'explicit_conversion':'struct K{explicit constexpr operator int()const{return 3;}};template<int>struct C{};C<K{}> c;int main(){return 0;}',
'narrow_conversion':'struct K{constexpr operator int()const{return 256;}};template<unsigned char>struct C{};C<K{}> c;int main(){return 0;}',
'private_conversion':'struct K{private:constexpr operator int()const{return 3;}};template<int>struct C{};C<K{}> c;int main(){return 0;}',
'deleted_conversion':'struct K{constexpr operator int()const=delete;};template<int>struct C{};C<K{}> c;int main(){return 0;}',
'effectful_construction':'int x;struct K{K(){++x;}constexpr operator int()const{return 3;}};template<int>struct C{};C<K{}> c;int main(){return 0;}',
'nonliteral_destruction':'struct K{~K(){}constexpr operator int()const{return 3;}};template<int>struct C{};C<K{}> c;int main(){return 0;}',
}
failed=[]
with tempfile.TemporaryDirectory(prefix='pa15-execution-') as td:
 for name,source in {**GOOD,**BAD}.items():
  src=Path(td)/(name+'.cpp');ir=src.with_suffix('.lowir');exe=src.with_suffix('.exe');src.write_text(source)
  r=subprocess.run([CC,'--emit-lowir','-O0','--validate-lowir','-o',ir,src],capture_output=True,text=True,timeout=20)
  okay=(r.returncode==0)==(name in GOOD)
  if okay and name=='storage_two_classes':okay=sum(line.startswith('global ') for line in ir.read_text().splitlines())==5
  if okay and name=='storage_later_definition':okay=sum(line.startswith('global ') for line in ir.read_text().splitlines())==2
  if okay and name in GOOD:
   r=subprocess.run([ROOT/'dev/lowir2native-ref','-O0','-o',exe,ir],capture_output=True,text=True);okay=r.returncode==0
   if okay:r=subprocess.run([exe],capture_output=True,text=True,timeout=10);okay=r.returncode==0
  print(name,'PASS' if okay else 'FAIL',r.returncode,r.stderr.strip(),flush=True)
  if not okay:failed.append(name)
assert not failed,failed
print(f'{len(GOOD)} native and {len(BAD)} rejection controls passed')
