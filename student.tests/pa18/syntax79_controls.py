#!/usr/bin/env python3
"""PA18 declaration prediction and dependent member arguments: CC WORK."""
from pathlib import Path
import sys
import ordering_controls as runner
SAME='template<class A,class B>struct Same{static const bool value=false;};template<class A>struct Same<A,A>{static const bool value=true;};'
runner.GOOD={
 'typedef_parenthesized':'template<class X>struct E{typedef bool(type)(const X&,const X&);};bool eq(const int&a,const int&b){return a==b;}int main(){E<int>::type*p=eq;return !p(2,2);}',
 'typedef_multiple':'struct E{typedef int(a), (b), (*c)(int);};int f(int x){return x;}int main(){E::a a=1;E::b b=2;E::c c=f;return c(a+b)!=3;}',
 'typedef_nested':'template<class X>struct E{typedef X (((type)))(X);};int f(int x){return x;}int main(){E<int>::type*p=f;return p(7)!=7;}',
 'alias_function':'template<class R,class...A>struct E{template<class X>using F=R(A...);F<void>*p;};int f(int x){return x;}int main(){E<int,int>e={f};return e.p(7)!=7;}',
 'alias_nested_function':'template<class X,class Y>struct Pair{using type=X;};template<class R,class...A>struct E{template<class X>using F=Pair<R(A...),X>;R f(A...){return 7;}};int main(){E<int,int>e;return e.f(2)!=7;}',
 'alias_late':'template<class R>struct E{template<class X>R f(X x){F<X>*p=0;return sizeof(p);}template<class X>using F=R(X);};int main(){E<int>e;return e.f(3)!=8;}',
}
for prefix,head in [('namespace','ns::box'),('global','::ns::box'),('alias','alias::box'),('class','ns::outer::box'),('specialization','ns::owner<int>::box')]:
 common='namespace ns{template<class A,class B>struct box{using type=A;};struct outer{template<class A,class B>struct box{using type=A;};};template<class>struct owner{template<class A,class B>struct box{using type=A;};};}namespace alias=ns;template<class T>struct Wrap{using type=T;};'
 runner.GOOD['qualified_'+prefix]=common+'template<class P>int f(P const&,typename Wrap<typename '+head+'<const P&,int>::type>::type p){return p;}int main(){int n=7;return f(n,n)!=7;}'
 runner.GOOD['qualified_cv_'+prefix]=common+'struct E{template<class P>int f(P const&,typename Wrap<typename '+head+'<const P&,int>::type>::type p)const{return p;}};int main(){int n=7;E e;return e.f(n,n)!=7;}'
runner.GOOD.update({
 'relational_argument':'template<bool>struct V{};struct E{typedef V<(1<2)> type;};int main(){E::type t;return sizeof(t)!=1;}',
 'shift_argument':'template<int>struct V{};struct E{using type=V<(8>>2)>;};int main(){E::type t;return sizeof(t)!=1;}',
 'value_hides_type':'struct X{};int X=7;template<int>struct V{static const int n=3;};int main(){return V<(sizeof(X))>::n!=3;}',
 'function_parameter':'template<class T>struct E{typedef int(*P)(T);static int f(P p,T x){return p(x);}};int f(long x){return x;}int main(){return E<long>::f(f,7)!=7;}',
})
P='template<class A,class B>struct Result{using type=A;};struct Provider{template<class A,class B>typename Result<A,B>::type get(){return A(7);}};template<class A,class B>struct Def{using first=A;using second=B;};'
runner.GOOD.update({
 'member_pack':P+'int sum(){return 0;}template<class T,class...A>int sum(T n,A...a){return n+sum(a...);}template<class...D>int f(Provider&p){return sum(p.get<typename D::first,typename D::second>()...);}int main(){Provider p;return f<>(p)!=0||f<Def<int,long>>(p)!=7||f<Def<int,long>,Def<long,int>>(p)!=14;}',
 'member_nonpack':P+'template<class D>int f(Provider&p){return p.get<typename D::first,typename D::second>();}int main(){Provider p;return f<Def<int,long>>(p)!=7||f<Def<long,int>>(p)!=7;}',
 'member_constexpr':'struct P{template<class T>constexpr T f()const{return T(7);}};template<class T>constexpr T f(const P&p){return p.f<T>();}static_assert(f<long>(P())==7,"");int main(){}',
 'member_noexcept':'struct P{template<class T>T f()noexcept(sizeof(T)==sizeof(int)){return T();}};template<class T>bool f(P&p){return noexcept(p.f<T>());}int main(){P p;return !f<int>(p)||f<long>(p);}',
 'member_address':'struct P{template<class T>static T f(){return T(7);}};template<class T>T run(P&p){T(*f)()=p.f<T>;return f();}int main(){P p;return run<int>(p)!=7||run<long>(p)!=7;}',
})
runner.BAD={
 'invalid_qualified_type':'namespace ns{int x;}template<class>struct V{};V<ns::x> v;',
 'member_missing_alias':P+'template<class D>int f(Provider&p){return p.get<typename D::missing,int>();}int main(){Provider p;return f<Def<int,long>>(p);}',
 'member_bad_argument':'struct P{template<class T>T f(){return T::missing;}};template<class T>int f(P&p){return p.f<T>();}int main(){P p;return f<int>(p);}',
}
runner.GOOD.update({
 'member_value_pack':'struct P{template<int N>int get(){return N;}};int sum(){return 0;}template<class T,class...A>int sum(T n,A...a){return n+sum(a...);}template<int...N>int f(P&p){return sum(p.get<N>()...);}int main(){P p;return f<1,2,3>(p)!=6||f<>(p)!=0;}',
 'member_alias_argument':P+'template<class T>using Id=T;template<class D>int f(Provider&p){return p.get<Id<typename D::first>,typename D::second>();}int main(){Provider p;return f<Def<int,long>>(p)!=7;}',
 'member_query_argument':P+'template<class D>auto f(Provider&p)->decltype(p.get<typename D::first,typename D::second>()){return p.get<typename D::first,typename D::second>();}int main(){Provider p;return f<Def<int,long>>(p)!=7;}',
 'member_dormant':'struct P{template<class T>T f(){return T::missing;}};template<class T>int f(P&p){return p.f<T>();}int main(){}',
})
if __name__=='__main__':sys.exit(0 if runner.run(Path(sys.argv[1]).resolve(),Path(sys.argv[2])) else 1)
