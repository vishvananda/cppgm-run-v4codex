#!/usr/bin/env python3
"""Array completion, constant storage and observable fallback: CC WORK."""
from pathlib import Path
import json,re,sys
import ordering_controls as runner
runner.GOOD={
 'unknown_nested_flat':'int main(){int a[][2]={1,2,3,4};return sizeof(a)!=16||a[1][1]!=4;}',
 'unknown_nested_mixed':'int main(){int a[][2]={{1},2,3,{4,5}};return sizeof(a)!=24||a[0][1]!=0||a[1][1]!=3||a[2][1]!=5;}',
 'unknown_struct_flat':'struct P{int x,y;};int main(){P a[]={1,2,3,4};return sizeof(a)!=16||a[1].y!=4;}',
 'unknown_struct_copy':'struct P{int x,y;};int main(){P p={3,4};P a[]={p,1,2};return sizeof(a)!=16||a[0].y!=4||a[1].y!=2;}',
 'unknown_string':'int main(){char a[]="abc";return sizeof(a)!=4||a[3]!=0;}',
 'unknown_braced_string':'int main(){char a[]={"abc"};return sizeof(a)!=4||a[2]!=99||a[3]!=0;}',
 'unknown_parenthesized_string':'int main(){char a[]("abc");char b[]=("abc");return sizeof(a)!=4||sizeof(b)!=4||b[2]!=99;}',
 'template_parenthesized_string':'template<class T>int f(){char a[]=("abc");return sizeof(a);}int main(){return f<void>()!=4;}',
 'array_value_member':'struct X{int a[3];X():a(){}};int main(){X x;return x.a[0]||x.a[2];}',
 'parenthesized_scalar_clauses':'int main(){int a[]={(1),(2),(3)};return sizeof(a)!=12||a[2]!=3;}',
 'parenthesized_pack_clauses':'template<int...N>int f(){int a[]={(N+1)...};return sizeof(a);}int main(){return f<1,2,3>()!=12;}',
 'unknown_string_rows':'int main(){char a[][4]={"abc","de"};return sizeof(a)!=8||a[0][3]!=0||a[1][2]!=0;}',
 'unknown_wide_string':'int main(){wchar_t a[]={L"ab"};return sizeof(a)!=12||a[1]!=98||a[2]!=0;}',
 'unknown_pack':'template<int...N>int f(){int a[]={N...};return sizeof(a)/sizeof(int);}int main(){return f<3,4,5>()!=3;}',
 'unknown_pack_rows':'template<int...N>int f(){int a[][2]={N...};return sizeof(a)/sizeof(int);}int main(){return f<1,2,3,4>()!=4;}',
 'unknown_pack_sentinel':'template<int...N>int f(){int a[]={7,N...};return sizeof(a)/sizeof(int);}int main(){return f<>()!=1||f<3,4>()!=3;}',
 'unknown_unused_pack':'template<int...N>void f(){int a[]={N...};}int main(){}',
 'unknown_static':'int a[][2]={1,2,3,4};static_assert(sizeof(a)==16,"");int main(){return a[1][1]!=4;}',
 'unknown_constexpr':'constexpr int a[][2]={1,2,3,4};static_assert(sizeof(a)==16&&a[1][1]==4,"");int main(){}',
 'unknown_prior_bound':'extern int a[3];int a[]={1};int main(){return sizeof(a)!=12||a[2]!=0;}',
 'unknown_self_address':'void* a[]={&a};int main(){return sizeof(a)!=8||a[0]!=&a;}',
 'unknown_class_ctor':'struct C{int x;C(int n):x(n){}};int main(){C a[]={1,2};return sizeof(a)!=8||a[1].x!=2;}',
 'unknown_empty_elements':'struct E{};int main(){E a[]={{},{}};return sizeof(a)!=2||&a[0]==&a[1];}',
 'array_identity':'int main(){int a[]={1,2},b[]={1,2};a[0]=7;return a==b||b[0]!=1||a[0]!=7;}',
 'array_wide':'int main(){long a[]={1L,2L};return a[0]+a[1]!=3;}',
 'array_constexpr_conversion':'struct C{constexpr operator int()const{return 7;}};int main(){int a[]={C(),C()};return a[0]+a[1]!=14;}',
 'array_effectful_conversion':'int hits;struct C{operator int(){return ++hits;}};int main(){int a[]={C(),C()};return hits!=2||a[0]!=1||a[1]!=2;}',
 'array_volatile':'int main(){volatile long a[]={1L,2L};return a[0]+a[1]!=3;}',
 'array_pointer_identity':'int x=3;int main(){int*p[]={&x,nullptr};return *p[0]!=3||p[1]!=nullptr;}',
 'union_constant_order':'struct tag{};template<class T>union U{unsigned char dummy;T value;constexpr U(tag):dummy(){}template<class...A>constexpr U(A...a):value(a...) {}};extern const U<int> u;int observe(){return u.value;}int before=observe();constexpr U<int> u=U<int>(7);int main(){return before!=7||u.value!=7;}',
 'union_active_address':'union U{int x;long y;constexpr U(long n):y(n){}};constexpr U u(17L);constexpr const long*p=&u.y;int main(){return *p!=17||p!=&u.y;}',
}
runner.GOOD.update({
 'template_fixed_bound':'template<class T>int f(){int a[][2]={1,2,3,4};return sizeof(a);}int main(){return f<void>()!=16;}',
 'template_braced_string':'template<class T>int f(){char a[]={"abc"};return sizeof(a);}int main(){return f<void>()!=4;}',
 'template_query_bound':'template<int...N>int f(){int a[]={N...};typedef decltype(a) A;A b={};return sizeof(b);}int main(){return f<1,2>()!=8||f<1,2,3>()!=12;}',
 'template_bound_overload':'template<int N>int size(int(&)[N]){return N;}template<int...N>int f(){int a[]={N...};return size(a);}int main(){return f<1,2>()!=2||f<1,2,3>()!=3;}',
 'template_dependent_struct':'template<class T>int f(){T a[]={1,2,3,4};return sizeof(a);}struct P{int a,b;};int main(){return f<P>()!=16||f<int>()!=16;}',
 'template_static_array':'template<class T>struct X{static constexpr int a[]={1,2,3};};template<class T>constexpr int X<T>::a[];int main(){return sizeof(X<void>::a)!=12||X<void>::a[2]!=3;}',
 'template_static_array_bound':'template<class T>struct X{static constexpr int a[]={1,2,3};};template<class T>constexpr int X<T>::a[3];int main(){return X<void>::a[2]!=3;}',
 'template_static_dependent_bound':'template<int N>struct X{static const int a[N];};template<int N>const int X<N>::a[]={7};int main(){return sizeof(X<3>::a)!=12||X<3>::a[2]!=0;}',
 'template_static_bound_renamed':'template<int N>struct X{static const int a[N];};template<int M>const int X<M>::a[M]={7};int main(){return sizeof(X<3>::a)!=12||X<3>::a[2]!=0;}',
 'template_ctor_dormant'  :'template<class T>struct C{C(int){T::missing();}};template<class T>void f(){C<T> a[]={1,2};}int main(){}',
 'template_pointer_rows':'template<int...N>int f(){int a[][2]={N...};int(*p)[2]=a;return p[1][1];}int main(){return f<1,2,3,4>()!=4;}',
})
runner.BAD={
 'unknown_empty':'int main(){int a[]={};}',
 'unknown_empty_pack':'template<int...N>void f(){int a[]={N...};}int main(){f<>();}',
 'unknown_nested_empty':'int main(){int a[][2]={};}',
 'unknown_braced_string_excess':'int main(){char a[]={"abc","def"};}',
 'template_static_array_conflict':'template<class T>struct X{static constexpr int a[]={1,2,3};};template<class T>constexpr int X<T>::a[2];int main(){return X<void>::a[0];}',
 'template_static_dependent_conflict':'template<int N>struct X{static const int a[N];};template<int N>const int X<N>::a[N+1]={7};',
 'empty_aggregate_scalar':'struct E{};int main(){E a[]={1};}',
 'empty_aggregate_template_scalar':'struct E{};template<class T>void f(){E a[]={1};}int main(){}',
 'trailing_narrow_after_pack':'template<int...N>void f(){int a[]={N...,1.5};}int main(){}',
 'narrow_pack_pattern':'template<int...N>void f(){int a[]={(N+0.5)...};}int main(){f<1>();}',
 'unknown_scalar_initializer':'int main(){int a[]=1;}',
 'unknown_paren_initializer':'int main(){int a[](1,2);}',
 'known_scalar_initializer':'int main(){int a[2]=1;}',
 'known_paren_initializer':'int main(){int a[2](1,2);}',
 'template_scalar_initializer':'template<class T>void f(){int a[]=1;}int main(){}',
 'parenthesized_string_excess':'int main(){char a[]={("abc"),"def"};}',
 'template_parenthesized_string_excess':'template<class T>void f(){char a[]={("abc"),"def"};}int main(){}',
 'unknown_narrow':'int main(){int a[]={1.5};}',
 'union_inactive':'union U{int x;long y;constexpr U():y(7){}};constexpr U u;static_assert(u.x==7,"");',
}
if __name__=='__main__':
 cc=Path(sys.argv[1]).resolve();work=Path(sys.argv[2]);ok=runner.run(cc,work)
 checks=[]
 for name,copies in [('array_identity',2),('array_wide',1),('array_constexpr_conversion',1),('array_effectful_conversion',0),('array_volatile',0),('unknown_braced_string',1),('unknown_nested_flat',1)]:
  p=work/(name+'.lowir');count=len(re.findall(r'^    copyobj ',p.read_text(),re.M)) if p.exists() else -1
  checks.append(dict(name=name,expected=copies,actual=count,passed=count==copies))
 (work/'storage.json').write_text(json.dumps(checks,indent=2)+'\n')
 for c in checks:print(c)
 sys.exit(0 if ok and all(c['passed'] for c in checks) else 1)
