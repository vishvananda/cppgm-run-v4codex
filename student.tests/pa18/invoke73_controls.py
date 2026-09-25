#!/usr/bin/env python3
"""Callable facts in queries, constant evaluation and ordinary LowIR: CC WORK."""
from pathlib import Path
import sys
import ordering_controls as runner
SAME='template<class A,class B>struct Same{static const bool value=false;};template<class A>struct Same<A,A>{static const bool value=true;};'
DECL='template<class T>T&&dv();template<class...>using Void=void;'
DETECT=DECL+'template<class F,class A,class=void>struct Has{static const bool value=false;};template<class F,class A>struct Has<F,A,Void<decltype(__builtin_invoke(dv<F>(),dv<A>()))>>{static const bool value=true;};'
FORWARD='template<class T>struct R{using type=T;};template<class T>struct R<T&>{using type=T;};template<class T>struct R<T&&>{using type=T;};template<class T>constexpr T&&fw(typename R<T>::type&x){return static_cast<T&&>(x);}'
runner.GOOD={
 'direct':'int f(int n){return n+1;}int main(){return __builtin_invoke(f,3)!=4;}',
 'pointer':'int f(int n){return n+1;}int main(){int(*p)(int)=f;return __builtin_invoke(p,3)!=4;}',
 'reference':'int f(int n){return n+1;}int main(){int(&r)(int)=f;return __builtin_invoke(r,3)!=4;}',
 'zero':'int f(){return 3;}int main(){return __builtin_invoke(f)!=3;}',
 'void':'void f(int&n){n=7;}int main(){int n=0;__builtin_invoke(f,n);return n!=7;}',
 'reference_result':'int&f(int&n){return n;}int main(){int n=0;__builtin_invoke(f,n)=7;return n!=7;}',
 'member':'struct F{int n;int operator()(int a){return n+a;}};int main(){F f={2};return __builtin_invoke(f,3)!=5;}',
 'member_template':'struct F{template<class T>T operator()(T a){return a;}};int main(){F f;return __builtin_invoke(f,3)!=3||__builtin_invoke(f,4L)!=4;}',
 'qualifiers':'struct F{int operator()(int)&{return 1;}int operator()(int)&&{return 2;}int operator()(int)const&{return 3;}};int main(){F f;const F c;return __builtin_invoke(f,0)!=1||__builtin_invoke(F(),0)!=2||__builtin_invoke(c,0)!=3;}',
 'surrogate':'int add(int n){return n+2;}struct F{using P=int(*)(int);operator P()const{return add;}};int main(){F f;return __builtin_invoke(f,3)!=5;}',
 'surrogate_effect':'int calls;int add(int n){return n+2;}struct F{using P=int(*)(int);operator P(){++calls;return add;}};F&get(F&f){++calls;return f;}int main(){F f;return __builtin_invoke(get(f),3)!=5||calls!=2;}',
 'receiver_effect':'int count;struct F{int operator()(int n){return n;}};F&get(F&f){++count;return f;}int main(){F f;return __builtin_invoke(get(f),3)!=3||count!=1;}',
 'callee_effect':'int count;int add(int n){return n+2;}using P=int(*)(int);P get(){++count;return add;}int main(){return __builtin_invoke(get(),3)!=5||count!=1;}',
 'default_member':'int count;int def(){++count;return 7;}struct F{int operator()(int n=def()){return n;}};int main(){F f;return __builtin_invoke(f)!=7||count!=1;}',
 'default_direct':'int f(int n=7){return n;}int main(){return __builtin_invoke(f)!=7;}',
 'class_result':'struct X{int n;};X f(int n){X x={n};return x;}int main(){X x=__builtin_invoke(f,7);return x.n!=7;}',
 'pack_forwarding':FORWARD+'template<class F,class...A>auto call(F&&f,A&&...a)->decltype(__builtin_invoke(fw<F>(f),fw<A>(a)...)){return __builtin_invoke(fw<F>(f),fw<A>(a)...);}int add(int a,int b){return a+b;}int zero(){return 4;}int main(){return call(add,2,3)!=5||call(zero)!=4;}',
 'function_query':SAME+DECL+'using P=long(*)(int);static_assert(Same<decltype(__builtin_invoke(dv<P>(),1)),long>::value,"");int main(){}',
 'category_query':SAME+DECL+'struct F{template<class T>T&&operator()(T&&)const;};static_assert(Same<decltype(__builtin_invoke(dv<F>(),dv<int&>())),int&>::value,"");static_assert(Same<decltype(__builtin_invoke(dv<F>(),dv<const long&>())),const long&>::value,"");int main(){}',
 'detector_invalid':DETECT+'struct G{int operator()(int);};struct B{};static_assert(Has<G,int>::value&&!Has<B,int>::value&&!Has<int,int>::value,"");int main(){}',
 'detector_deleted':DETECT+'struct B{int operator()(int)=delete;};static_assert(!Has<B,int>::value,"");int main(){}',
 'detector_private':DETECT+'class B{int operator()(int);};static_assert(!Has<B,int>::value,"");int main(){}',
 'detector_explicit_surrogate':DETECT+'struct B{using P=int(*)(int);explicit operator P();};static_assert(!Has<B,int>::value,"");int main(){}',
 'detector_ref':DETECT+'struct B{int operator()(int)&;};static_assert(!Has<B,int>::value&&Has<B&,int>::value,"");int main(){}',
 'detector_ambiguous':DETECT+'struct B{int operator()(long);int operator()(double);};static_assert(!Has<B,int>::value,"");int main(){}',
 'detector_void':DETECT+'using F=void(*)(int);static_assert(Has<F,int>::value&&!Has<F,int*>::value,"");int main(){}',
 'empty_pack_query':DECL+'template<class V,class...A>struct H{static const bool value=false;};template<class...A>struct H<Void<decltype(__builtin_invoke(dv<A>()...))>,A...>{static const bool value=true;};static_assert(!H<void>::value,"");int main(){}',
 'dormant_member':SAME+DECL+'struct F{template<class T>int operator()(T){return T::missing;}};static_assert(Same<decltype(__builtin_invoke(dv<F>(),1)),int>::value,"");int main(){}',
 'noexcept':'struct F{int operator()(int=3)const noexcept{return 2;}};static_assert(noexcept(__builtin_invoke(F())),"");int main(){}',
 'throwing_conversion':'struct A{operator int();};struct F{int operator()(int)noexcept;};static_assert(!noexcept(__builtin_invoke(F(),A())),"");int main(){}',
 'constexpr_direct':'constexpr int add(int a){return a+2;}static_assert(__builtin_invoke(add,3)==5,"");int main(){}',
 'constexpr_pointer':'constexpr int add(int a){return a+2;}constexpr int(*p)(int)=add;static_assert(__builtin_invoke(p,3)==5,"");int main(){}',
 'constexpr_member':'struct F{constexpr int operator()(int n)const{return n+2;}};static_assert(__builtin_invoke(F(),3)==5,"");int main(){}',
 'constexpr_reference':FORWARD+'constexpr int add(int n){return n+2;}template<class F>constexpr int call(F&&f){return fw<F>(f)(3);}static_assert(call(add)==5,"");int main(){}',
 'fixed_direct':'int f(int n){return n+2;}template<class T>int call(T){return __builtin_invoke(f,3);}int main(){return call(0)!=5||call(0L)!=5;}',
 'fixed_pointer':'int f(int n){return n+2;}template<class T>int call(T,int(*p)(int)){return __builtin_invoke(p,3);}int main(){return call(0,f)!=5||call(0L,f)!=5;}',
 'fixed_object':'struct F{int operator()(int n){return n+2;}};template<class T>int call(T,F&f){return __builtin_invoke(f,3);}int main(){F f;return call(0,f)!=5||call(0L,f)!=5;}',
 'fixed_surrogate':'int f(int n){return n+2;}struct F{using P=int(*)(int);operator P(){return f;}};template<class T>int call(T,F&f){return __builtin_invoke(f,3);}int main(){F f;return call(0,f)!=5||call(0L,f)!=5;}',
 'shadow':'int __builtin_invoke(int n){return n+2;}int main(){return __builtin_invoke(3)!=5;}',
}
runner.GOOD.update({
 'conversion_private':DETECT+'class A{operator int();};struct F{int operator()(int);};static_assert(!Has<F,A>::value,"");int main(){}',
 'conversion_deleted':DETECT+'struct A{operator int()=delete;};struct F{int operator()(int);};static_assert(!Has<F,A>::value,"");int main(){}',
 'conversion_ctor_private':DETECT+'class A{A(int);};struct F{int operator()(A);};static_assert(!Has<F,int>::value,"");int main(){}',
 'conversion_ctor_deleted':DETECT+'struct A{A(int)=delete;};struct F{int operator()(A);};static_assert(!Has<F,int>::value,"");int main(){}',
 'surrogate_private':DETECT+'class F{using P=int(*)(int);operator P();};static_assert(!Has<F,int>::value,"");int main(){}',
 'inherited_using':DETECT+'struct B{int operator()(int n){return n;}};class F:private B{public:using B::operator();};static_assert(Has<F,int>::value,"");int main(){F f;return __builtin_invoke(f,7)!=7;}',
 'inherited_private':DETECT+'struct B{int operator()(int);};class F:private B{};static_assert(!Has<F,int>::value,"");int main(){}',
 'conversion_private_base':DETECT+'struct B{};class D:private B{};struct F{int operator()(B&);};static_assert(!Has<F,D&>::value,"");int main(){}',
 'fixed_defaults':'int c;int d(){++c;return 7;}struct F{int operator()(int n=d()){return n;}};template<class T>int call(T,F&f){return __builtin_invoke(f);}int main(){F f;return call(0,f)!=7||call(0L,f)!=7||c!=2;}',
 'fixed_class_result':'struct X{int n;};X f(int n){X x={n};return x;}template<class T>X call(T){return __builtin_invoke(f,7);}int main(){return call(0).n!=7||call(0L).n!=7;}',
 'constexpr_surrogate':'constexpr int add(int n){return n+2;}struct F{using P=int(*)(int);constexpr operator P()const{return add;}};static_assert(__builtin_invoke(F(),3)==5,"");int main(){}',
 'overload_and_surrogate':'int add(int n){return 9;}struct F{using P=int(*)(long);operator P();int operator()(int n){return n;}};int main(){F f;return __builtin_invoke(f,3)!=3;}',
 'constant_query_ref':FORWARD+'constexpr int f(int n){return n+2;}template<int N>struct X{static const int n=N;};template<class F>using T=X<__builtin_invoke(fw<F>(f),3)>;int main(){return T<int(&)(int)>::n!=5;}',
})

runner.GOOD.update({
 'constexpr_fixed_pointer':'constexpr int add(int n){return n+2;}template<class T>constexpr int call(T,int(*p)(int)){return __builtin_invoke(p,3);}static_assert(call(0,add)==5&&call(0L,add)==5,"");int main(){}',
 'constexpr_fixed_object':'struct F{constexpr int operator()(int n)const{return n+2;}};template<class T>constexpr int call(T,const F&f){return __builtin_invoke(f,3);}static_assert(call(0,F())==5&&call(0L,F())==5,"");int main(){}',
 'constexpr_fixed_surrogate':'constexpr int add(int n){return n+2;}struct F{using P=int(*)(int);constexpr operator P()const{return add;}};template<class T>constexpr int call(T,const F&f){return __builtin_invoke(f,3);}static_assert(call(0,F())==5&&call(0L,F())==5,"");int main(){}',
 'surrogate_noexcept':'struct F{using P=int(*)(int);operator P()const noexcept;};static_assert(!noexcept(__builtin_invoke(F(),3)),"");int main(){}',
})

runner.BAD={
 'empty':'int main(){__builtin_invoke();}',
 'not_callable':'int main(){__builtin_invoke(3,2);}',
 'deleted':'struct F{int operator()(int)=delete;};int main(){F f;return __builtin_invoke(f,0);}',
 'private':'class F{int operator()(int);};int main(){F f;return __builtin_invoke(f,0);}',
 'const':'struct F{int operator()(int){return 1;}};int main(){const F f;return __builtin_invoke(f,0);}',
 'wrong_arity':'int f(int);int main(){return __builtin_invoke(f);}',
 'bad_fixed':'template<class T>int f(T){return __builtin_invoke(3,2);}int main(){}',
 'type_operand':'int main(){__builtin_invoke(int,3);}',
}
if __name__=='__main__':sys.exit(0 if runner.run(Path(sys.argv[1]).resolve(),Path(sys.argv[2])) else 1)
