#!/usr/bin/env python3
"""Class completion versus immediate inherited lookup failure: CC WORK."""
from pathlib import Path
import sys
import ordering_controls as runner
V='template<class...>using Void=void;'
TYPE=V+'template<class T,class=void>struct Has{static const bool value=false;};template<class T>struct Has<T,Void<typename T::type>>{static const bool value=true;};'
BASES='struct L{using type=int;};struct R{using type=long;};struct D:L,R{};'
runner.GOOD={
 'ambiguous_partial':TYPE+BASES+'static_assert(!Has<D>::value,"");int main(){}',
 'ambiguous_return':BASES+'template<class T>typename T::type f(int);template<class T>char f(...);static_assert(sizeof(f<D>(0))==1,"");int main(){}',
 'ambiguous_default':BASES+'template<class T,class=typename T::type>long f(int);template<class>char f(...);static_assert(sizeof(f<D>(0))==1,"");int main(){}',
 'ambiguous_alias':TYPE+BASES+'template<class T>using A=typename T::type;template<class T,class=A<T>>long f(int);template<class>char f(...);static_assert(sizeof(f<D>(0))==1,"");int main(){}',
 'template_base':TYPE+'template<class T>struct L{using type=T;};template<class T>struct R{using type=T*;};template<class T>struct D:L<T>,R<T>{};static_assert(!Has<D<int>>::value,"");int main(){}',
 'nested_base':TYPE+'template<class T>struct O{struct L{using type=T;};struct R{using type=T*;};struct D:L,R{};struct Bad{using X=typename T::missing;};};static_assert(!Has<O<int>::D>::value,"");int main(){}',
 'hidden':TYPE+BASES+'struct H:D{using type=char;};static_assert(Has<H>::value,"");int main(){}',
 'using_hides':TYPE+BASES+'struct H:D{using L::type;};static_assert(Has<H>::value,"");int main(){}',
 'ambiguous_branch':TYPE+BASES+'struct E{using type=char;};struct F:D,E{};static_assert(!Has<F>::value,"");int main(){}',
 'repeated_base_identity':TYPE+'struct A{using type=int;};struct L:A{};struct R:A{};struct D:L,R{};static_assert(Has<D>::value,"");int main(){}',
 'virtual_base_identity':TYPE+'struct A{using type=int;};struct L:virtual A{};struct R:virtual A{};struct D:L,R{};static_assert(Has<D>::value,"");int main(){}',
 'private_type':TYPE+'class A{using type=int;};static_assert(!Has<A>::value,"");int main(){}',
 'private_base':TYPE+'struct A{using type=int;};class D:private A{};static_assert(!Has<D>::value,"");int main(){}',
 'owner_isolation':TYPE+BASES+'struct H:D{using type=int;};static_assert(!Has<D>::value&&Has<H>::value&&!Has<R*>::value&&Has<L>::value,"");int main(){}',
 'pointer_dormant':TYPE+'template<class T>struct O{struct type{using X=typename T::missing;};};static_assert(Has<O<int>>::value,"");int main(){}',
 'field_query':'struct L{int n;};struct R{int n;};struct D:L,R{};template<class T>auto f(int)->decltype(((T*)0)->n,char());template<class>long f(...);static_assert(sizeof(f<D>(0))==sizeof(long),"");int main(){}',
 'static_query':'struct L{static const int n=1;};struct R{static const int n=2;};struct D:L,R{};template<class T>auto f(int)->decltype(T::n,char());template<class>long f(...);static_assert(sizeof(f<D>(0))==sizeof(long),"");int main(){}',
 'enum_query':'struct L{enum{n=1};};struct R{enum{n=2};};struct D:L,R{};template<class T>auto f(int)->decltype(T::n,char());template<class>long f(...);static_assert(sizeof(f<D>(0))==sizeof(long),"");int main(){}',
 'static_hidden':'struct L{static const int n=1;};struct R{static const int n=2;};struct D:L,R{static const int n=3;};template<class T>auto f(int)->decltype(T::n,char());template<class>long f(...);static_assert(sizeof(f<D>(0))==1,"");int main(){}',
 'repeat_negative':TYPE+BASES+''.join('static_assert(!Has<D>::value,"");' for _ in range(100))+'int main(){}',
}
runner.BAD={
 'ordinary_alias':BASES+'using X=D::type;',
 'ordinary_value':'struct L{static const int n=1;};struct R{static const int n=2;};struct D:L,R{};int n=D::n;',
 'class_side_effect':TYPE+BASES+'template<class T>struct B:T{using X=typename T::type;using type=int;};static_assert(!Has<B<D>>::value,"");',
 'nested_side_effect':TYPE+'template<class T>struct O{struct N{using X=typename T::missing;using type=int;};};static_assert(!Has<O<int>::N>::value,"");',
 'template_body':BASES+'template<class T>void f(){typename T::type x;}int main(){f<D>();}',
 'fixed_lookup':BASES+'template<class T>void f(){D::type x;}',
 'no_fallback':BASES+'template<class T>typename T::type f(int);int main(){f<D>(0);}',
}
if __name__ == '__main__':
 sys.exit(0 if runner.run(Path(sys.argv[1]).resolve(),Path(sys.argv[2])) else 1)
