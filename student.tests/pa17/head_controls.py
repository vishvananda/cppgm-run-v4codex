#!/usr/bin/env python3
"""Explicit PA17 retained member-head identity and demand controls."""
from pathlib import Path
import sys
import entity_controls as harness
harness.GOOD = {
'renamed_two_heads': '''template<class T>struct A{template<class U>int f(U);};template<class X>template<class Y>int A<X>::f(Y x){return sizeof(X)+sizeof(Y)+x;}int main(){A<char>a;A<long>b;return a.f(2)+b.f(char(3))-19;}''',
'head_roles': '''template<class H>struct A{template<class T>int f();template<int N>int f();};template<class H>template<class T>int A<H>::f(){return sizeof(T);}template<class H>template<int N>int A<H>::f(){return N;}int main(){A<char>a;return a.f<int>()+a.f<7>()-11;}''',
'outer_inner_distinct': '''template<class T>struct A{template<class U>int f(T,U);template<class U>int f(U,T*);};template<class X>template<class Y>int A<X>::f(X,Y){return 1;}template<class X>template<class Y>int A<X>::f(Y,X*){return 2;}int main(){A<int>a;int*p=0;return a.f(3,'x')+a.f('x',p)-3;}''',
'late_selected_body': '''template<class T>struct A{template<class U>int f(U);};int g(){A<char>a;return a.f(3);}template<class X>template<class Y>int A<X>::f(Y x){return sizeof(X)+x;}int main(){return g()-4;}''',
'late_nested_body': '''template<class T>struct A{struct B{template<class U>int f(U);};};int g(){A<char>::B b;return b.f(3);}template<class X>template<class Y>int A<X>::B::f(Y x){return sizeof(X)+x;}int main(){return g()-4;}''',
'nested_constructor': '''template<class T>struct A{struct B{int n;template<class U>B(U);};};template<class X>template<class Y>A<X>::B::B(Y y):n(sizeof(X)+y){}int main(){A<char>::B b(3);return b.n-4;}''',
'partial_member_template': '''template<class T>struct A;template<class T>struct A<T*>{template<class U>int f(U);};template<class X>template<class Y>int A<X*>::f(Y y){return sizeof(X)+y;}int main(){A<int*>a;return a.f(3)-7;}''',
'head_default_scope': '''template<class T>struct A{template<class U=T>static int f(U x=U(3));};template<class X>template<class Y>int A<X>::f(Y x){return sizeof(X)+x;}int main(){return A<char>::f()-4;}''',
'inline_nested_template': '''template<class T>struct A{template<class U>struct B{static int f(){return sizeof(T)*10+sizeof(U);}};};int main(){return A<char>::B<int>::f()+A<int>::B<char>::f()-55;}''',
'inline_member_body': '''template<class T>struct A{T n;template<class U>int f(U x){return n+x;}};int main(){A<int>a={4};return a.f(3)-7;}''',
'forward_member_name': '''template<class T>struct A{template<class U>int f(U x){return g(x);}template<class U>int g(U x){return x+sizeof(T);}};int main(){A<char>a;return a.f(3)-4;}''',
'variable_dependent_head': '''template<class T>struct A{template<T N>int f();};template<class X>template<X M>int A<X>::f(){return M;}int main(){A<int>a;return a.f<7>()-7;}''',
'pack_member': '''template<class T>struct A{template<class...U>int f(U...);};template<class X>template<class...Y>int A<X>::f(Y...){return sizeof(X)+sizeof...(Y);}int main(){A<char>a;return a.f()+a.f(1,2,3)-5;}''',
'pack_outer': '''template<class...T>struct A{template<class U>int f(U);};template<class...X>template<class Y>int A<X...>::f(Y y){return sizeof...(X)+y;}int main(){A<>a;A<int,char>b;return a.f(1)+b.f(2)-5;}''',
'dormant_member_body': '''template<class T>struct A{template<class U>int good(U);template<class U>int bad(U);};template<class T>template<class U>int A<T>::good(U x){return x;}template<class T>template<class U>int A<T>::bad(U){return sizeof(typename U::missing);}int main(){A<int>a;return a.good(7)-7;}''',
'extern_member_template': '''template<class T>struct A{template<class U>int f(U);};template<class X>template<class Y>int A<X>::f(Y y){return sizeof(X)+y;}extern template struct A<int>;int main(){A<int>a;return a.f(3)-7;}''',
'private_definition': '''template<class T>class A{typedef T R;public:template<class U>R f(U);};template<class X>template<class Y>typename A<X>::R A<X>::f(Y y){return y;}int main(){A<int>a;return a.f(7)-7;}''',
'explicit_member_owner': '''template<class T>struct A{template<class U>int f(U){return 1;}};A<int>a;template<>template<class U>int A<int>::f(U){return 2;}int main(){A<char>b;return a.f(0)+b.f(0)-3;}''',
}
harness.BAD = {
'duplicate_renamed_definition': 'template<class T>struct A{template<class U>int f(U);};template<class T>template<class U>int A<T>::f(U){return 1;}template<class X>template<class Y>int A<X>::f(Y){return 2;}',
'wrong_head_kind': 'template<class T>struct A{template<class U>int f();};template<class T>template<int N>int A<T>::f(){return 1;}',
'wrong_head_count': 'template<class T>struct A{template<class U>int f();};template<class T>template<class X,class Y>int A<T>::f(){return 1;}',
'crossed_signature': 'template<class T>struct A{template<class U>int f(T,U*);};template<class T>template<class U>int A<T>::f(U,T*){return 1;}',
'already_inline': 'template<class T>struct A{template<class U>int f(U){return 1;}};template<class T>template<class U>int A<T>::f(U){return 2;}',
'wrong_namespace': 'namespace N{template<class T>struct A{template<class U>int f(U);};}namespace M{template<class T>template<class U>int N::A<T>::f(U){return 1;}',
'duplicate_explicit_owner': 'template<class T>struct A{template<class U>int f(U){return 1;}};template<>template<class U>int A<int>::f(U){return 2;}template<>template<class V>int A<int>::f(V){return 3;}',
}
if __name__=='__main__':
 cc=Path(sys.argv[1]).resolve() if len(sys.argv)>1 else harness.ROOT/'dev/cppgm++'
 work=Path(sys.argv[2]) if len(sys.argv)>2 else Path('/tmp/pa17-head-controls')
 sys.exit(0 if harness.run(cc,work) else 1)
