#!/usr/bin/env python3
"""Pack partitions, expansion ownership, storage and native behavior controls."""
from pathlib import Path
import subprocess, tempfile, sys
ROOT=Path(__file__).resolve().parents[2]
CC=Path(sys.argv[1]).resolve() if len(sys.argv)>1 else ROOT/'dev/cppgm++'
CASES={
 'explicit_pack_prefix': """template<class...T>int f(T...){return sizeof...(T);}
 int main(){int(*p)(int,char)=f<int,char>;return f<int>(1)+f(1)+f<int>(2.5,'a')+f<int,char>(1,'a')+p(1,'a')-8;}""",
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
CASES.update({
 'base_lifetime_order': """int trace;template<int I>struct B{B(){trace=trace*10+I;}~B(){trace=trace*10+I;}};
 template<int...I>struct D:B<I>...{D():B<I>()...{}};
 int main(){{D<1,2> d;if(trace!=12)return 1;}return trace!=1221;}""",
 'placement_default': """typedef unsigned long size_t;void* operator new(size_t,void* p){return p;}
 template<class T>T&& declval();struct B{int x;B():x(7){}};
 template<class T,class... A,class=decltype(::new(declval<void*>()) T(declval<A>()...))>
 T* make(T* p,A&&...a){return ::new((void*)p) T(static_cast<A&&>(a)...);}
 int main(){char data[sizeof(B)];B* b=make((B*)data);return b->x-7;}""",
 'literal_elements': r'''static_assert(L"ab"[2]==0 && u"cd"[1]==100 && U"ef"[0]==101,"units");
 template<int I>struct C{static const int v=L"abc"[I];};
 static_assert(C<1>::v==98 && 1["ab"]=='b',"queries");int main(){return 0;}''',
 'literal_cooked': """int operator""_c(unsigned long long n){return (int)n;}
 template<char...C>int operator""_c(){return 99;}
 int main(){return 035_c+0x10_c-45;}""",
 'literal_float': """long double operator""_f(long double n){return n*2;}
 int main(){return 1.25_f==2.5L?0:1;}""",
 'literal_char': """int operator""_c(char c){return c-'a';}
 int main(){return 'd'_c-3;}""",
 'literal_raw': """int operator""_r(const char* p){return p[0]=='0'&&p[1]=='x'&&p[2]=='f'&&p[3]==0?0:1;}
 int main(){return 0xf_r;}""",
 'literal_template': """template<char...C>int operator""_n(){return sizeof...(C);}
 int main(){return 0X007_n+1.2e3_n+99999999999999999999999999_n-36;}""",
 'literal_reference': """int n;int& operator""_r(unsigned long long){return n;}
 int main(){1_r=7;return n-7;}""",
 'literal_object': """struct B{int x;B(int v):x(v){}};
 B operator""_b(unsigned long long n){return B(n);}
 int main(){B b=4_b;return b.x-4;}""",
})
# Equal flattened arguments with 64 distinct boundaries, each used twice.
source='template<class...>struct L{};template<class...A,class...B>int partition(L<A...>,B...){return sizeof...(A)*100+sizeof...(B);}'
source+='int main(){int sum=0;'
for n in range(64):
 args=','.join(['int']*n); tail=','.join(['0']*(64-n))
 source+=f'sum+=partition(L<{args}>(),{tail});sum+=partition(L<{args}>(),{tail});'
source+=f'return sum-{2*sum(n*100+64-n for n in range(64))};'+'}'
CASES['partition_growth']=source
BAD={
 'literal_not_constant': 'int operator""_x(unsigned long long n){return n+7;}static_assert(2_x==2,"not constant");',
 'literal_bounds': 'static_assert(L"ab"[3]==0,"past end");',
 'literal_ambiguous': 'int operator""_x(const char*);template<char...C>int operator""_x();int main(){return 2_x;}',
 'literal_cooked_overflow': 'int operator""_x(unsigned long long);int main(){return 999999999999999999999_x;}',
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
