#!/usr/bin/env python3
"""Independent final audit of PA18 deduction and shared call-boundary consumers."""
from pathlib import Path
import sys
import ordering_controls as runner

# Expectations follow N3485 [temp.deduct], [temp.inst], [expr.call],
# [conv.lval], [class.temporary] and [expr.unary.noexcept]. No live compiler oracle.
runner.GOOD = {
    'ellipsis_copy_default_effect': 'int defaults,copies,dead;int next(){return ++defaults;}struct A{int n;A():n(9){}A(const A&a,int k=next()):n(a.n+k){++copies;}~A(){++dead;}};int f(...)noexcept{return defaults*10+copies;}int main(){A a;int x=f(a);return x!=11||defaults!=1||copies!=1||dead!=1||a.n!=9;}',
    'ellipsis_default_temporary': 'int marks,copies,dead;struct M{~M(){++marks;}};int arg(const M&){return 3;}struct A{A(){}A(const A&,int=arg(M())){++copies;}~A(){++dead;}};int f(...){return marks+dead;}int main(){A a;int x=f(a);return x||marks!=1||copies!=1||dead!=1;}',
    'ellipsis_volatile_copy': 'int copies,dead;struct A{int n;A():n(4){}A(volatile A&a):n(a.n){++copies;}~A(){++dead;}};int f(...){return copies-dead;}int main(){volatile A a;int x=f(a);return x!=1||copies!=1||dead!=1;}',
    'ellipsis_indirect_lifetime': 'int copies,dead;struct A{A(){}A(const A&){++copies;}~A(){++dead;}};int f(int x,...){return x+copies-dead;}int main(){A a;int(*p)(int,...)=f;int x=p(7,a);return x!=8||copies!=1||dead!=1;}',
    'ellipsis_nested_branch': 'int copies,dead;struct A{A(){}A(const A&){++copies;}~A(){++dead;}};A a;int f(...){return copies-dead;}int use(bool b,bool c){return b?(c?f(a):f(a,a)):0;}int main(){int x=use(false,true),y=use(true,false),z=use(true,true);return x||y!=2||z!=1||copies!=3||dead!=3;}',
    'ellipsis_sequence_cleanup': 'int copies,dead;struct A{A(){}A(const A&){++copies;}~A(){++dead;}};A a;int f(...){return copies-dead;}int main(){int x=(f(a),f(a));return x!=2||copies!=2||dead!=2;}',
    'ellipsis_prvalue_conditional': 'int made,copied,dead;struct A{A(){++made;}A(const A&){++copied;}~A(){++dead;}};int f(...){return made+copied*100;}int main(){bool b=true;int x=f(b?A():A());return x!=1||made!=1||copied||dead!=1;}',
    'ellipsis_dormant_nested_default': 'template<class T>struct A{A(const A&,int=sizeof(typename T::missing))noexcept;};template<class T>T& source()noexcept;long f(...)noexcept;template<class T>auto size()->decltype(sizeof(f(source<A<T>>()))){return sizeof(f(source<A<T>>()));}int main(){return size<int>()!=sizeof(long);}',
    'ellipsis_query_then_execution': 'int copies,dead;struct A{A(){}A(const A&)noexcept{++copies;}~A()noexcept{++dead;}};int f(...)noexcept{return copies-dead;}template<class T>int call(T&v){static_assert(noexcept(f(v)),"quiet copy");return f(v);}int main(){A a;int x=call(a);return x!=1||copies!=1||dead!=1;}',
    'ellipsis_constexpr_default_query': 'struct A{int n;constexpr A(int k):n(k){}constexpr A(const A&a,int k=2):n(a.n+k){}};constexpr A a(3);constexpr int f(...){return 7;}template<int N=f(a)>struct V{static const int n=N;};static_assert(V<>::n==7,"value query");int main(){return V<>::n!=7;}',
    'sfinae_body_separation': 'template<class T>struct R{typedef T type;static int bad(){return T::missing;}};template<class T>typename R<T>::type f(T x){return x;}int main(){return f(7)!=7;}',
    'alias_pointer_cv_deduction': 'template<class T>using P=T*;template<class T>int f(P<const T>){return 1;}template<class T>int f(T){return 2;}int main(){const int x=7;return f(&x)!=1;}',
    'overload_nondeduced_consistency': 'int g(int x){return x;}long g(long x){return x;}template<class T>T f(T(*p)(T),T x){return p(x);}int main(){return f(g,7)!=7||f(g,9L)!=9;}',
    'address_explicit_pack': 'template<class...T>int f(T...){return sizeof...(T);}int main(){int(*p)(int,long)=f<int>;return p(1,2)!=2;}',
    'conversion_reference_selection': 'int value=13;struct S{template<class T>operator T&()const{return value;}};int main(){S s;int&r=s;return &r!=&value||r!=13;}',
    'nttp_alias_identity': 'int a=11,b=17;template<int*P>struct V{static int get(){return *P;}};template<int*P>using W=V<P>;int main(){return W<&a>::get()!=11||W<&b>::get()!=17;}',
    'braced_array_repeated_bound': 'template<class T,int N>int f(const T(&a)[N],const T(&b)[N]){return a[N-1]+b[0];}int main(){return f({2,3},{5,7})!=8;}',
    'canonical_alias_result_indirect': 'template<class T>using Id=T;struct A{double x;A(double n):x(n){}};template<class T>Id<T> make(double n){return T(n);}int main(){A(*p)(double)=make<A>;A a=p(7.5);return a.x!=7.5;}',
    'late_default_declaration': 'template<class T>int f(T,int);template<class T>int f(T,int x=7){return x;}int main(){return f(3)!=7;}',
}
runner.BAD = {
    'ellipsis_deleted_move': 'struct A{A(){}A(const A&){}A(A&&)=delete;};int f(...);int main(){A a;return f(static_cast<A&&>(a));}',
    'ellipsis_required_bad_default': 'template<class T>struct A{A(){}A(const A&,int=sizeof(typename T::missing)){} };int f(...);int main(){A<int>a;return f(a);}',
    'ellipsis_private_destructor_prvalue': 'struct A{A(){}private:~A(){} };int f(...);int main(){return f(A());}',
    'sfinae_hard_body': 'template<class T>int f(T){return T::missing;}int f(...){return 1;}int main(){return f(0);}',
    'deduced_bound_conflict': 'template<class T,int N>int f(const T(&)[N],const T(&)[N]){return N;}int main(){return f({1,2},{3});}',
    'return_nondeduced_call': 'template<class T>T f(){return T();}int main(){return f();}',
}

# N3485 [temp.deduct.call]/1 leaves braced array operands non-deduced;
# [dcl.fct.default]/4 permits later defaults only for non-template functions.
# Keep the original exploratory sources as required rejections.
for name in ('braced_array_repeated_bound', 'late_default_declaration'):
    runner.BAD[name] = runner.GOOD.pop(name)

# [temp.arg.explicit]/9: a supplied pack prefix remains extensible. Independent
# deductions of the completed pack must still agree, including nested types.
runner.GOOD.update({
    'pack_prefix_call': 'template<class...T>int f(T...){return sizeof...(T);}int main(){return f<int*,float*>(0,0,7)!=3;}',
    'pack_prefix_reference_address': 'template<class...T>int f(T...){return sizeof...(T);}int main(){int(&p)(int,long)=f<int>;return p(1,2)!=2;}',
    'pack_prefix_function_argument': 'template<class...T>int f(T...){return sizeof...(T);}int apply(int(*p)(int,long)){return p(1,2);}int main(){return apply(f<int>)!=2;}',
    'pack_prefix_nested_match': 'template<class...T>struct L{};template<class...T>int f(L<T...>,T...){return sizeof...(T);}int main(){return f<int>(L<int,long>(),1,2L)!=2;}',
    'pack_prefix_nested_only': 'template<class...T>struct L{};template<class...T>int f(L<T...>){return sizeof...(T);}int main(){return f<int>(L<int,long>())!=2;}',
    'pack_prefix_function_type': 'int g(int,long){return 7;}template<class...T>int f(int(*p)(T...),T...v){return p(v...);}int main(){return f<int>(g,1,2L)!=7;}',
    'pack_prefix_unused': 'template<class...T>int f(int){return sizeof...(T);}int main(){int(*p)(int)=f<int,long>;return f<int,long>(0)!=2||p(0)!=2;}',
    'pack_prefix_result': 'template<class...T>struct L{int n;};template<class...T>L<T...> f(T...){return L<T...>{sizeof...(T)};}int main(){L<int,long>(*p)(int,long)=f<int>;return p(1,2).n!=2;}',
    'pack_prefix_nontype': 'template<int N>struct I{};template<int...N>int f(I<N>...){return sizeof...(N);}int main(){return f<1>(I<1>(),I<2>())!=2;}',
    'pack_prefix_member': 'struct S{template<class...T>int f(T...){return sizeof...(T);}template<class...T>static int g(T...){return sizeof...(T);}};int main(){S s;int(*p)(int,long)=S::g<int>;return s.f<int>(1,2L)!=2||p(1,2)!=2;}',
    'pack_prefix_query_constant': 'template<class...T>constexpr int f(T...){return sizeof...(T);}template<class T,int N=f<int>(1,T())>struct V{static const int n=N;};static_assert(V<long>::n==2,"extended constexpr pack");int main(){return V<long>::n!=2;}',
    'pack_prefix_address_constant': 'template<class...T>constexpr int f(T...){return sizeof...(T);}constexpr int(*p)(int,long)=f<int>;static_assert(p(1,2)==2,"extended address pack");int main(){return p(1,2)!=2;}',
    'pack_prefix_overloaded_address': 'template<class...T>int f(T...){return 1;}template<class T>int f(T,int){return 2;}int main(){int(*p)(int,long)=f<int>;return p(1,2)!=1;}',
    'pack_prefix_conflict_sfinae': 'template<class...T>struct L{};template<class...T>int f(L<T...>,T...){return 1;}template<class...T>int f(...){return 2;}int main(){return f<int>(L<int>(),1,2L)!=2;}',
    'pack_prefix_longer_deduction_sfinae': 'template<class...T>struct L{};template<class...T>int f(L<T...>,T...){return 1;}template<class...T>int f(...){return 2;}int main(){return f<int>(L<int,long>(),1)!=2;}',
})
runner.BAD.update({
    'pack_prefix_too_many': 'template<class...T>int f(T...){return sizeof...(T);}int main(){return f<int,long>(1);}',
    'pack_prefix_wrong_target': 'template<class...T>int f(T...){return sizeof...(T);}int main(){int(*p)(long,int)=f<int>;return p(1,2);}',
    'pack_prefix_nested_conflict': 'template<class...T>struct L{};template<class...T>int f(L<T...>,T...){return sizeof...(T);}int main(){return f<int>(L<int>(),1,2L);}',
    'pack_prefix_nested_type_conflict': 'template<class...T>struct L{};template<class...T>int f(L<T...>){return sizeof...(T);}int main(){return f<int>(L<long,long>());}',
    'pack_prefix_nontype_conflict': 'template<int N>struct I{};template<int...N>int f(I<N>...){return sizeof...(N);}int main(){return f<1>(I<2>(),I<3>());}',
    'pack_prefix_fixed_ref': 'template<class...T>int f(T&&...){return sizeof...(T);}int main(){int x=1;return f<int>(x,2);}',
})

# Recheck the same boundaries through retained fixed call recipes. Each wrapper
# is demanded twice, with distinct contextual object and lifetime identities.
for name in ('ellipsis_copy_default_effect', 'ellipsis_default_temporary',
             'ellipsis_volatile_copy', 'ellipsis_indirect_lifetime',
             'ellipsis_query_then_execution'):
    prefix, body = runner.GOOD[name].split('int main()')
    reset = 'copies=dead=0;'
    if name == 'ellipsis_copy_default_effect': reset += 'defaults=0;'
    if name == 'ellipsis_default_temporary': reset += 'marks=0;'
    runner.GOOD[name+'_fixed'] = prefix+'template<class T>int use()'+body+'int main(){int x=use<int>();'+reset+'return x||use<long>();}'

if __name__ == '__main__':
    sys.exit(0 if runner.run(Path(sys.argv[1]).resolve(),Path(sys.argv[2])) else 1)
