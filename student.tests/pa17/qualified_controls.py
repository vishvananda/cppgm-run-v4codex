#!/usr/bin/env python3
"""Current-instantiation paths and immediate inline namespace visibility."""
from pathlib import Path
import sys
import entity_controls as runner

runner.GOOD = {
'inherited_current': '''template<class T>struct B{using type=T;};template<class T>struct D:B<T>{using type=typename D::type;type x;};int main(){D<int>d;d.x=7;return d.x-7;}''',
'inherited_current_explicit': '''template<class T>struct B{using type=T;};template<class T>struct D:B<T>{using value=typename D<T>::type;value x;};int main(){D<long>d;d.x=9;return d.x-9;}''',
'inherited_reference_alias': '''template<class T>struct B{using type=T;};template<class T>struct D:B<T>{using type=typename D::type;};int main(){int x=6;D<int&>::type y=x;y=8;return x-8;}''',
'inherited_specialization': '''template<class T>struct B{using type=int;};template<class T>struct B<T*>{using type=long;};template<class T>struct D:B<T>{using value=typename D::type;};static_assert(sizeof(D<int>::value)==4 && sizeof(D<char*>::value)==8,"selected base");int main(){return 0;}''',
'inherited_nested_template': '''template<class T>struct B{template<class U>struct apply{using type=U;};};template<class T>struct D:B<T>{using value=typename D::template apply<T>::type;};int main(){D<int>::value x=3;return x-3;}''',
'inherited_alias_template': '''template<class T>struct B{template<class U>using apply=U;};template<class T>struct D:B<T>{using value=typename D::template apply<T>;};int main(){D<int>::value x=3;return x-3;}''',
'inherited_second_qualifier': '''template<class T>struct B{struct nested{using type=T;};};template<class T>struct D:B<T>{using value=typename D::nested::type;};int main(){D<int>::value x=3;return x-3;}''',
'inherited_outer_inner': '''template<class T>struct B{using type=T;};struct O{template<class T>struct D:B<T>{using value=typename D::type;};};int main(){O::D<int>::value x=3;return x-3;}''',
'inherited_partial_current': '''template<class T>struct B{using type=T;};template<class T>struct D;template<class T>struct D<T*>:B<T>{using value=typename D::type;};int main(){D<int*>::value x=3;return x-3;}''',
'inherited_out_of_class': '''template<class T>struct B{using type=T;};template<class T>struct D:B<T>{int f(typename D::type);};template<class U>int D<U>::f(typename D::type x){return x;}int main(){D<int>d;return d.f(5)-5;}''',
'known_current_no_typename': '''template<class T>struct B{using type=long;};template<class T>struct D:B<T>{using type=int;D::type x;};static_assert(sizeof(D<char>::type)==4,"local hides");int main(){D<char>d;d.x=4;return d.x-4;}''',
'known_fixed_base_no_typename': '''struct B{using type=int;};template<class T>struct D:B{D::type x;};int main(){D<char>d;d.x=4;return d.x-4;}''',
'unknown_unused_deferred': '''template<class T>struct D:T{using value=typename D::missing;};int main(){return 0;}''',
'private_inherited_inside': '''template<class T>class B{protected:using type=T;};template<class T>struct D:B<T>{using value=typename D::type;};int main(){D<int>::value x=3;return x-3;}''',
'inline_qualified_function': '''namespace N{inline namespace V{template<int I>int f(){return I;}int g(){return N::f<7>();}}}int main(){return N::g()-7;}''',
'inline_alias_qualified_function': '''namespace N{inline namespace V{template<int I>int f(){return I;}namespace A=N;int g(){return A::f<7>();}}}int main(){return N::g()-7;}''',
'inline_nested_qualified_function': '''namespace N{inline namespace V{inline namespace W{template<int I>int f(){return I;}int g(){return N::f<7>();}}}}int main(){return N::g()-7;}''',
'inline_qualified_type': '''namespace N{inline namespace V{template<int I>struct C{static const int n=I;};int g(){return N::C<7>::n;}}}int main(){return N::g()-7;}''',
'inline_reopened': '''namespace N{inline namespace V{template<int I>int f(){return I;}}namespace V{int g(){return N::f<7>();}}}int main(){return N::g()-7;}''',
'inline_qualified_pack': '''namespace N{inline namespace V{template<int I>int f(){return I;}int sum(int a,int b){return a+b;}template<int...I>int g(){return sum(N::f<I>()...);}}}int main(){return N::g<2,5>()-7;}''',
'inline_qualified_type_pack': '''namespace N{inline namespace V{template<class T>int f(T){return sizeof(T);}int sum(int a,int b){return a+b;}template<class...T>int g(T...x){return sum(N::f<T>(x)...);}}}int main(){return N::g(1,2L)-12;}''',
}
runner.BAD = {
'inherited_requires_typename': 'template<class T>struct B{using type=T;};template<class T>struct D:B<T>{D::type x;};',
'inherited_requires_template': 'template<class T>struct B{template<class U>struct apply{using type=U;};};template<class T>struct D:B<T>{using value=typename D::apply<T>::type;};',
'inherited_template_requires_typename': 'template<class T>struct B{template<class U>struct apply{};};template<class T>struct D:B<T>{D::template apply<T> x;};',
'inherited_missing_after_substitution': 'struct B{};template<class T>struct D:T{using value=typename D::missing;};D<B>d;',
'closed_current_missing': 'template<class T>struct D{using value=typename D::missing;};D<int>d;',
'closed_fixed_base_missing': 'struct B{};template<class T>struct D:B{using value=typename D::missing;};D<int>d;',
'inherited_private': 'template<class T>class B{using type=T;};template<class T>struct D:B<T>{using value=typename D::type;};D<int>d;',
'inherited_ambiguous': 'template<class T>struct A{using type=T;};template<class T>struct B{using type=T;};template<class T>struct D:A<T>,B<T>{using value=typename D::type;};D<int>d;',
'different_specialization_requires_typename': 'template<class T>struct D{using type=int;D<T*>::type x;};',
'inline_local_value_hides': 'namespace N{inline namespace V{template<int>int f(){return 1;}int g(){int f=2;return f<1>();}}}',
'inline_qualified_no_parent': 'namespace N{template<int>int f(){return 1;}inline namespace V{int g(){return V::f<1>();}}}',
}
runner.GOOD.update({
'fixed_and_dependent_own_hides': 'struct A{using type=int;};struct B{using type=long;};template<class T>struct D:A,T{using type=char;D::type x;};int main(){D<B>d;d.x=4;return d.x-4;}',
'fixed_and_dependent_unambiguous': 'struct A{using type=int;};struct B{};template<class T>struct D:A,T{D::type x;};int main(){D<B>d;d.x=4;return d.x-4;}',
'fixed_pack_head_return': 'template<class T,class...U>struct First{using type=T;};template<class...T>typename First<T...>::type f(){return 7;}int main(){return f<int,long>()-7;}',
'fixed_pack_two_heads': 'template<class T,class U,class...V>struct Second{using type=U;};template<class...T>typename Second<T...>::type f(){return 7;}int main(){return f<char,int,long>()-7;}',
'fixed_pack_empty_tail': 'template<class T,class...U>struct First{using type=T;};template<class...T>typename First<T...>::type f(){return 7;}int main(){return f<int>()-7;}',
'fixed_pack_reference': 'template<class T,class...U>struct First{using type=T;};template<class...T>typename First<T...>::type f(int&x){return x;}int main(){int n=7;f<int&,long>(n)=9;return n-9;}',
'fixed_pack_alias': 'template<class T,class...U>struct First{using type=T;};template<class...T>using Result=typename First<T...>::type;int main(){Result<int,long>n=7;return n-7;}',
'fixed_pack_recursive': 'template<int I,class T,class...U>struct E:E<I-1,U...>{};template<class T,class...U>struct E<0,T,U...>{using type=T;};template<int I,class...T>typename E<I,T...>::type f(){return 7;}int main(){return f<2,char,long,int>()-7;}',
'fixed_pack_value': 'template<int I,int...V>struct First{static const int value=I;};template<int...I>int f(){return First<I...>::value;}int main(){return f<7,9>()-7;}',
'fixed_pack_enclosing_member': 'template<class T,class...U>struct First{using type=T;};template<class...T>struct Outer{template<int N>typename First<T...>::type f(){return N;}};int main(){Outer<int,long>o;return o.f<7>()-7;}',
})
runner.BAD.update({
'fixed_pack_wrong_type_kind': 'template<int I,class...T>struct S;template<class...T>S<T...>* f();',
'fixed_pack_wrong_value_kind': 'template<class T,int...I>struct S;template<int...I>S<I...>* f();',
'fixed_and_dependent_ambiguity': 'struct A{using type=int;};struct B{using type=long;};template<class T>struct D:A,T{D::type x;};D<B>d;',
'fixed_pack_missing_head': 'template<class T,class...U>struct First{using type=T;};template<class...T>typename First<T...>::type f(){return 7;}int main(){return f<>();}',
'fixed_pack_missing_second': 'template<class T,class U,class...V>struct Second{using type=U;};template<class...T>typename Second<T...>::type f(){return 7;}int main(){return f<int>();}',
'closed_current_missing_unused': 'template<class T>struct D{using value=typename D::missing;};',
})
if __name__=='__main__':
 sys.exit(0 if runner.run(Path(sys.argv[1]).resolve(),Path(sys.argv[2])) else 1)
