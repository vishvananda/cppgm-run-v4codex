#!/usr/bin/env python3
"""Correlated outer/inner type and value pack formation: CC WORK."""
from pathlib import Path
import sys
import ordering_controls as runner
GATE='template<bool,class T=void>struct Gate{};template<class T>struct Gate<true,T>{using type=T;};'
HEAD=GATE+'template<class A,class B>struct Pair{};template<class...>struct List{};'
runner.GOOD={}
for outer in range(4):
 for inner in range(4):
  source=HEAD+'template<class...T>struct X{template<class...U,class=List<Pair<T,U>...>>int f(U...){return 1;}int f(...){return 2;}};'
  source+='int main(){X<'+','.join(['int']*outer)+'>x;return x.f('+','.join(['0']*inner)+')!='+str(1 if outer==inner and inner else 2)+';}'
  # An empty argument list chooses the nontemplate ellipsis on an exact tie.
  runner.GOOD[f'correlated_{outer}_{inner}']=source
runner.GOOD.update({
 'single_length_not_element':HEAD+'template<class...T>struct X{template<class...U,class=List<Pair<T,U>...>>int f(U...){return 1;}int f(...){return 2;}};int main(){X<int>x;return x.f(0)!=1||x.f(0,0)!=2||x.f()!=2;}',
 'element_constraint':GATE+'template<class T,class U>using Valid=typename Gate<sizeof(T)==sizeof(U)>::type;template<class...>struct List{};template<class...T>struct X{template<class...U,class=List<Valid<T,U>...>>int f(U...){return 1;}int f(...){return 2;}};int main(){X<int,long>x;return x.f(0,0L)!=1||x.f(0L,0)!=2||x.f(0)!=2;}',
 'nonpack_capture':HEAD+'template<class A,class...T>struct X{template<class...U,class=typename Gate<sizeof(A)==4,List<Pair<T,U>...>>::type>int f(U...){return 1;}int f(...){return 2;}};int main(){X<int,int,long>x;X<long,int,long>y;return x.f(0,0)!=1||y.f(0,0)!=2;}',
 'nested_alias_capture':HEAD+'template<class...T>struct X{template<class...U>using Z=List<Pair<T,U>...>;template<class...U,class=Z<U...>>int f(U...){return 1;}int f(...){return 2;}};int main(){X<int,int>x;return x.f(0,0)!=1||x.f(0)!=2;}',
 'member_result_capture':HEAD+'template<class...T>struct X{template<class...U>List<Pair<T,U>...>f(U...){return {};}long f(...){return 2;}};int main(){X<int,int>x;return sizeof(x.f(0,0))!=1||x.f(0)!=2;}',
 'outer_value_pack':GATE+'template<int,class>struct P{};template<class...>struct L{};template<int...N>struct X{template<class...U,class=L<P<N,U>...>>int f(U...){return 1;}int f(...){return 2;}};int main(){X<1,2>x;return x.f(0,0)!=1||x.f(0)!=2;}',
 'three_packs':HEAD+'template<class...T>struct X{template<class...V>struct Y{template<class...U,class=List<Pair<Pair<T,V>,U>...>>int f(U...){return 1;}int f(...){return 2;}};};int main(){X<int,int>::Y<long,long>x;X<int,int>::Y<long>y;return x.f(0,0)!=1||x.f(0)!=2||y.f(0,0)!=2;}',
 'query_expansion':HEAD+'int g(int,int);template<class...T>struct X{template<class...U,class=decltype(g(Pair<T,U>()...))>int f(U...){return 1;}int f(...){return 2;}};int main(){X<int,int>x;return x.f(0,0)!=2||x.f(0)!=2;}',
})
runner.GOOD.update({
 'query_valid_expansion':HEAD+'template<class...A>int g(A...);template<class...T>struct X{template<class...U,class=decltype(g(Pair<T,U>()...))>int f(U...){return 1;}int f(...){return 2;}};int main(){X<int,int>x;return x.f(0,0)!=1||x.f(0)!=2;}',
 'query_value_expansion':'template<int,class>struct P{};template<class...A>int g(A...);template<int...N>struct X{template<class...U,class=decltype(g(P<N,U>()...))>int f(U...){return 1;}int f(...){return 2;}};int main(){X<1,2>x;return x.f(0,0)!=1||x.f(0)!=2;}',
 'renamed_member_definition':HEAD+'template<class...T>struct X{template<class...U>List<Pair<T,U>...>f(U...);};template<class...A>template<class...B>List<Pair<A,B>...>X<A...>::f(B...){return {};}int main(){X<int,long>x;return sizeof(x.f(0,0))!=1;}',
 'captured_alias_equivalence':HEAD+'template<class...T>struct X{template<class...U>using Z=List<Pair<T,U>...>;template<class...U>Z<U...>f(U...);};template<class...A>template<class...B>List<Pair<A,B>...>X<A...>::f(B...){return {};}int main(){X<int,long>x;return sizeof(x.f(0,0))!=1;}',
})
runner.GOOD.update({
 'prefix_capture':HEAD+'template<class...T>struct X{template<class...U,class=List<Pair<T,U>...>>int f(U...){return 1;}int f(...){return 2;}};template<class...T>struct Y:X<int,T...>{};int main(){Y<long>x;return x.f(0,0)!=1||x.f(0)!=2;}',
 'outer_alias_chain':HEAD+'template<class...T>struct X{template<class...U>using Z=List<Pair<T,U>...>;template<class...U>using W=Z<U...>;template<class...U,class=W<U...>>int f(U...){return 1;}int f(...){return 2;}};int main(){X<int,int>x;return x.f(0,0)!=1||x.f(0)!=2;}',
 'same_pack_nested_expansion':'template<class...>struct L{};template<class A,class B>struct P{};template<class...T>using Z=L<P<T,L<T...>>...>;int main(){static_assert(sizeof(Z<int,long>)==1,"");return 0;}',
})
runner.GOOD.update({
 'explicit_correlated':HEAD+'template<class...K,class...T>List<Pair<K,T>...>make(T...){return {};}template<class...K>long make(...){return 9;}int main(){return sizeof(make<char,long>(0,0L))!=1||make<char>(0,0L)!=9||make<char,long>(0)!=9;}',
 'nested_pack_types':'template<class A,class B>struct Same{static const bool value=false;};template<class A>struct Same<A,A>{static const bool value=true;};template<class...>struct L{};template<class A,class B>struct P{};template<class...T>using Z=L<P<T,L<T...>>...>;int main(){static_assert(Same<Z<int,long>,L<P<int,L<int,long>>,P<long,L<int,long>>>>::value,"");return 0;}',
})
runner.BAD={
 'hard_length':HEAD+'template<class...T>struct X{template<class...U>using Z=List<Pair<T,U>...>;};X<int,int>::Z<int>*p;int main(){}',
}
if __name__=='__main__':sys.exit(0 if runner.run(Path(sys.argv[1]).resolve(),Path(sys.argv[2])) else 1)
