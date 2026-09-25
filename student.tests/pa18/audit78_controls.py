#!/usr/bin/env python3
"""Audit 78: retained initialization, allocation and declaration demand controls."""
from pathlib import Path
import sys
import ordering_controls as runner
GOOD = {}
BAD = {}
def detect(name, declarations, expression, accepted=(), rejected=()):
    source = declarations + 'template<class T>auto f(int)->decltype(' + expression + ',char());template<class>long f(...);'
    for t in accepted:
        source += 'static_assert(sizeof(f<' + t + '>(0))==1, "accepted");'
    for t in rejected:
        source += 'static_assert(sizeof(f<' + t + '>(0))==sizeof(long), "discarded");'
    GOOD[name] = source + 'int main(){}'
detect('new_narrow', '', 'new T{1.5}', ('double', 'float'), ('int', 'bool'))
detect('new_integer_range', '', 'new T{256}', ('int', 'double'), ('char', 'bool'))
detect('new_aggregate', 'struct A{int x;};struct B{int x;int y;};', 'new T{1,2}', ('B',), ('A',))
detect('new_nested_aggregate', 'struct A{int x[2];int y;};', 'new T{{1,2},3}', ('A',))
detect('new_string', 'struct A{char x[3];};struct B{char x[2];};', 'new T{"ab"}', ('A',), ('B',))
detect('new_deleted_destructor', 'struct A{A(int);~A()=delete;};', 'new T{1}', ('A',))
detect('new_private_destructor', 'class A{~A();public:A(int);};', 'new T{1}', ('A',))
detect('new_parenthesized_deleted_destructor', 'struct A{A(int);~A()=delete;};', 'new T(1)', ('A',))
detect('temporary_deleted_destructor', 'struct A{A(int);~A()=delete;};', 'T{1}', (), ('A',))
detect('new_subobject_deleted_destructor', 'struct A{~A()=delete;};struct B{A a;};', 'new T{}', (), ('B',))
detect('new_explicit_ctor', 'struct A{explicit A(int);};', 'new T{1}', ('A',))
detect('new_private_ctor', 'class A{A(int);};', 'new T{1}', (), ('A',))
detect('new_deleted_ctor', 'struct A{A(int)=delete;};', 'new T{1}', (), ('A',))
detect('new_poison_body', 'struct A{template<class U>A(U){typename U::bad n;}};', 'new T{1}', ('A',))
detect('new_allocation_access', 'class A{static void*operator new(unsigned long);};struct B{static void*operator new(unsigned long);};', 'new T{}', ('B',), ('A',))
detect('new_global_allocation', 'class A{static void*operator new(unsigned long);};', '::new T{}', ('A',))
detect('new_inherited_list', 'struct B{template<class U>B(U,int=U::missing);};struct D:B{using B::B;};', 'new T{1}', ('D',))
GOOD['new_noexcept_destructor'] = 'struct A{static void*operator new(unsigned long)noexcept;A(int)noexcept;~A()noexcept(false);};template<class T>constexpr bool f(){return noexcept(new T{1});}static_assert(f<A>(), "allocated destructor is dormant");int main(){}'
GOOD['new_noexcept_field'] = 'struct M{M(int)noexcept(false);};struct A{static void*operator new(unsigned long)noexcept;M m;};template<class T>constexpr bool f(){return noexcept(new T{1});}static_assert(!f<A>(), "field construction throws");int main(){}'
GOOD['unused_parameter'] = 'template<class T>struct A{typename T::missing x;};void f(A<int>);int main(){}'
GOOD['unused_nested_parameter'] = 'template<class T>struct A{struct B{typename T::missing x;};};void f(A<int>::B);int main(){}'
GOOD['incomplete_parameter'] = 'struct A;void f(A);int main(){}'
GOOD['parameter_completed_later'] = 'template<class T>struct A{T x;};int f(A<int>);int f(A<int> a){return a.x;}int main(){A<int>a={7};return f(a)!=7;}'
GOOD['nested_parameter_completed_later'] = 'template<class T>struct A{struct B{T x;};};int f(A<int>::B);int f(A<int>::B b){return b.x;}int main(){A<int>::B b={9};return f(b)!=9;}'
GOOD['dormant_then_specialized'] = 'template<class T>struct A{typename T::missing x;};void f(A<int>);template<>struct A<int>{int n;};int main(){A<int>a={3};return a.n!=3;}'
GOOD['nested_inherited_constants'] = 'template<class T>struct O{struct B{int n;template<class U>constexpr B(U n,int k=2):n(n+k){}};struct D:B{using B::B;};};template<class T>struct V{static const int n=typename T::D{3}.n;};static_assert(V<O<int>>::n==5, "list and forwarding");int main(){O<int>::D d{4};return d.n!=6;}'
BAD['demanded_poison_parameter'] = 'template<class T>struct A{typename T::missing x;};void f(A<int> a){}int main(){}'
BAD['ordinary_new_narrow'] = 'int main(){int*p=new int{0.5};}'
BAD['ordinary_new_poison'] = 'struct A{template<class U>A(U){typename U::bad n;}};int main(){A*p=new A{1};}'
detect('new_ambiguous_allocation', 'struct L{static void*operator new(unsigned long);};struct R{static void*operator new(unsigned long);};struct A:L,R{};', 'new T{}', (), ('A',))
detect('new_placement_private_conversion', 'struct X{private:operator int();};struct A{static void*operator new(unsigned long,int);};template<class T>T dv();', 'new (dv<X>()) T{}', (), ('A',))
GOOD['constexpr_then_runtime'] = 'struct B{int n;template<class T>constexpr B(T n,int k=2):n(n+k){}};struct D:B{using B::B;};template<class T>struct V{static const int n=T{3}.n;};static_assert(V<D>::n==5, "");int main(){D d{4};return d.n!=6;}'
GOOD['constexpr_then_runtime_chain'] = 'struct B{int n;template<class T>constexpr B(T n,int k=2):n(n+k){}};struct D:B{using B::B;};struct E:D{using D::D;};template<class T>struct V{static const int n=T{3}.n;};static_assert(V<E>::n==5, "");int main(){E d{4};return d.n!=6;}'
GOOD['constexpr_only_inherited'] = 'struct B{int n;template<class T>constexpr B(T n,int k=2):n(n+k){}};struct D:B{using B::B;};template<class T>struct V{static const int n=T{3}.n;};static_assert(V<D>::n==5, "");int main(){}'
GOOD['constexpr_then_runtime_value'] = 'struct A{int n;constexpr A(int n):n(n){}constexpr A(const A&a):n(a.n+1){}constexpr A(A&&a):n(a.n+2){}};struct B{int n;template<class T>constexpr B(T n):n(n.n){}};struct D:B{using B::B;};template<class T>struct V{static const int n=T{A{3}}.n;};static_assert(V<D>::n>=5, "");int main(){A a(3);D d{a};return d.n!=6;}'
if __name__ == '__main__':
    runner.GOOD, runner.BAD = GOOD, BAD
    sys.exit(0 if runner.run(Path(sys.argv[1]).resolve(), Path(sys.argv[2])) else 1)
