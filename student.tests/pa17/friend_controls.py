#!/usr/bin/env python3
"""Explicit native/rejection controls for friend identity, access and demand."""
from pathlib import Path
import sys
import entity_controls as harness
harness.GOOD = {
 'class_all_specializations': 'class A{int n;template<class>friend struct F;public:A():n(7){}};template<class T>struct F{static int read(A&a){return a.n;}};template<>struct F<char>{static int read(A&a){return a.n+1;}};int main(){A a;return F<int>::read(a)+F<char>::read(a)-15;}',
 'function_all_specializations': 'class A{int n;template<class T>friend int f(A&);public:A():n(7){}};template<class T>int f(A&a){return a.n;}template<>int f<char>(A&a){return a.n+1;}int main(){A a;return f<int>(a)+f<char>(a)-15;}',
 'friend_class_nested': 'class A{typedef int I;template<class>friend struct F;};template<class T>struct F{struct Inner{static int f(){A::I x=7;return x;}};};int main(){return F<void>::Inner::f()-7;}',
 'private_base': 'class A{struct B{static int f(){return 7;}};template<class>friend struct F;};template<class T>struct F:A::B{};int main(){return F<void>::f()-7;}',
 'qualified_renamed_head': 'struct A;namespace N{template<class T,int V>int f(A&);}struct A{private:int n;public:A():n(3){}template<class U,int W>friend int N::f(A&);};namespace N{template<class X,int Z>int f(A&a){return a.n+Z;}}int main(){A a;return N::f<void,4>(a)-7;}',
 'qualified_member': 'struct A;struct F{template<class T>int f(A&)const;};struct A{private:int n;public:A():n(7){}template<class X>friend int F::f(A&)const;};template<class U>int F::f(A&a)const{return a.n;}int main(){A a;F f;return f.f<void>(a)-7;}',
 'class_template_friend_function': 'template<class>class A;template<class U>int f(A<U>&);template<class T>class A{int n;template<class U>friend int f(A<U>&);public:A():n(sizeof(T)) {}};template<class U>int f(A<U>&a){return a.n;}int main(){A<char>a;A<int>b;return f(a)+f(b)-5;}',
 'typename_friend': 'template<class T>class A{friend typename T::Self;A(){};};struct F{typedef F Self;static int f(){A<F>a;return 7;}};int main(){return F::f()-7;}',
 'nonclass_friend_ignored': 'template<class T>class A{friend T;public:int f(){return 7;}};int main(){A<int>a;return a.f()-7;}',
 'hidden_template_fixed_member': 'struct A{int n;template<class T>friend int f(T x,A&a){a.n+=x;return a.n;}};int main(){A a={3};return f(4,a)-7;}',
 'hidden_template_later_member': 'struct A{template<class T>friend int f(T x,A&a){a.n+=x;return a.n;}int n;};int main(){A a={3};return f(4,a)-7;}',
 'hidden_template_operator': 'struct A{int n;template<class T>friend T&operator<<(T&x,A&a){x+=a.n;return x;}};int main(){A a={3};int n=4;return (n<<a)-7;}',
 'hidden_outer_argument': 'template<class T>struct A{template<class U>friend int f(A const&,U u){return sizeof(T)+u;}};int main(){A<char>a;A<int>b;return f(a,2)+f(b,3)-10;}',
 'ordinary_friend_two_specializations': 'template<class T>struct A{friend int f(A const&){return sizeof(T);}};int main(){A<char>a;A<int>b;return f(a)+f(b)-5;}',
 'ordinary_dormant_friend': 'template<class T>struct A{friend int f(A){return T::missing;}};int main(){A<int>a;return sizeof(a)-1;}',
 'ordinary_constexpr_friend': 'template<class T>struct A{friend constexpr int f(A){return sizeof(T);}};static_assert(f(A<int>())==4,"");int main(){return f(A<char>())-1;}',
 'ordinary_friend_chain': 'template<class T>struct A{friend bool operator==(A const&,A const&){return true;}friend bool operator!=(A const&a,A const&b){return !(a==b);}};int main(){A<int>a,b;return a!=b;}',
 'hidden_function_redeclaration': 'struct A{friend int f(int){return 7;}};int f(int);int main(){return f(0)-7;}',
 'hidden_template_redeclaration': 'struct A{template<class T>friend int f(T){return 7;}};template<class U>int f(U);int main(){return f(0)-7;}',
 'namespace_identity': 'namespace X{template<class>struct V{};}namespace Y{template<class>struct V{};struct A{template<class T>friend int f(A&,X::V<T>){return 1;}template<class T>friend int f(A&,V<T>){return 2;}};}int main(){Y::A a;X::V<int>x;Y::V<int>y;return f(a,x)+f(a,y)-3;}',
}
harness.BAD = {
 'unrelated_function': 'class A{int n;template<class T>friend int f(A&);};template<class T>int g(A&a){return a.n;}int main(){A a;return g<int>(a);}',
 'friend_not_transitive': 'class A{int n;template<class>friend struct F;};struct G;template<class T>struct F{friend struct G;};struct G{int f(A&a){return a.n;}};',
 'friend_not_inherited': 'class A{int n;template<class>friend struct F;};template<class>struct F{};struct G:F<int>{int f(A&a){return a.n;}};',
 'single_specialization': 'template<class>struct F;class A{int n;friend struct F<int>;};template<class T>struct F{int f(A&a){return a.n;}};int main(){A a;F<char>f;return f.f(a);}',
 'hidden_template_unqualified': 'struct A{template<class T>friend int f(T){return 7;}};int main(){return f(0);}',
 'hidden_template_qualified': 'namespace N{struct A{template<class T>friend int f(T,A&){return 7;}};}int main(){N::A a;return N::f(0,a);}',
 'friend_is_not_member': 'struct A{template<class T>friend int f(T,A&){return 7;}};int main(){A a;return a.f(0,a);}',
 'qualified_absent': 'namespace N{}struct A{template<class T>friend int N::f(T);};',
 'qualified_wrong_signature': 'namespace N{template<class T>int f(T*);}struct A{template<class T>friend int N::f(T);};',
 'qualified_wrong_head': 'namespace N{template<class T>int f();}struct A{template<int N>friend int N::f();};',
 'ordinary_demanded_invalid': 'template<class T>struct A{friend int f(A){return T::missing;}};int main(){return f(A<int>());}',
 'dependent_friend_isolation': 'template<class T>class A{friend T;int n;};struct F{int f(A<F>&a){return a.n;}};struct G{int f(A<F>&a){return a.n;}};',
 'duplicate_friend_body': 'struct A{template<class T>friend int f(T,A&){return 1;}template<class U>friend int f(U,A&){return 2;}};',
}
harness.GOOD.update({
 'class_template_access_stable': 'struct A{template<class>struct B{};private:int n;};int main(){return sizeof(A::B<int>)-1;}',
 'class_specialization_access_stable': 'struct A{template<class>struct B;private:struct Base{};};template<class T>struct A::B{Base*x;};template<>struct A::B<void>{Base*x;};int main(){return sizeof(A::B<void>)-sizeof(void*);}',
 'variable_template_access_stable': 'struct A{template<class T>static constexpr int n=sizeof(T);private:int x;};int main(){return A::n<int>-4;}',
 'operator_specialization_identity': 'template<class T>struct A;template<class T>int operator+(int,A<T>const&);template<class T>struct A{private:int n;friend int operator+<>(int,A const&);public:A():n(sizeof(T)) {}};template<class T>int operator+(int x,A<T>const&a){return a.n+x;}int main(){A<char>a;A<int>b;return (2+a)+(3+b)-10;}',
 'operator_query_identity': 'template<class T>struct A;template<class T>int operator+(int,A<T>const&);template<class T>struct A{friend int operator+<>(int,A const&);};template<class T>int operator+(int x,A<T>const&a){return sizeof(T)+x;}template<class T>auto f(T const&t)->decltype(1+t){return 1+t;}int main(){A<int>a;return f(a)-5;}',
 'friend_template_enclosing_body': 'template<class T>struct A{template<class U>friend int f(U){return sizeof(T);}};A<int>a;template<class U>int f(U);int main(){return f(0)-4;}',
 'friend_existing_specialization': 'template<class T>struct A;template<class T>int f(A<T>&);template<class T>struct A{private:int n;friend int f<>(A&);public:A():n(sizeof(T)) {}};template<class T>int f(A<T>&a){return a.n;}int main(){A<char>a;A<int>b;return f(a)+f(b)-5;}',
 'qualified_class_template': 'namespace N{template<class>struct F;}class A{int n;template<class T>friend struct N::F;public:A():n(7){}};namespace N{template<class>struct F{static int f(A&a){return a.n;}};}int main(){A a;return N::F<int>::f(a)-7;}',
})
harness.BAD.update({
 'private_class_template_stable': 'class A{template<class>struct B{};public:int n;};A::B<int>b;',
 'private_variable_template_stable': 'class A{template<class T>static constexpr int n=sizeof(T);public:int x;};int main(){return A::n<int>;}',
 'friend_class_hidden': 'class A{template<class>friend struct F;};F<int>*f;',
 'qualified_class_absent': 'namespace N{}class A{template<class>friend struct N::F;};',
 'friend_partial_declaration': 'template<class>struct F;class A{template<class T>friend struct F<T*>;};',
 'friend_template_body_redefinition': 'template<class T>struct A{template<class U>friend int f(U){return sizeof(T);}};A<int>a;A<char>b;',
})
if __name__ == '__main__':
 cc=Path(sys.argv[1]).resolve() if len(sys.argv)>1 else harness.ROOT/'dev/cppgm++'
 work=Path(sys.argv[2]) if len(sys.argv)>2 else Path('/tmp/pa17-friend-controls')
 sys.exit(0 if harness.run(cc,work) else 1)
