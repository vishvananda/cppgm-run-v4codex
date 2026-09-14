#!/usr/bin/env python3
"""Accumulated PA17 ownership audit controls; run explicitly."""
from pathlib import Path
import sys
import entity_controls as harness

harness.GOOD = {
 'nested_renamed_default_alias': 'template<class T>struct A{template<class U=T>struct B{template<class V=U>using C=V;};};int main(){A<char>::B<>::C<> c=0;static_assert(sizeof(c)==1, "");return c;}',
 'nested_friend_access': 'template<class T>class A{int n;template<class U>friend struct F;public:A():n(sizeof(T)){}};template<class U>struct F{template<class T>static int get(A<T>&a){return a.n;}};int main(){A<int>a;A<char>b;return F<void>::get(a)+F<void>::get(b)-5;}',
 'friend_then_namespace_definition': 'template<class T>struct A{template<class U>friend int f(A<T> const&,U);};template<class T,class U>int f(A<T>const&,U){return 2;}template<class U>int f(A<int>const&,U){return 7;}int main(){A<int>a;return f(a,0)-7;}',
 'ordinary_friend_recursive_demand': 'template<class T>struct A{friend int f(A a,int n){return n ? f(a,n-1)+1 : sizeof(T);}};int main(){return f(A<char>(),6)-7;}',
 'ordinary_friend_address_demand': 'template<class T>struct A{friend int f(A){return sizeof(T);}};A<int>a;int f(A<int>);int(*p)(A<int>)=&f;int main(){return p(a)-4;}',
 'current_volatile_value_alias': 'template<int N>struct A{static const int I=N;typedef int type;A<I>::type n;};int main(){A<1>a={7};return a.n-7;}',
 'paren_multiple_declarators': 'struct P{static const int n=3;};template<class T>int f(){int a(T::n),b(T::n);return a+b;}int main(){return f<P>()-6;}',
 'paren_nested_bodies': 'struct P{static const int n=3;};template<class T>int f(){int a(T::n);{int b(T::n);a+=b;}return a;}int main(){return f<P>()-6;}',
 'fixed_receiver_chain': 'struct P{template<class T>int f(T n){return n;}P&self(){return *this;}};int main(){P p;return p.self().self().f<int>(7)-7;}',
 'member_template_ordinary_conversion_preference': 'template<class T>struct A{int n;A(int):n(1){}template<class U>A(U):n(2){}};A<char> f(){return 0;}int main(){return f().n-1;}',
 'member_template_conversion_better': 'template<class T>struct A{int n;A(long):n(1){}template<class U>A(U):n(2){}};A<char> f(){return 0;}int main(){return f().n-2;}',
 'renamed_inner_return_access': 'template<class T>struct A{template<class U>class B{typedef U R;public:template<class V>R f(V);};};template<class X>template<class Y>template<class Z>typename A<X>::template B<Y>::R A<X>::B<Y>::f(Z z){return z;}int main(){A<void>::B<int>b;return b.f(7)-7;}',
 'late_member_after_two_demands': 'template<class T>struct A{template<class U>static int f(U);};int a(){return A<char>::f(1);}int b(){return A<int>::f(2);}template<class X>template<class Y>int A<X>::f(Y n){return n+sizeof(X);}int main(){return a()+b()-8;}',
 'explicit_friend_function_specialization': 'class A{int n;template<class>friend int f(A&);public:A():n(3){}};template<class T>int f(A&a){return a.n;}template<>int f<void>(A&a){return 2*a.n;}int main(){A a;return f<int>(a)+f<void>(a)-9;}',
}
harness.BAD = {
 'qualified_dormant_friend_absent': 'namespace N{}template<class T>struct A{friend int N::f(int);};',
 'qualified_dormant_friend_wrong_signature': 'namespace N{int f(char*);}template<class T>struct A{friend int N::f(int);};',
 'different_value_alias_type': 'template<int N>struct A{static const long I=N;typedef int type;A<I>::type n;};',
 'parenthesized_value_alias': 'template<int N>struct A{static const int I=(N);typedef int type;A<I>::type n;};',
 'friend_body_access_not_transitive': 'struct A{private:int n;template<class>friend struct F;};template<class T>struct F{static int f(A&a){return a.n;}};template<class T>int g(A&a){return a.n;}int main(){A a;return F<int>::f(a)+g<int>(a);}',
 'ordinary_friend_redefinition_two_owners': 'template<class T>struct A{friend int f(int){return sizeof(T);}};A<int>a;A<char>b;',
 'friend_class_wrong_head': 'template<class>struct F;struct A{template<int>friend struct F;};',
 'dormant_current_non_type': 'template<class T>struct A{static const int n=3;A<T>::n x;};',
 'member_template_private_access': 'template<class T>class A{template<class U>static int f(U){return 7;}};int main(){return A<int>::f(0);}',
 'member_template_duplicate_default': 'template<class T>struct A{template<class U=int>int f(U);};template<class X>template<class Y=int>int A<X>::f(Y){return 0;}',
}
harness.GOOD.update({
 'ordinary_friend_decay_demand': 'template<class T>struct A{friend int f(A){return sizeof(T);}};A<int>a;int f(A<int>);int(*p)(A<int>)=f;int main(){return p(a)-4;}',
 'ordinary_friend_reference_demand': 'template<class T>struct A{friend int f(A){return sizeof(T);}};A<int>a;int f(A<int>);int(&p)(A<int>)=f;int main(){return p(a)-4;}',
 'ordinary_friend_operand_decay': 'template<class T>struct A{friend int f(A){return sizeof(T);}};A<int>a;int f(A<int>);int g(int(*p)(A<int>)){return p(a);}int main(){return g(f)-4;}',
 'ordinary_friend_unevaluated': 'template<class T>struct A{friend int f(A){return T::missing;}};A<int>a;int f(A<int>);int main(){return sizeof(&f)-sizeof(void*);}',
 'qualified_fixed_friend': 'namespace N{int f(int);}template<class T>class A{int n;friend int N::f(int);public:A():n(sizeof(T)){}};int N::f(int){A<char>a;return a.n;}int main(){return N::f(0)-1;}',
 'qualified_deduced_friend': 'namespace N{template<class U>int f(U);}template<class T>class A{int n;friend int N::f(int);public:A():n(sizeof(T)){}};template<class U>int N::f(U){A<U>a;return a.n;}int main(){return N::f(0)-4;}',
 'fixed_template_id_friend': 'template<class U>int f(U);template<class T>class A{int n;friend int f<int>(int);public:A():n(sizeof(T)){}};template<class U>int f(U){A<U>a;return a.n;}int main(){return f(0)-4;}',
 'in_class_default_preserved': 'template<class T>struct A{template<class U=int>static int f(){return sizeof(U);}};int main(){return A<char>::f()-4;}',
 'namespace_function_default': 'template<class T=int>int f(){return sizeof(T);}int main(){return f()-4;}',
 'nested_source_parameter_names': 'template<class T>struct A{template<class U>struct B{template<class V>int f(){int n=sizeof(T)+sizeof(U)+sizeof(V);return n;}};};int main(){A<char>::B<int>b;return b.f<char>()-6;}',
})
harness.BAD.update({
 'fixed_template_id_friend_absent': 'template<class T>struct A{friend int f<int>(int);};',
 'fixed_template_id_friend_mismatch': 'template<class U>int f(U*);template<class T>struct A{friend int f<int>(int);};',
 'qualified_friend_undeducible_head': 'namespace N{template<class U>int f(int);}template<class T>struct A{friend int N::f(int);};',
 'member_template_new_default': 'template<class T>struct A{template<class U>int f(U);};template<class X>template<class Y=int>int A<X>::f(Y){return 0;}',
 'outer_head_default': 'template<class T>struct A{int f();};template<class T=int>int A<T>::f(){return 0;}',
 'nested_class_definition_default': 'template<class T>struct A{template<class U>struct B;};template<class T>template<class U=int>struct A<T>::B{};',
 'nested_body_shadows_outer': 'template<class T>struct A{template<class U>int f(){int T=0;return T;}};',
 'nested_body_shadows_inner': 'template<class T>struct A{template<class U>int f(){int U=0;return U;}};',
 'retained_body_shadows_outer': 'template<class T>struct A{template<class U>int f();};template<class X>template<class Y>int A<X>::f(){int X=0;return X;}',
 'retained_body_shadows_inner': 'template<class T>struct A{template<class U>int f();};template<class X>template<class Y>int A<X>::f(){int Y=0;return Y;}',
})
if __name__ == '__main__':
 cc=Path(sys.argv[1]).resolve() if len(sys.argv)>1 else harness.ROOT/'dev/cppgm++'
 work=Path(sys.argv[2]) if len(sys.argv)>2 else Path('/tmp/pa17-checkpoint52-controls')
 sys.exit(0 if harness.run(cc,work) else 1)
