#!/usr/bin/env python3
"""Out-of-class template owner controls. Run explicitly with compiler/workdir."""
from pathlib import Path
import sys
import entity_controls as harness
harness.GOOD = {
'partial_deduced_tuple': """template<class X,class Y>struct P{};
template<class T>struct A{int f();};template<class X,class Y>struct A<P<X,Y>>{int f();};
template<class T>int A<T>::f(){return 1;}template<class K,class V>int A<P<K,V>>::f(){return sizeof(K)*10+sizeof(V);}
int main(){A<int>a;A<P<char,int>>b;return a.f()+b.f()-15;}""",
'partial_alias_signature': """template<class T>struct A;template<class T>struct A<T*>{typedef T R;static R f(R);};
template<class U>typename A<U*>::R A<U*>::f(R x){return x+1;}
int main(){return A<int*>::f(6)-7;}""",
'nested_owner_separation': """template<class T>struct A{struct B;};template<class T>struct A<T*>{struct B;};
template<class U>struct A<U>::B{int f();};template<class U>struct A<U*>::B{int f();};
template<class V>int A<V>::B::f(){return 1;}template<class V>int A<V*>::B::f(){return sizeof(V)+2;}
int main(){A<char>::B a;A<int*>::B b;return a.f()+b.f()-7;}""",
'partial_static_storage': """template<class T>struct A{static int n;};template<class T>struct A<T*>{static int n;};
template<class T>int A<T>::n=1;template<class U>int A<U*>::n=sizeof(U)+2;
int main(){return A<char>::n+A<int*>::n-7;}""",
'partial_late_body': """template<class T>struct A;template<class T>struct A<T*>{int f();};
int g(){A<int*>a;return a.f();}template<class U>int A<U*>::f(){return sizeof(U)+3;}
int main(){return g()-7;}""",
'partial_overloaded_members': """template<class T>struct A;template<class T>struct A<T*>{int f(int);int f(char);};
template<class U>int A<U*>::f(int){return 1;}template<class U>int A<U*>::f(char){return 2;}
int main(){A<int*>a;return a.f(0)+a.f('x')-3;}""",
'partial_value_owner': """template<class T,int N>struct A;template<class T,int N>struct A<T*,N>{int f();};
template<class U,int M>int A<U*,M>::f(){return sizeof(U)+M;}
int main(){A<int*,3>a;return a.f()-7;}""",
'partial_pack_owner': """template<class...T>struct L{};template<class T>struct A;
template<class...T>struct A<L<T...>>{int f();};
template<class...U>int A<L<U...>>::f(){return sizeof...(U);}
int main(){A<L<>>a;A<L<int,char>>b;return a.f()+b.f()-2;}""",
'partial_unused_member': """template<class T>struct A;template<class T>struct A<T*>{int f();int bad();};
template<class U>int A<U*>::f(){return 7;}template<class U>int A<U*>::bad(){return sizeof(typename U::missing);}
int main(){A<int*>a;return a.f()-7;}""",
'partial_namespace_owner': """namespace N{template<class T>struct A;template<class T>struct A<T*>{int f();};}
template<class U>int N::A<U*>::f(){return 7;}int main(){N::A<int*>a;return a.f()-7;}""",
}
harness.BAD = {
'undeclared_partial_owner':'template<class T>struct A{int f();};template<class T>int A<T*>::f(){return 1;}',
'partial_duplicate_body':'template<class T>struct A;template<class T>struct A<T*>{int f();};template<class U>int A<U*>::f(){return 1;}template<class V>int A<V*>::f(){return 2;}',
'partial_wrong_signature':'template<class T>struct A;template<class T>struct A<T*>{int f(T);};template<class U>int A<U*>::f(char){return 1;}',
'partial_wrong_namespace':'namespace N{template<class T>struct A;template<class T>struct A<T*>{int f();};}namespace M{template<class U>int N::A<U*>::f(){return 1;}}',
'partial_missing_parameter':'template<class T>struct A;template<class T>struct A<T*>{int f();};template<class U,class V>int A<U*>::f(){return 1;}',
}
if __name__=='__main__':
 cc=Path(sys.argv[1]).resolve() if len(sys.argv)>1 else harness.ROOT/'dev/cppgm++'
 work=Path(sys.argv[2]) if len(sys.argv)>2 else Path('/tmp/pa17-definition-controls')
 sys.exit(0 if harness.run(cc,work) else 1)
