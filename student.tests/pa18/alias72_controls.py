#!/usr/bin/env python3
"""Retained alias formation: CC WORK. Positive cases execute generated LowIR."""
from pathlib import Path
import sys
import ordering_controls as runner
FIRST='template<class T,class...>using First=T;template<class...>using Void=void;'
SAME='template<class A,class B>struct Same{static const bool value=false;};template<class A>struct Same<A,A>{static const bool value=true;};'
TAG='struct Good{using type=int;};struct Bad{};'
runner.GOOD={
 'erased_result':FIRST+TAG+'template<class T>First<int,typename T::type>f(T){return 1;}int f(...){return 2;}int main(){return f(Good())!=1||f(Bad())!=2;}',
 'erased_default':FIRST+TAG+'template<class T,class=Void<typename T::type>>int f(T){return 1;}int f(...){return 2;}int main(){return f(Bad())!=2||f(Good())!=1||f(Bad())!=2;}',
 'dependent_result':FIRST+TAG+'template<class T,class U>First<T,typename U::type>f(T x,U){return x;}int f(...){return 9;}int main(){return f(3,Good())!=3||f(3,Bad())!=9;}',
 'nested_erasure':FIRST+TAG+'template<class T>using Check=Void<First<int,typename T::type>>;template<class T,class=Check<T>>int f(T){return 1;}int f(...){return 2;}int main(){return f(Good())!=1||f(Bad())!=2;}',
 'partial_detector':FIRST+TAG+'template<class T,class=void>struct D{static const int n=0;};template<class T>struct D<T,Void<typename T::type>>{static const int n=1;};int main(){return D<Bad>::n!=0||D<Good>::n!=1;}',
 'pack_result':FIRST+TAG+'template<class...T>First<int,typename T::type...>f(T...){return 1;}int f(...){return 2;}int main(){return f(Good(),Good())!=1||f(Good(),Bad())!=2;}',
 'pack_empty':FIRST+'template<class...T>First<int,typename T::type...>f(){return 1;}int main(){return f<>()!=1;}',
 'pack_nested_alias':FIRST+TAG+'template<class T>using Check=Void<typename T::type>;template<class...T>First<int,Check<T>...>f(T...){return 1;}int f(...){return 2;}int main(){return f(Good(),Good())!=1||f(Good(),Bad())!=2;}',
 'alias_identity':SAME+'template<class T>using Id=T;template<class T>int f(Id<T>){return 1;}int main(){return f(3)!=1;}',
 'alias_pointer':SAME+'template<class T>using Ptr=T*;template<class T>int f(Ptr<T>){return 1;}int main(){int n;return f(&n)!=1;}',
 'alias_array':'template<class T>using Arr=T[3];template<class T>int f(Arr<T>&){return 1;}int main(){int a[3];return f(a)!=1;}',
 'alias_forwarding':'template<class T>using Id=T;template<class T>int f(Id<T>&&){return 1;}int main(){int n;return f(n)!=1||f(0)!=1;}',
 'alias_redeclaration':'template<class T>using Id=T;template<class T>int f(Id<T>);template<class U>int f(U){return 1;}int main(){return f(3)!=1;}',
 'alias_cv':SAME+'template<class T>using Id=T;template<class T>int f(const Id<T>&){return 1;}int main(){const int n=3;return f(n)!=1;}',
 'erased_pointer_ref':FIRST+'template<class T,class=Void<T*>>int f(int){return 1;}template<class>int f(...){return 2;}int main(){return f<int>(0)!=1||f<int&>(0)!=2;}',
 'erased_void_ref':FIRST+'template<class T,class=Void<T&>>int f(int){return 1;}template<class>int f(...){return 2;}int main(){return f<int>(0)!=1||f<void>(0)!=2;}',
 'erased_array':FIRST+'template<class T,class=Void<T[2]>>int f(int){return 1;}template<class>int f(...){return 2;}int main(){return f<int>(0)!=1||f<void>(0)!=2;}',
 'erased_deleted':FIRST+'struct Good{void f();};struct Bad{void f()=delete;};template<class T>T&v();template<class T,class=Void<decltype(v<T>().f())>>int f(int){return 1;}template<class>int f(...){return 2;}int main(){return f<Good>(0)!=1||f<Bad>(0)!=2;}',
 'erased_private':FIRST+'struct Good{using type=int;};class Bad{using type=int;};template<class T,class=Void<typename T::type>>int f(int){return 1;}template<class>int f(...){return 2;}int main(){return f<Good>(0)!=1||f<Bad>(0)!=2;}',
 'erased_default_arity':FIRST+'template<class T,class U=typename T::type>using Check=int;struct G{using type=void;};template<class T,class=Check<T>>int f(T){return 1;}int f(...){return 2;}int main(){return f(G())!=1||f(0)!=2;}',
 'dormant_body':FIRST+TAG+'template<class T>int body(){return T::missing;}template<class T>First<int,decltype(body<T>())>f(T){return 1;}int main(){return f(Bad())!=1;}',
 'outer_member_alias':FIRST+'template<class T>struct A{template<class U>using Check=First<T,typename U::type>;template<class U>Check<U>f(U){return 3;}T f(...){return 8;}};struct G{using type=int;};int main(){A<int>a;A<long>b;return a.f(G())!=3||a.f(0)!=8||b.f(G())!=3||b.f(0)!=8;}',
 'erased_reference_identity':FIRST+'template<class T>First<int&,typename T::type>f(T,int&n){return n;}struct G{using type=int;};int main(){int n=3;f(G(),n)=7;return n!=7;}',
}
runner.BAD={
 'hard_erased_alias':FIRST+'using X=Void<typename int::type>;int main(){}',
 'hard_substitution':FIRST+'template<class T>using Check=Void<typename T::missing>;Check<int>*p;int main(){}',
 'class_side_effect':FIRST+'template<class T>struct Explode{using type=typename T::missing;};template<class T,class=Void<typename Explode<T>::type>>int f(T){return 1;}int f(...){return 2;}int main(){return f(0);}',
 'transparent_redefinition':'template<class T>using Id=T;template<class T>int f(Id<T>){return 1;}template<class U>int f(U){return 2;}int main(){}',
 'erased_redefinition':FIRST+'template<class T>First<int,typename T::A>f(T){return 1;}template<class T>First<int,typename T::B>f(T){return 2;}int main(){}',
}
if __name__=='__main__':sys.exit(0 if runner.run(Path(sys.argv[1]).resolve(),Path(sys.argv[2])) else 1)
