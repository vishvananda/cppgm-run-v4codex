#!/usr/bin/env python3
"""Cross-handoff ownership reducers for the accumulated PA17 audit."""
from pathlib import Path
import sys
import entity_controls as runner

CONST_RECEIVER = 'struct B{int n;constexpr B(int x):n(x){}constexpr int f()const{return n;}};struct L:B{constexpr L(int x):B(x){}};struct R:B{constexpr R(int x):B(x){}};template<class T>struct D:L,T{constexpr D():L(3),T(7){}constexpr int g()const{return T::f();}};'

runner.GOOD = {
 'query_nested_template_qualifier': 'namespace n{template<class U>struct B{constexpr int f()const{return 7;}};}template<class T>struct D:n::B<int>{};template<class T>auto read(T const&t)->decltype(t.n::B<int>::f()){return t.n::B<int>::f();}int main(){D<char>x;return read(x)!=7;}',
 'deferred_free_template_emission': 'struct X{static constexpr int f(int n){return n+1;}};template<class T>constexpr int g(int n){return X::f(n);}static_assert(g<int>(2)==3,"value");int main(){volatile int n=2;return g<int>(n)!=3;}',
 'deferred_cycle': 'struct X{static constexpr int f(int n){return n?g(n-1):0;}static constexpr int g(int n){return n?f(n-1):0;}};static_assert(X::f(4)==0,"value");int main(){volatile int n=4;return X::f(n);}',
 'query_constexpr_receiver': CONST_RECEIVER+'template<int N>struct Tag{static const int n=N;};static_assert(Tag<(D<R>().R::f())>::n==7,"query path");int main(){}',
 'query_constexpr_field': CONST_RECEIVER+'template<int N>struct Tag{static const int n=N;};static_assert(Tag<(D<R>().R::n)>::n==7,"query field path");int main(){}',
 'query_nested_constexpr_receiver': CONST_RECEIVER+'template<int N>struct Tag{static const int n=N;};static_assert(Tag<(D<R>().g())>::n==7,"query path");int main(){}',
 'query_dependent_qualifier': CONST_RECEIVER+'template<class T>auto read(T const&t)->decltype(t.R::f()){return t.R::f();}int main(){D<R>x;return read(x)!=7;}',
 'deferred_member_emission': 'template<class T>struct X{static constexpr int f(int n){return n+1;}static constexpr int g(int n){return f(n);}};static_assert(X<int>::g(2)==3,"value");int main(){volatile int n=2;return X<int>::g(n)!=3;}',
 'deferred_use_after_demand': 'template<class T>struct X{static constexpr int f(int n){return n+1;}static constexpr int g(int n){return f(n);}};int run(int n){return X<int>::g(n);}static_assert(X<int>::g(2)==3,"value");int main(){volatile int n=2;return run(n)!=3;}',
 'deferred_unused_definition': 'template<class T>struct X{static constexpr int bad(){return T::missing;}static constexpr int g(int n){return n?7:bad();}};static_assert(X<int>::g(1)==7,"value");int main(){return 0;}',
 'deferred_unevaluated_operand': 'template<class T>struct X{static constexpr int bad(){return T::missing;}static constexpr int g(){return sizeof(bad());}};static_assert(X<int>::g()==4,"value");int main(){return X<int>::g()!=4;}',
 'deferred_transitive_emission': 'template<int N>constexpr int step(int n){return n+N;}template<class T>struct X{static constexpr int f(int n){return step<3>(n);}static constexpr int g(int n){return f(n);}};static_assert(X<int>::g(2)==5,"value");int main(){volatile int n=2;return X<int>::g(n)!=5;}',
 'constexpr_qualified_receivers': 'struct B{int n;constexpr B(int x):n(x){}constexpr int f()const{return n;}};struct L:B{constexpr L(int x):B(x){}};struct R:B{constexpr R(int x):B(x){}};template<class T>struct D:L,T{constexpr D():L(3),T(7){}constexpr int g()const{return T::f();}};constexpr D<R>x;static_assert(x.g()==7 && x.L::f()==3 && x.R::f()==7,"qualified constant receivers");int main(){return x.g()!=7;}',
 'query_added_default': 'template<class T,class U>long f(T);char f(...);using Before=decltype(f(1));template<class T,class U=int>long f(T);using After=decltype(f(1));static_assert(sizeof(Before)==1 && sizeof(After)==8,"defaults publication");int main(){}',
 'query_added_overload': 'char f(...);using Before=decltype(f(1));long f(int);using After=decltype(f(1));static_assert(sizeof(Before)==1 && sizeof(After)==8,"declaration publication");int main(){}',
 'qualified_arrow_receiver': 'struct B{int n;int f(){return n;}};struct L:B{};struct R:B{};template<class T>struct D:L,T{};int main(){D<R>x;static_cast<R&>(x).n=7;D<R>*p=&x;return p->R::f()!=7;}',
 'qualified_alias_receiver': 'struct B{int n;int f(){return n;}};struct L:B{};struct R:B{};template<class T>struct D:L,T{using base=T;int g(){return base::f();}};int main(){D<R>x;static_cast<R&>(x).n=7;return x.g()!=7;}',
 'qualified_virtual_suppression': 'struct B{virtual int f(){return 3;}};struct L:B{};struct R:B{};template<class T>struct D:L,T{int f(){return 7;}};int main(){D<R>x;return x.R::f()!=3||x.f()!=7;}',
 'qualified_overload_cv': 'struct B{int f(){return 3;}int f()const{return 7;}};struct L:B{};struct R:B{};template<class T>struct D:L,T{};int main(){D<R>x;const D<R>&y=x;return x.R::f()!=3||y.R::f()!=7;}',
 'qualified_static_repeated_base': 'struct B{static int f(){return 7;}};struct L:B{};struct R:B{};template<class T>struct D:L,T{};int main(){D<R>x;return x.B::f()!=7;}',
 'exposed_base_address': 'struct B{static int f(){return 7;}};class D:private B{public:using B::f;};template<class T>struct X{using P=decltype(&D::f);static P p;};template<class T>typename X<T>::P X<T>::p=&D::f;int main(){return X<int>::p()!=7;}',
 'dependent_private_address_friend': 'class B{static int f(){return 7;}template<class>friend struct X;};template<class T>struct X{using P=decltype(&T::f);static P p;};template<class T>typename X<T>::P X<T>::p=&T::f;int main(){return X<B>::p()!=7;}',
 'qualified_repeated_base': 'struct B{int n;int f(){return n;}};struct L:B{};struct R:B{};template<class T>struct D:L,T{};int main(){D<R>x;static_cast<L&>(x).n=3;static_cast<R&>(x).n=7;return x.L::f()!=3||x.R::f()!=7;}',
 'fixed_qualified_repeated_base': 'struct B{int n;int f(){return n;}};struct L:B{};struct R:B{};struct D:L,R{};template<class T>int run(D&x){return x.L::f()+x.R::f();}int main(){D x;static_cast<L&>(x).n=3;static_cast<R&>(x).n=7;return run<int>(x)!=10;}',
 'dependent_qualified_repeated_base': 'struct B{int n;int f(){return n;}};struct L:B{};struct R:B{};template<class T>struct D:L,T{int g(){return T::f();}};int main(){D<R>x;static_cast<R&>(x).n=7;return x.g()!=7;}',
 'qualified_const_receiver': 'struct B{int n;int f()const{return n;}};struct L:B{};struct R:B{};template<class T>struct D:L,T{};int main(){D<R>x;static_cast<R&>(x).n=7;const D<R>&y=x;return y.R::f()!=7;}',
 'signature_function_query': 'template<class T>struct X{static decltype(T::f()) n;};template<class U>decltype(U::f()) X<U>::n=7;struct Y{static int f();};int main(){return X<Y>::n!=7;}',
 'signature_parenthesized_query': 'template<class T>struct X{static decltype((T::n)) n;};template<class U>decltype((U::n)) X<U>::n=U::n;struct Y{static int n;};int Y::n=7;int main(){X<Y>::n=9;return Y::n!=9;}',
 'signature_bound_names': 'int g(int);long g(long);template<class T>struct X{static decltype(g(T())) n;};template<class U>decltype(g(U())) X<U>::n=7;int main(){return X<int>::n!=7||X<long>::n!=7;}',
 'member_template_default_holes': 'template<class T>struct X{template<class U=int,class V>int f(V){return sizeof(T)+sizeof(U)+sizeof(V);}};int main(){X<char>x;X<long>y;return x.f(char())!=6||y.f(long())!=20;}',
 'partial_head_default_holes': 'template<class T,class U=int,class V>int f(T,V){return sizeof(T)+sizeof(U)+sizeof(V);}int main(){return f<char>(char(),long())!=13||f<long>(long(),char())!=13;}',
 'private_address_own_context': 'class X{static int f(){return 7;}public:template<class T>static int run(){struct S{decltype(&f) p;};static S s={&f};return s.p();}};int main(){return X::run<int>()!=7;}',
 'private_overload_address_own_context': 'class X{static int f(){return 7;}static int f(int n){return n;}public:template<class T>static int run(){struct S{int(*p)();};static S s={&f};return s.p();}};int main(){return X::run<int>()!=7;}',
}
runner.BAD = {
 'qualified_private_overload': 'struct B{static int f();static int f(int);};class D:private B{};template<class T>int g(){struct S{int(*p)();};static S s={&D::f};return s.p();}int main(){return g<int>();}',
 'private_qualified_query_call': 'struct B{static int f();};class D:private B{};template<class T>struct X{using P=decltype(D::f());};X<int>x;int main(){}',
 'inaccessible_receiver_qualifier': 'struct B{int f(){return 7;}};struct L:B{};struct R:B{};template<class T>class D:public L,private T{};int main(){D<R>x;return x.R::f();}',
 'ambiguous_receiver_qualifier': 'struct B{int f(){return 7;}};struct L:B{};struct R:B{};template<class T>struct D:L,T{};int main(){D<R>x;return x.B::f();}',
 'dependent_private_address': 'struct B{static int f();};class D:private B{};template<class T>struct X{using P=decltype(&T::f);};X<D>x;int main(){}',
 'qualified_private_base_address': 'struct B{static int f(){return 7;}};class D:private B{};template<class T>struct X{using P=decltype(&D::f);};X<int>x;int main(){}',
 'private_address_query': 'class X{static int f();};template<class T>struct Y{using P=decltype(&X::f);};Y<int>x;int main(){}',
 'private_overload_table': 'class X{static int f();static int f(int);};template<class T>int g(){struct S{int(*p)();};static S s={&X::f};return s.p();}int main(){return g<int>();}',
 'signature_distinct_names': 'int f(int);int g(int);template<class T>struct X{static decltype(f(T())) n;};template<class T>decltype(g(T())) X<T>::n;int main(){}',
 'qualified_const_mutation': 'struct B{int f(){return 0;}};struct L:B{};struct R:B{};template<class T>struct D:L,T{};int main(){const D<R>x;return x.R::f();}',
}
if __name__ == '__main__':
 sys.exit(0 if runner.run(Path(sys.argv[1]).resolve(),Path(sys.argv[2])) else 1)
