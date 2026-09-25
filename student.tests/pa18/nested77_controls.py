#!/usr/bin/env python3
"""Nested class declaration/completion ownership: CC WORK."""
from pathlib import Path
import sys
import ordering_controls as runner

runner.GOOD = {
 'unused_assert': 'template<class T>struct O{struct N{static_assert(sizeof(T)==0,"");};};O<int> o;int main(){}',
 'unused_alias': 'template<class T>struct O{struct N{using X=typename T::missing;};};O<int> o;int main(){}',
 'pointer_alias': 'template<class T>struct O{struct N{using X=typename T::missing;};};using N=O<int>::N;N*p;int main(){return p!=0;}',
 'member_template': 'template<class T>struct O{template<class U>struct M{struct N{using X=typename T::missing;};};};O<int>::M<long> o;int main(){}',
 'unused_sibling': 'template<class T>struct O{struct A{using X=typename T::missing;};struct B{T n;};};int main(){O<int>::B b={7};return b.n!=7;}',
 'deep': 'template<class T>struct O{struct A{struct B{struct C{using X=typename T::missing;};T n;};};};int main(){O<int>::A::B b={7};return b.n!=7;}',
 'same_identity': 'template<class T>struct O{struct N{T n;};using A=N;};void f(O<int>::N*);void f(O<int>::A*p){p->n=9;}int main(){O<int>::N n={7};f(&n);return n.n!=9;}',
 'forward_definition': 'template<class T>struct O{struct N;using A=N;struct N{T n;};};int main(){O<int>::A n={7};return n.n!=7;}',
 'self_pointer': 'template<class T>struct O{struct N{N*next;T n;};};int main(){O<int>::N n={0,7};return n.next!=0||n.n!=7;}',
 'mutual_pointers': 'template<class T>struct O{struct B;struct A{B*p;};struct B{A*p;T n;};};int main(){O<int>::A a={0};O<int>::B b={&a,7};a.p=&b;return a.p->n!=7;}',
 'nested_constructor': 'template<class T>struct O{struct N{T n;N(T n):n(n){}N(T*):n(T::missing){}};};int main(){O<int>::N n(7);return n.n!=7;}',
 'nested_destructor': 'int count;template<class T>struct O{struct N{T n;N(T n):n(n){}~N(){count+=n;}};};int main(){{O<int>::N n(7);}return count!=7;}',
 'nested_method': 'template<class T>struct O{struct N{T n;T get(){return n;}void unused(){typename T::missing x;}};};int main(){O<int>::N n={7};return n.get()!=7;}',
 'nested_method_template': 'template<class T>struct O{struct N{template<class U>T get(U n){return n;}};};int main(){O<int>::N n;return n.get(7L)!=7;}',
 'member_object': 'template<class T>struct O{struct N{T n;};N n;};int main(){O<int> o={{7}};return o.n.n!=7;}',
 'member_array': 'template<class T>struct O{struct N{T n;};N n[2];};int main(){O<int> o={{{3},{7}}};return o.n[1].n!=7;}',
 'base': 'template<class T>struct O{struct N:T{int n;};};struct B{int b;};int main(){O<B>::N n;n.b=7;return n.b!=7;}',
 'derived': 'template<class T>struct O{struct N{T n;};};struct D:O<int>::N{};int main(){D d;d.n=7;return d.n!=7;}',
 'anonymous_union': 'template<class T>struct O{union{T n;long m;};};int main(){O<int> o;o.n=7;return o.n!=7;}',
 'named_union': 'template<class T>struct O{union N{T n;long m;};};int main(){O<int>::N o;o.n=7;return o.n!=7;}',
 'unused_named_union': 'template<class T>struct O{union N{typename T::missing n;};};O<int> o;int main(){}',
 'outer_later_type': 'template<class T>struct O{struct N{int f(){return sizeof(A);}};using A=T;};int main(){O<long>::N n;return n.f()!=8;}',
 'outer_later_default': 'template<class T>struct O{struct N{int f(int n=sizeof(A)){return n;}};using A=T;};int main(){O<long>::N n;return n.f()!=8;}',
 'nested_later_default': 'template<class T>struct O{struct N{int f(int n=sizeof(A)){return n;}using A=T;};};int main(){O<long>::N n;return n.f()!=8;}',
 'out_of_class_definition': 'template<class T>struct O{struct N;};template<class T>struct O<T>::N{T n;};int main(){O<int>::N n={7};return n.n!=7;}',
 'out_of_class_method': 'template<class T>struct O{struct N{T f();};};template<class T>T O<T>::N::f(){return 7;}int main(){O<int>::N n;return n.f()!=7;}',
 'static_member': 'template<class T>struct O{struct N{static T n;};};template<class T>T O<T>::N::n=7;int main(){return O<int>::N::n!=7;}',
 'constexpr': 'template<class T>struct O{struct N{T n;constexpr N(T n):n(n){}};};constexpr O<int>::N n(7);static_assert(n.n==7,"");int main(){}',
 'alignas_dormant': 'template<class T>struct O{struct alignas(sizeof(typename T::missing)) N{int n;};};O<int>o;int main(){}',
 'alignment': 'template<class T>struct O{struct alignas(16) N{T n;};};static_assert(alignof(O<int>::N)==16,"");int main(){}',
 'explicit_outer': 'template<class T>struct O{struct N{T f(){return 7;}};};template struct O<int>;int main(){O<int>::N n;return n.f()!=7;}',
 'extern_outer': 'template<class T>struct O{struct N{using X=typename T::missing;};};extern template struct O<int>;int main(){}',
 'function_local': 'template<class T>int f(){struct N{T n;};N n={7};return n.n;}int main(){return f<int>()!=7;}',
 'local_nested': 'template<class T>int f(){struct L{struct N{T n;};N n;};L l={{7}};return l.n.n;}int main(){return f<int>()!=7;}',
}
runner.BAD = {
 'local_nested_required': 'template<class T>void f(){struct L{struct N{using X=typename T::missing;};};}int main(){f<int>();}',
 'demand_assert': 'template<class T>struct O{struct N{static_assert(sizeof(T)==0,"");};};O<int>::N n;',
 'demand_alias': 'template<class T>struct O{struct N{using X=typename T::missing;};};O<int>::N n;',
 'demand_size': 'template<class T>struct O{struct N{using X=typename T::missing;};};int n=sizeof(O<int>::N);',
 'demand_member': 'template<class T>struct O{struct N{using X=typename T::missing;};N n;};O<int> o;',
 'demand_base': 'template<class T>struct O{struct N:T{};};O<int>::N n;',
 'definition_side_effect': 'template<class T>struct O{struct N{using X=typename T::missing;using type=int;};};template<class T>typename O<T>::N::type f(int);template<class>long f(...);int n=sizeof(f<int>(0));',
 'demand_alignment': 'template<class T>struct O{struct alignas(sizeof(typename T::missing)) N{int n;};};O<int>::N n;',
 'explicit_outer_invalid': 'template<class T>struct O{struct N{using X=typename T::missing;};};template struct O<int>;',
 'ordinary_nested': 'struct O{struct N{static_assert(false,"");};};',
 'fixed_nested': 'template<class T>struct O{struct N{static_assert(false,"");};};',
 'duplicate_definition': 'template<class T>struct O{struct N{};struct N{};};',
 'recursive_value': 'template<class T>struct O{struct N{N n;};};O<int>::N n;',
 'private_name': 'template<class T>class O{struct N{T n;};};O<int>::N*p;',
 'private_member': 'template<class T>struct O{class N{T n;};};int main(){O<int>::N n;n.n=7;}',
 'final_base': 'template<class T>struct O{struct N final{T n;};};struct D:O<int>::N{};',
 'anonymous_union_eager': 'template<class T>struct O{union{typename T::missing n;};};O<int>o;',
 'selected_method': 'template<class T>struct O{struct N{void f(){typename T::missing n;}};};int main(){O<int>::N n;n.f();}',
}
if __name__ == '__main__':
 sys.exit(0 if runner.run(Path(sys.argv[1]).resolve(),Path(sys.argv[2])) else 1)
