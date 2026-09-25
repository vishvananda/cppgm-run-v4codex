#!/usr/bin/env python3
"""Scalar conversion facts through templates, reference storage and control flow: CC WORK."""
from pathlib import Path
import sys
import ordering_controls as runner
runner.GOOD={
 'enum_condition':'template<class T>struct P{enum{no=0,yes=7};};int main(){if(P<int>::no)return 1;if(P<long>::yes)return 0;return 2;}',
 'enum_negative':'enum E:long{n=-7,z=0};int main(){if(z)return 1;if(n)return 0;return 2;}',
 'const_integral_condition':'template<class T>struct P{static const long value=sizeof(T);};int main(){if(P<int>::value)return 0;return 1;}',
 'volatile_condition':'volatile int v=0;int main(){if(v)return 1;v=7;if(v)return 0;return 2;}',
 'pointer_prvalue_ref':'template<class T>bool f(T&&p){return p==nullptr;}int main(){return !f((const char*)0)||!f(static_cast<int*>(0));}',
 'pointer_convert_ref':'int f(const char*const&p){return p==nullptr;}int main(){return !f(0L)||!f(nullptr);}',
 'pointer_nonzero_ref':'int x=7;template<class T>int f(T&&p){return *p;}int main(){return f(&x)!=7;}',
 'pointer_alias_ref':'template<class T>using P=T*;template<class T>int f(T&&p){return p!=nullptr;}int main(){return f(P<int>(0));}',
 'pointer_temp_distinct':'bool f(int*const&a,int*const&b){return &a!=&b&&a==b;}int main(){return !f((int*)0,(int*)0);}',
 'pointer_temp_repeated':'int* saved;int f(int*const&p){return p==saved;}int x;int main(){saved=&x;return !f(&x)||f((int*)0);}',
 'pointer_cv_ref':'int x=7;int f(const int*const&p){return *p;}int main(){return f(&x)!=7;}',
 'pointer_base_ref':'struct A{int x;};struct B{int y;};struct D:A,B{};int f(B*const&p){return p->y;}int main(){D d;d.y=7;return f(&d)!=7;}',
 'pointer_null_base_ref':'struct A{int x;};struct B{int y;};struct D:A,B{};int f(B*const&p){return p!=nullptr;}int main(){D*p=nullptr;return f(p);}',
 'scalar_width_ref':'long f(const long&v){return v;}double g(const double&v){return v;}int main(){return f(-7)!=-7||g(7)!=7.;}',
 'scalar_unsigned_ref':'unsigned long f(const unsigned long&v){return v;}int main(){return f(4294967295u)!=4294967295ul;}',
 'scalar_bitfield_ref':'struct B{unsigned x:4;};int f(const int&v){return v;}int main(){B b={7};return f(b.x)!=7;}',
 'cast_bitfield_rref':'struct B{unsigned x:4;};int main(){B b={7};unsigned&&r=static_cast<unsigned&&>(b.x);r=3;return b.x!=7||r!=3;}',
 'cast_bitfield_lref':'struct B{unsigned x:4;};int main(){B b={7};const unsigned&r=static_cast<const unsigned&>(b.x);b.x=3;return r!=7;}',
 'cast_prvalue_ref':'int main(){const double&r=static_cast<const double&>(7);double&&q=static_cast<double&&>(9);return r!=7.||q!=9.;}',
 'cast_function_category':'int f(){return 7;}template<class T>T&&dv();template<class T>char pick(T&);template<class T>long pick(T&&);static_assert(sizeof(pick(static_cast<int(&&)()>(f)))==1,"");int main(){return static_cast<int(&&)()>(f)()!=7;}',
 'alias_cast':'template<class T>struct A{using type=T;};template<class T>using V=typename A<T>::type;int main(){return V<long>(7)!=7;}',
}
D='template<class T>T&&dv();template<class A,class B>struct Same{static const bool value=false;};template<class A>struct Same<A,A>{static const bool value=true;};'
for name,source,target in [('int_double','int','const double&'),('long_double','long','double&&'),('null_pointer','decltype(nullptr)','int*const&'),('pointer_void','int*','void*const&'),('pointer_cv','int*','const int*const&')]:
 runner.GOOD['cast_query_'+name]=D+f'template<class T>auto f(int)->decltype(static_cast<{target}>(dv<T>()));template<class>char f(...);static_assert(Same<decltype(f<{source}>(0)),{target}>::value,"");int main(){{}}'
for name,target,expr,expected in [('double','double','7.0','7.'),('pointer','int*','&value','&value'),('reference','const long&','value','7'),('rref','long&&','static_cast<long&&>(value)','7')]:
 storage='int value=7;' if name=='pointer' else 'long value=7;'
 runner.GOOD['cast_user_query_'+name]=D+storage+f'struct X{{explicit operator {target}(){{return {expr};}}}};template<class T>auto f(T&v)->decltype(static_cast<{target}>(v)){{return static_cast<{target}>(v);}}int main(){{X x;return f(x)!={expected};}}'
runner.GOOD.update({
 'cast_user_effect':'int hits;struct X{explicit operator double(){++hits;return 7.;}};template<class T>auto f(T&v)->decltype(static_cast<double>(v)){return static_cast<double>(v);}int main(){X x;return f(x)!=7.||hits!=1;}',
 'cast_constexpr_query':D+'constexpr int n=7;template<class T>constexpr auto f(T v)->decltype(static_cast<const double&>(v)){return static_cast<const double&>(v);}template<class T>struct P{static const bool value=sizeof(static_cast<const double&>(dv<T>()))==sizeof(double);};static_assert(P<int>::value,"");int main(){}',
 'cast_query_noexcept':D+'template<class T>bool f(){return noexcept(static_cast<const double&>(dv<T>()));}int main(){return f<int>();}',
 'cast_volatile_copy':'int main(){volatile int n=7;const double&r=static_cast<const double&>(n);n=3;return r!=7.;}',
})
runner.BAD={
 'cast_mutable_lref':'int main(){int&x=static_cast<int&>(7);}',
 'cast_cv_drop':'int main(){const int x=7;int&&r=static_cast<int&&>(x);}',
 'cast_const_bitfield':'struct B{unsigned x:4;};int main(){B b={7};unsigned&r=const_cast<unsigned&>(b.x);}',
 'cast_reinterpret_double':'struct X{operator double(){return 7.;}};int main(){X x;return reinterpret_cast<double>(x);}',
}
for name,cast,target in [('reinterpret','reinterpret_cast','double'),('const','const_cast','double'),('reference','static_cast','int&')]:
 runner.GOOD['query_reject_'+name]=D+f'template<class T>auto f(int)->decltype({cast}<{target}>(dv<T>()),char());template<class>long f(...);static_assert(sizeof(f<int>(0))==sizeof(long),"");int main(){{}}'
# cv/category and type variations share the same selected conversion owner.
for target in ('const short&','const long&','const float&','const double&','long&&','double&&'):
 name=target.replace(' ','_').replace('&','r')
 runner.GOOD['converted_reference_'+name]=D+f'template<class T>auto f(T n)->decltype(static_cast<{target}>(n)){{return static_cast<{target}>(n);}}template<class T>auto probe(int)->decltype(static_cast<{target}>(dv<T>()));static_assert(Same<decltype(probe<int>(0)),{target}>::value,"");int main(){{const int n=7;return static_cast<{target}>(n)!=7;}}'
runner.GOOD.update({
 'constexpr_narrow_ref':'static_assert(static_cast<const short&>(65543)==7,"");template<int N>struct A{};A<static_cast<const short&>(65543)> a;int main(){return sizeof(a)!=1;}',
 'constexpr_float_ref':'static_assert(static_cast<const int&>(7.75)==7,"");template<int N>struct A{};A<static_cast<const int&>(7.75)> a;int main(){return sizeof(a)!=1;}',
 'constexpr_query_narrow':'template<class T>struct V{static const int n=static_cast<const short&>(65543);};static_assert(V<int>::n==7,"");int main(){}',
 'constexpr_query_cast_value':'template<int N>struct V{static const int n=N;};template<class T>auto f()->V<static_cast<const short&>(65543)>;static_assert(decltype(f<int>())::n==7,"");int main(){}',
 'constexpr_user_double':'struct X{constexpr explicit operator double()const{return 7.;}};template<class T>constexpr auto f(T x)->decltype(static_cast<double>(x)){return static_cast<double>(x);}static_assert(f(X())==7.,"");int main(){}',
 'cast_query_user_noexcept':'template<class T>T&&dv()noexcept;struct X{explicit operator double()noexcept{return 7.;}};struct Y{explicit operator double(){return 8.;}};template<class T>struct P{static const bool value=noexcept(static_cast<double>(dv<T>()));};static_assert(P<X>::value&&!P<Y>::value,"");int main(){}',
 'cast_query_bitfield':'template<class T>T&&dv();struct X{unsigned n:4;};template<class T>auto f(int)->decltype(static_cast<unsigned&>(dv<T>().n),char());template<class>long f(...);static_assert(sizeof(f<X>(0))==sizeof(long),"");int main(){}',
 'cast_query_bitfield_rref':'template<class T>auto f(T&b)->decltype(static_cast<unsigned&&>(b.n)){return static_cast<unsigned&&>(b.n);}struct X{unsigned n:4;};template<class T>int use(T&b){unsigned&&r=static_cast<unsigned&&>(b.n);r=3;return b.n!=7||r!=3;}int main(){X x={7};return use(x);}',
 'cast_bitfield_signed':'struct X{int n:4;};int main(){X x={-3};const int&r=static_cast<const int&>(x.n);x.n=2;return r!=-3;}',
 'cast_bitfield_volatile':'struct X{volatile unsigned n:4;};int main(){X x={7};volatile unsigned&&r=static_cast<volatile unsigned&&>(x.n);x.n=2;return r!=7;}',
 'cast_query_deleted':'template<class T>T&&dv();struct X{explicit operator double()=delete;};template<class T>auto f(int)->decltype(static_cast<double>(dv<T>()),char());template<class>long f(...);static_assert(sizeof(f<X>(0))==sizeof(long),"");int main(){}',
 'cast_query_private':'template<class T>T&&dv();class X{explicit operator double();};template<class T>auto f(int)->decltype(static_cast<double>(dv<T>()),char());template<class>long f(...);static_assert(sizeof(f<X>(0))==sizeof(long),"");int main(){}',
})
runner.BAD['cast_mutable_bitfield']='struct X{unsigned n:4;};int main(){X x={7};unsigned&r=static_cast<unsigned&>(x.n);}'
runner.BAD['cast_reinterpret_bitfield']='struct X{unsigned n:4;};int main(){X x={7};unsigned&r=reinterpret_cast<unsigned&>(x.n);}'

runner.GOOD.update({
 'constexpr_user_reference_temp':'struct X{constexpr operator int()const{return 65543;}};static_assert(static_cast<const short&>(X())==7,"");int main(){return static_cast<const short&>(X())!=7;}',
 'constexpr_user_query_reference_temp':'struct X{constexpr operator int()const{return 65543;}};template<int N>struct A{static const int value=N;};template<class T>auto f()->A<static_cast<const short&>(T())>;static_assert(decltype(f<X>())::value==7,"");int main(){}',
 'reference_temp_pointer_effect':'int hits;int* get(){++hits;return nullptr;}template<class T>int f(T&&p){return p!=nullptr;}int main(){return f(get())||hits!=1;}',
 'reference_temp_conversion_effect':'int hits;struct X{operator int*(){++hits;return nullptr;}};int f(int*const&p){return p!=nullptr;}int main(){X x;return f(x)||hits!=1;}',
 'reference_default_distinct':'int f(int*const&a=(int*)0,int*const&b=(int*)0){return &a==&b||a!=nullptr||b!=nullptr;}int main(){return f()||f();}',
 'cast_query_inaccessible_base':'template<class T>T&&dv();struct B{};class X:private B{};template<class T>auto f(int)->decltype(static_cast<B&>(dv<T&>()),char());template<class>long f(...);static_assert(sizeof(f<X>(0))==sizeof(long),"");int main(){}',
})
runner.BAD['cast_volatile_bitfield_cv']='struct X{volatile unsigned n:4;};int main(){X x={7};unsigned&&r=static_cast<unsigned&&>(x.n);}'

if __name__=='__main__':sys.exit(0 if runner.run(Path(sys.argv[1]).resolve(),Path(sys.argv[2])) else 1)
