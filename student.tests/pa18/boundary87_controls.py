#!/usr/bin/env python3
"""Class value boundaries: real native behavior, queries and rejected transfers."""
from pathlib import Path
import sys
import ordering_controls as runner
runner.GOOD={
 'ellipsis_lvalue':'struct A{int n;};int f(...){return 7;}int main(){A a={3};return f(a)!=7||a.n!=3;}',
 'ellipsis_const':'struct A{int n;};int f(...){return 7;}int main(){const A a={3};return f(a)!=7;}',
 'ellipsis_prvalue':'struct A{int n;};int f(...){return 7;}int main(){return f(A{3})!=7;}',
 'ellipsis_empty':'struct A{};int f(...){return 7;}int main(){A a;return f(a,A())!=7;}',
 'ellipsis_large':'struct A{long n[20];};int f(...){return 7;}int main(){A a={{3}};return f(a)!=7;}',
 'ellipsis_indirect':'struct A{int n;};int f(int n,...){return n;}int main(){int(*p)(int,...)=f;A a={3};return p(7,a)!=7;}',
 'ellipsis_member':'struct A{int n;};struct F{int f(...){return 7;}int operator()(...){return 8;}};int main(){A a={3};F f;return f.f(a)!=7||f(a)!=8;}',
 'ellipsis_ctor':'struct A{int n;};struct F{int n;F(int x,...):n(x){}};int main(){A a={3};F f(7,a);return f.n!=7;}',
 'ellipsis_overload':'struct A{int n;};int f(A){return 1;}int f(...){return 2;}int main(){A a={3};return f(a)!=1||f(a,a)!=2;}',
 'ellipsis_template':'struct A{int n;};template<class T>int f(int n,T a){return n;}int f(...){return 7;}template<class T>int use(T a){return f(a);}int main(){A a={3};return use(a)!=7;}',
 'ellipsis_fixed_recipe':'struct A{int n;};int f(...){return 7;}A a={3};template<class T>int use(){return f(a);}int main(){return use<int>()!=7||use<long>()!=7;}',
 'ellipsis_copy_effect':'int copies,dead;struct A{int n;A(int x):n(x){}A(const A&a):n(a.n){++copies;}~A(){++dead;}};int f(...){return copies==1&&dead==0;}int main(){A a(3);int n=f(a);return n!=1||copies!=1||dead!=1||a.n!=3;}',
 'ellipsis_move_effect':'int moves,dead;struct A{A(){}A(const A&)=delete;A(A&&){++moves;}~A(){++dead;}};int f(...){return moves==1&&dead==0;}int main(){A a;int n=f(static_cast<A&&>(a));return n!=1||moves!=1||dead!=1;}',
 'ellipsis_prvalue_lifetime':'int made,dead;struct A{A(){++made;}~A(){++dead;}};int f(...){return made==1&&dead==0;}int main(){int n=f(A());return n!=1||made!=1||dead!=1;}',
 'ellipsis_empty_copy_effect':'int copies,dead;struct A{A(){}A(const A&){++copies;}~A(){++dead;}};int f(...){return copies==1&&dead==0;}int main(){A a;int n=f(a);return n!=1||copies!=1||dead!=1;}',
 'ellipsis_query':'struct A{int n;};int f(...)noexcept;template<class T>auto test(T a)->decltype(f(a)){return noexcept(f(a));}int main(){A a={3};return test(a)!=1;}',
 'ellipsis_query_throw':'struct A{A(const A&)noexcept(false);};int f(...)noexcept;template<class T>constexpr bool test(){return noexcept(f(*static_cast<T*>(0)));}int main(){return test<A>();}',
 'ellipsis_query_dtor':'struct A{~A()noexcept(false);};int f(...)noexcept;int main(){return noexcept(f(*static_cast<A*>(0)));}',
 'ellipsis_query_default':'int g();struct A{A(const A&,int=g())noexcept;};int f(...)noexcept;int main(){return noexcept(f(*static_cast<A*>(0)));}',
 'ellipsis_constant':'struct A{int n;};constexpr int f(...){return 7;}static_assert(f(A{3})==7,"class ellipsis value");int main(){return f(A{3})!=7;}',
 'ellipsis_constant_lvalue':'struct A{int n;};constexpr A a={3};constexpr int f(...){return 7;}static_assert(f(a)==7,"class ellipsis lvalue");int main(){return f(a)!=7;}',
 'ellipsis_promotions':'struct A{int n;};int f(...){return 7;}int main(){A a={3};char c=2;float n=3;return f(a,c,n,nullptr)!=7;}',
 'alias_result_pointer':'template<class T>struct Id{typedef T type;};template<class T>struct A{int n;A(int x):n(x){}};template<class T>typename Id<A<T>>::type make(int x){return A<T>(x);}int main(){A<int>(*p)(int)=make<int>;A<int>a=p(7);return a.n!=7;}',
 'alias_result_mix':'template<class T>struct Id{typedef T type;};struct A{int n;A(int x):n(x){}};template<class T>typename Id<T>::type make(int x){return T(x);}A plain(int x){return A(x);}int main(){A(*p)(int)=make<A>;A a=p(7);p=plain;A b=p(9);return a.n!=7||b.n!=9;}',
}
runner.BAD={
 'ellipsis_deleted_copy':'struct A{A(){}A(const A&)=delete;};int f(...){return 0;}int main(){A a;return f(a);}',
 'ellipsis_private_copy':'struct A{A(){}private:A(const A&);};int f(...){return 0;}int main(){A a;return f(a);}',
 'ellipsis_volatile_copy':'struct A{int n;};int f(...){return 0;}int main(){volatile A a;return f(a);}',
 'ellipsis_constant_effect':'int g();struct A{int n;};constexpr int f(...){return 7;}static_assert(f(A{g()})==7,"must evaluate variadic source");',
}
for name in ('ellipsis_copy_effect','ellipsis_move_effect','ellipsis_prvalue_lifetime','ellipsis_empty_copy_effect'):
 source=runner.GOOD[name]
 prefix,body=source.split('int main()')
 runner.GOOD[name+'_template']=prefix+'template<class T>int use()'+body+'int main(){return use<int>();}'
runner.GOOD.update({
 'ellipsis_branch':'int copies,dead;struct A{A(){}A(const A&)noexcept{++copies;}~A()noexcept{++dead;}};A a;int f(...)noexcept{return 7;}int use(bool b){return b?f(a):3;}int main(){int x=use(false);int y=use(true);return x!=3||y!=7||copies!=1||dead!=1;}',
 'ellipsis_logical':'int copies,dead;struct A{A(){}A(const A&)noexcept{++copies;}~A()noexcept{++dead;}};A a;int f(...)noexcept{return 7;}bool use(bool b){return b&&f(a);}int main(){bool x=use(false);bool y=use(true);return x||!y||copies!=1||dead!=1;}',
 'ellipsis_multiple':'int copies,dead;struct A{A(){}A(const A&)noexcept{++copies;}~A()noexcept{++dead;}};A a;int f(...)noexcept{return copies-dead;}int main(){int x=f(a,a);return x!=2||copies!=2||dead!=2;}',
 'conversion_explicit':'struct A{int n;};struct X{A a;template<class T>operator T()const{return a;}};int main(){X x={{7}};A a=x.operator A();return a.n!=7;}',
 'floating_zero_results':'struct A{float a;double b;long double c;};A seed={0};struct X{A a;template<class T>operator T()const{return a;}}const x={};float negative=-0.0f;int main(){A a=x;return a.a!=0||a.b!=0||a.c!=0||seed.a!=0||seed.b!=0||seed.c!=0||1.0f/negative>0;}',
 'ellipsis_constexpr_copy':'struct A{int n;constexpr A(int x):n(x){}constexpr A(const A&a):n(a.n+1){}};constexpr int f(...){return 7;}constexpr A a(3);static_assert(f(a)==7,"copy into ellipsis");int main(){return f(a)!=7;}',
 'ellipsis_query_dormant':'template<class T>struct A{A(const A&)noexcept{T::missing();}};int f(...)noexcept;template<class T>constexpr bool test(){return noexcept(f(*static_cast<A<T>*>(0)));}int main(){return !test<int>();}',
 'ellipsis_fixed_nontrivial':'int copies,dead;struct A{A(){}A(const A&){++copies;}~A(){++dead;}};A a;int f(...){return copies-dead;}template<class T>int use(){return f(a);}int main(){int x=use<int>();int y=use<long>();return x!=1||y!=1||copies!=2||dead!=2;}',
 'ellipsis_default':'struct A{int n;};A a={3};int f(...){return 7;}template<class T>int use(int x=f(a)){return x;}int main(){return use<int>()!=7||use<long>()!=7;}',
 # [conv.lval]/2 suppresses class copying in the unevaluated decltype operand.
 'ellipsis_query_sfinae':'struct A{A(const A&)=delete;};int f(...)noexcept;template<class T>auto test(T*p)->decltype(f(*p),char()){return 1;}int test(...){return 2;}int main(){return test(static_cast<A*>(0))!=1;}',
 'ellipsis_query_volatile':'struct A{};template<class T>T&&declval();long f(...);static_assert(sizeof(f(declval<volatile A&>()))==sizeof(long),"unevaluated ellipsis");int main(){}',
 'ellipsis_query_deleted':'struct A{A(const A&)=delete;};template<class T>T&&declval();long f(...);static_assert(sizeof(f(declval<A&>()))==sizeof(long),"unevaluated ellipsis");int main(){}',
})
# Native catch execution belongs to PA21; preserve the exploratory inputs and
# their failed observations without claiming that the PA18 surface implements it.
LATER={
 'ellipsis_throw_cleanup':'int copies,dead;struct A{A(){}A(const A&){++copies;}~A(){++dead;}};int f(...){throw 3;}int main(){A a;try{f(a);}catch(int x){return x!=3||copies!=1||dead!=1;}return 2;}',
 'ellipsis_throw_copy':'int dead;struct A{A(){}A(const A&){throw 3;}~A(){++dead;}};int f(...){return 2;}int main(){A a;try{f(a);}catch(int x){return x!=3||dead!=0;}return 2;}',
}
if __name__=='__main__':sys.exit(0 if runner.run(Path(sys.argv[1]).resolve(),Path(sys.argv[2])) else 1)
