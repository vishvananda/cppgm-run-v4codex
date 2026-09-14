#!/usr/bin/env python3
"""Pack partitions, expansion ownership, storage and native behavior controls."""
from pathlib import Path
import subprocess, tempfile, sys
ROOT=Path(__file__).resolve().parents[2]
CC=Path(sys.argv[1]).resolve() if len(sys.argv)>1 else ROOT/'dev/cppgm++'
CASES={
 'empty_and_prefix': '''template<class... T>int count(int n,T...){return n+sizeof...(T);}
int main(){return count(7)+count(5,1,'a',2L)-15;}''',
 'explicit_values': '''template<unsigned... N>int count(){return sizeof...(N);}
int main(){return count<>()+count<3,5,8>()-3;}''',
 'empty_class': '''template<class... T>struct C{};
static_assert(sizeof(C<>)==1 && sizeof(C<int,char>)==1, "size");int main(){return 0;}''',
 'ref_storage': '''void set(int& a,long& b){a=4;b=8;}
template<class... T>void forward(T&... t){set(t...);}
int main(){int a=1;long b=2;forward(a,b);return a+b-12;}''',
 'same_pack_nested': '''int sum(int a,int b){return a+b;}
template<class... T>int f(T...t){return sum(t...);}
template<class... T>int g(T...t){return f(f(t...)+t...);}
int main(){return g(2,3)-15;}''',
 'independent_nested': '''template<class...>struct List{};
int sum(int a,int b){return a+b;}
template<class U,class... T>int size(T...){return sizeof(U)+sizeof...(T);}
template<class... U,class... T>int f(List<U...>,T... t){return sum(size<U>(t...)...);}
int main(){return f(List<char,long>(),2,3)-13;}''',
 'same_flat_partition': '''template<class...>struct List{};
template<class... U,class... T>int f(List<U...>,T...){return sizeof...(U)*10+sizeof...(T);}
int main(){int total=0;for(int i=0;i<3;++i){total+=f(List<int,int>(),1);total+=f(List<int>(),1,2);}return total-99;}''',
 'expanded_sizeof': '''template<unsigned N>int value(){return N;}
int use(int a){return a;}
template<unsigned... N>int f(){return use(value<N+sizeof...(N)>()...);}
int main(){return f<4>()-5;}''',
 'qualified_types': '''namespace N{struct A{int x;};struct B{long y;};}
template<class... T>struct D:T...{};
int main(){D<N::A,N::B> d;d.x=4;d.y=8;return d.x+d.y-12;}''',
 'brace_array': '''template<int... N>int f(){int a[]={N...,9};return a[0]+a[2];}
int main(){return f<4,6>()-13;}''',
 'brace_object': '''struct P{int a;int b;};template<class... T>int f(T... t){P p{t...};return p.a+p.b;}
int main(){return f(4,9)-13;}''',
 'pack_constructor': '''struct P{int n;P(int a,int b):n(a+b){}};
template<class... T>int f(T... t){P p{3,t...};return p.n;}
int main(){return f(7)-10;}''',
 'ref_return': '''template<class... T>struct L{};
template<class... T>L<T&&...> forward(T&&...){return L<T&&...>();}
int main(){int x;L<int&,long&&> t=forward(x,2L);(void)t;return 0;}''',
 'renamed_head': '''template<class... T>struct C{static int f(int,T...);};
template<class... U>int C<U...>::f(int a,U...){return a+sizeof...(U);}
int main(){return C<>::f(4)+C<int,char>::f(1,2,'a')-7;}''',
 'value_pack_call': '''template<int N>struct L{};template<int N>L<N> make(){return L<N>();}
void use(L<5>,L<8>){};template<int... N>void f(L<N>...t){use(make<N+1>()...);}
int main(){f<4,7>(L<4>(),L<7>());return 0;}''',
}
BAD={
 'unequal_lengths': '''template<class...>struct L{};int sum(int,int);template<class... U,class... T>int f(L<U...>,T...t){return sum((sizeof(U)+t)...);}int main(){return f(L<int>(),1,2);}''',
 'sizeof_nonpack': '''template<class T>int f(){return sizeof...(T);}int main(){return f<int>();}''',
 'nonintegral_pack': '''template<int... N>struct C{};C<int> bad;''',
 'value_in_type_pack': '''template<class... T>struct C{};C<1> bad;''',
}
with tempfile.TemporaryDirectory(prefix='pa15-packs-') as temp:
 for name,source in {**CASES,**BAD}.items():
  p=Path(temp);src=p/(name+'.cpp');ir=p/(name+'.lowir');exe=p/name;src.write_text(source)
  r=subprocess.run([CC,'--emit-lowir','-O0','--validate-lowir','-o',ir,src],capture_output=True,text=True)
  assert (r.returncode==0)==(name in CASES),(name,r.returncode,r.stderr)
  if name in CASES:
   r=subprocess.run([ROOT/'dev/lowir2native-ref','-O0','-o',exe,ir],capture_output=True,text=True)
   assert r.returncode==0,(name,'backend',r.stderr)
   r=subprocess.run([exe],timeout=5);assert r.returncode==0,(name,'runtime',r.returncode,ir.read_text())
  print(name,'pass',flush=True)
print(len(CASES),'native controls;',len(BAD),'rejections passed')
