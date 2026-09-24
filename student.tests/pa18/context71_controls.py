#!/usr/bin/env python3
"""Retained lexical/concrete template context and decltype-base controls: CC WORK."""
from pathlib import Path
import sys
import ordering_controls as runner
GATE='template<bool,class T=void>struct Gate{};template<class T>struct Gate<true,T>{using type=T;};template<bool B,class T=void>using Enable=typename Gate<B,T>::type;'
SAME='template<class A,class B>struct Same{static const bool value=false;};template<class A>struct Same<A,A>{static const bool value=true;};'
runner.GOOD={
 'member_result':GATE+'template<class T>struct A{template<class U=T>Enable<sizeof(U)==sizeof(T),T> f(U x){return x;}template<class U>Enable<sizeof(U)!=sizeof(T),T> f(U){return 9;}};int main(){A<int>a;A<long>b;return a.f(3)!=3||a.f(3L)!=9||b.f(4L)!=4||b.f(4)!=9;}',
 'outer_alias_default':GATE+'template<class T>struct A{template<class U>using Valid=Enable<sizeof(U)==sizeof(T)>;template<class U,class=Valid<U>>int f(U){return 3;}int f(...){return 8;}};int main(){A<int>a;A<long>b;return a.f(0)!=3||a.f(0L)!=8||b.f(0)!=8||b.f(0L)!=3;}',
 'outer_alias_result':GATE+'template<class T>struct A{template<class U>using Valid=Enable<sizeof(U)==sizeof(T),T>;template<class U>Valid<U> f(U x){return x;}int f(...){return 8;}};int main(){A<int>a;A<long>b;return a.f(3)!=3||a.f(0L)!=8||b.f(0)!=8||b.f(4L)!=4;}',
 'private_alias_default':GATE+'template<class T>class A{template<class U>using Valid=Enable<sizeof(U)==sizeof(T)>;public:template<class U,class=Valid<U>>int f(U){return 3;}int f(...){return 8;}};int main(){A<int>a;return a.f(0)!=3||a.f(0L)!=8;}',
 'symbolic_static_call':GATE+'template<class T>struct A{template<class U>static constexpr bool ok(){return sizeof(T)==sizeof(U);}template<class U>using Valid=Enable<ok<U>()>;template<class U,class=Valid<U>>int f(U){return 3;}int f(...){return 8;}};int main(){A<int>a;A<long>b;return a.f(0)!=3||a.f(0L)!=8||b.f(0)!=8||b.f(0L)!=3;}',
 'symbolic_static_data':GATE+'template<class T>struct A{static const int n=sizeof(T);template<class U>using Valid=Enable<sizeof(U)==n>;template<class U,class=Valid<U>>int f(U){return 3;}int f(...){return 8;}};int main(){A<int>a;A<long>b;return a.f(0)!=3||a.f(0L)!=8||b.f(0)!=8||b.f(0L)!=3;}',
 'class_pack_size':GATE+'template<class...T>struct A{template<class...U>using Valid=Enable<sizeof...(T)==sizeof...(U)>;template<class...U,class=Valid<U...>>int f(U...){return 3;}int f(...){return 8;}};int main(){A<int,long>a;A<char>b;return a.f(0,0)!=3||a.f(0)!=8||b.f(0)!=3||b.f(0,0)!=8;}',
 'nested_concrete_owner':GATE+'template<class T>struct A{template<class V>struct B{template<class U>Enable<sizeof(U)==sizeof(T),V> f(U){return 3;}V f(...){return 8;}};};int main(){A<int>::B<long>a;A<long>::B<int>b;return a.f(0)!=3||a.f(0L)!=8||b.f(0)!=8||b.f(0L)!=3;}',
 'out_of_class_result':GATE+'template<class T>struct A{template<class U>Enable<sizeof(U)==sizeof(T),T> f(U);};template<class V>template<class W>Enable<sizeof(W)==sizeof(V),V>A<V>::f(W x){return x;}int main(){A<int>a;A<long>b;return a.f(3)!=3||b.f(4L)!=4;}',
 'result_ref':GATE+SAME+'template<class T>struct A{template<class U=T>Enable<sizeof(U)==sizeof(T),T&> f(T&x){return x;}};int main(){int n=3;A<int>a;static_assert(Same<decltype(a.f(n)),int&>::value,"");a.f(n)=7;return n!=7;}',
 'unselected_body':GATE+'template<class T>struct A{template<class U>Enable<sizeof(U)==sizeof(T),int> f(U){return 3;}template<class U>Enable<sizeof(U)!=sizeof(T),int>f(U){return U::missing;}};int main(){A<int>a;return a.f(0)!=3;}',
 'base_decltype':'struct B{int n;};B choose(int);template<class T>struct A:decltype(choose(T())){};int main(){A<int>a;a.n=7;return a.n!=7;}',
 'base_pack_explicit':'template<class...>struct B{static const int n=1;};template<class...T>B<T...>choose(int);template<class...T>struct A:decltype(choose<T...>(0)){};int main(){return A<>::n!=1||A<int,long>::n!=1;}',
 'base_pack_arity':GATE+'struct Yes{static const int n=1;};struct No{static const int n=2;};template<class...T>Enable<sizeof...(T)==2,Yes>choose(int);template<class...>No choose(...);template<class...T>struct A:decltype(choose<T...>(0)){};int main(){return A<>::n!=2||A<int>::n!=2||A<int,long>::n!=1;}',
 'base_call_cv':'template<class T>T&v();struct Yes{static const int n=1;};struct No{static const int n=2;};struct F{int operator()(int);};template<class T>auto choose(int)->decltype(v<T>()(1),Yes());template<class>No choose(...);template<class T>struct A:decltype(choose<T>(0)){};int main(){return A<F>::n!=1||A<const F>::n!=2;}',
 'base_query_dormant':'struct B{static const int n=3;};template<class T>B choose(T){return T::missing;}template<class T>struct A:decltype(choose(T())){};int main(){return A<int>::n!=3;}',
}
runner.BAD={
 'private_alias_use':GATE+'template<class T>class A{template<class U>using Valid=Enable<sizeof(U)==sizeof(T)>;};A<int>::Valid<int>*p;int main(){}',
 'base_nonclass':'template<class T>T choose();template<class T>struct A:decltype(choose<T>()){};A<int>a;int main(){}',
 'base_reference':'struct B{};template<class T>T&choose();template<class T>struct A:decltype(choose<T>()){};A<B>a;int main(){}',
 'base_incomplete':'struct B;template<class T>T choose();template<class T>struct A:decltype(choose<T>()){};A<B>a;int main(){}',
 'base_final':'struct B final{};template<class T>T choose();template<class T>struct A:decltype(choose<T>()){};A<B>a;int main(){}',
}
if __name__=='__main__':sys.exit(0 if runner.run(Path(sys.argv[1]).resolve(),Path(sys.argv[2])) else 1)
