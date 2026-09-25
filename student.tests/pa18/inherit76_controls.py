#!/usr/bin/env python3
"""Inherited template candidate ownership and consumers: CC WORK."""
from pathlib import Path
import sys
import ordering_controls as runner
E='template<bool,class T=void>struct E{};template<class T>struct E<true,T>{using type=T;};'
S='template<class A,class B>struct S{static const bool value=false;};template<class A>struct S<A,A>{static const bool value=true;};'
Q='template<class T>T&&dv();template<class...>using Void=void;template<class T,class A,class=void>struct Has{static const bool value=false;};template<class T,class A>struct Has<T,A,Void<decltype(T(dv<A>()))>>{static const bool value=true;};'
runner.GOOD={
 'ordinary':'struct B{int n;template<class T>B(T n):n(n){}};struct D:B{using B::B;};int main(){D a(7);D b(9L);return a.n!=7||b.n!=9;}',
 'dependent':'template<class T>struct B{T n;template<class U>B(U n):n(n){}};template<class T>struct D:B<T>{using B<T>::B;};int main(){D<int>a(7);D<long>b(9);return a.n!=7||b.n!=9;}',
 'base_alias':'struct B{int n;template<class T>B(T n):n(n){}};using A=B;struct D:A{using A::A;};int main(){D a(7);return a.n!=7;}',
 'chain':'struct B{int n;template<class T>B(T n):n(n){}};struct D:B{using B::B;};struct F:D{using D::D;};int main(){F a(7);return a.n!=7;}',
 'lref':'struct B{int&n;template<class T>B(T&&n):n(n){}};struct D:B{using B::B;};int main(){int n=7;D d(n);d.n=9;return n!=9;}',
 'constref':'struct B{const int&n;template<class T>B(T&&n):n(n){}};struct D:B{using B::B;};int main(){const int n=7;D d(n);return &d.n!=&n;}',
 'array':'struct B{int n;template<class T,int N>B(T(&)[N]):n(N){}};struct D:B{using B::B;};int main(){int a[5];D d(a);return d.n!=5;}',
 'pack':'struct B{int n;template<class...T>B(T&&...):n(sizeof...(T)){}};struct D:B{using B::B;};int main(){int n;D a(n,2L,3);return a.n!=3;}',
 'pack_empty':'struct B{int n;template<class...T>B(T&&...):n(sizeof...(T)){}};struct D:B{using B::B;};int main(){D a;return a.n;}',
 'default_head':'struct B{int n;template<class T=int>B(int n):n(n+sizeof(T)){}};struct D:B{using B::B;};int main(){D a(7);return a.n!=11;}',
 'function_default':'int count;int def(){++count;return 5;}struct B{int n;template<class T>B(T n,int x=def()):n(n+x){}};struct D:B{using B::B;};int main(){D a(7);D b(2,3);return a.n!=12||b.n!=5||count!=1;}',
 'dependent_default':'struct B{int n;template<class T>B(T n,int x=sizeof(T)):n(n+x){}};struct D:B{using B::B;};int main(){D a(7);D b(2L);return a.n!=11||b.n!=10;}',
 'namespace_default':'namespace N{int n=5;struct B{int v;template<class T>B(T x,int a=n):v(x+a){}};}int n=99;struct D:N::B{using B::B;};int main(){D d(2);return d.v!=7;}',
 'sfinae':E+S+'struct B{int n;template<class T,typename E<S<T,int>::value,int>::type=0>B(T n):n(n){}B(...):n(9){}};struct D:B{using B::B;};int main(){D a(7);D b(2L);return a.n!=7||b.n!=9;}',
 'ordinary_preferred':'struct B{int n;template<class T>B(T):n(1){}B(int):n(2){}};struct D:B{using B::B;};int main(){D a(7);D b(2L);return a.n!=2||b.n!=1;}',
 'local_ordinary':'struct B{int n;template<class T>B(T):n(1){}};struct D:B{using B::B;D(int n):B(n+2){this->n=n+2;}};int main(){D a(7);D b(2L);return a.n!=9||b.n!=1;}',
 'local_template':'struct B{int n;template<class T>B(T):n(1){}};struct D:B{using B::B;template<class U>D(U):B(0){n=2;}};int main(){D a(7);return a.n!=2;}',
 'ordering':'struct B{int n;template<class T>B(T):n(1){}template<class T>B(T*):n(2){}};struct D:B{using B::B;};int main(){int n;D a(&n);D b(n);return a.n!=2||b.n!=1;}',
 'noexcept':'struct B{template<class T>B(T)noexcept{}};struct D:B{using B::B;};static_assert(noexcept(D(3)),"");int main(){}',
 'noexcept_dependent':'struct B{template<class T>B(T)noexcept(sizeof(T)==4){}};struct D:B{using B::B;};static_assert(noexcept(D(3))&&!noexcept(D(3L)),"");int main(){}',
 'noexcept_member':'struct M{M()noexcept(false){}};struct B{template<class T>B(T)noexcept{}};struct D:B{using B::B;M m;};static_assert(!noexcept(D(3)),"");int main(){D d(3);}',
 'query_dormant':Q+'struct B{template<class T>B(T){typename T::missing n;}};struct D:B{using B::B;};static_assert(Has<D,int>::value,"");int main(){}',
 'query_private':Q+'class B{template<class T>B(T){}};struct D:B{using B::B;};static_assert(!Has<D,int>::value,"");int main(){}',
 'query_deleted':Q+'struct B{template<class T>B(T)=delete;};struct D:B{using B::B;};static_assert(!Has<D,int>::value,"");int main(){}',
 'private_inheritance':'struct B{int n;template<class T>B(T n):n(n){}};class D:private B{using B::B;public:int get(){return n;}};int main(){D d(7);return d.get()!=7;}',
 'copy_conversion':'struct B{int n;template<class T>B(T n):n(n){}};struct D:B{using B::B;};int take(D d){return d.n;}int main(){return take(7)!=7;}',
 'constexpr':'struct B{int n;template<class T>constexpr B(T n):n(n){}};struct D:B{using B::B;};constexpr D d(7);static_assert(d.n==7,"");int main(){return d.n!=7;}',
 'constexpr_default':'struct B{int n;template<class T>constexpr B(T n,int a=2):n(n+a){}};struct D:B{using B::B;};constexpr D d(7);static_assert(d.n==9,"");int main(){return d.n!=9;}',
 'member_init':'int count;int f(){++count;return 8;}struct B{int n;template<class T>B(T n):n(n){}};struct D:B{using B::B;int m=f();};int main(){D d(7);return d.n!=7||d.m!=8||count!=1;}',
 'default_coexists':'struct B{int n;B():n(4){}template<class T>B(T n):n(n){}};struct D:B{using B::B;};int main(){D a;D b(7);return a.n!=4||b.n!=7;}',
 'list':'struct B{int n;template<class T>B(T n):n(n){}};struct D:B{using B::B;};int main(){D a{7};D b={9};return a.n!=7||b.n!=9;}',
}
runner.BAD={
 'private':'class B{template<class T>B(T){}};struct D:B{using B::B;};int main(){D d(3);}',
 'protected':'struct B{protected:template<class T>B(T){}};struct D:B{using B::B;};int main(){D d(3);}',
 'deleted':'struct B{template<class T>B(T)=delete;};struct D:B{using B::B;};int main(){D d(3);}',
 'explicit_copy':'struct B{template<class T>explicit B(T){}};struct D:B{using B::B;};int main(){D d=3;}',
 'explicit_copy_list':'struct B{template<class T>explicit B(T){}};struct D:B{using B::B;};int main(){D d={3};}',
 'selected_body':'struct B{template<class T>B(T){typename T::missing n;}};struct D:B{using B::B;};int main(){D d(3);}',
 'deleted_member':'struct M{M()=delete;};struct B{template<class T>B(T){}};struct D:B{using B::B;M m;};int main(){D d(3);}',
 'reference_member':'struct B{template<class T>B(T){}};struct D:B{using B::B;int&r;};int main(){D d(3);}',
 'sfinae_reject':E+'struct B{template<class T,typename E<(sizeof(T)==1),int>::type=0>B(T){}};struct D:B{using B::B;};int main(){D d(3);}',
 'ambiguous':'struct B{template<class T>B(T*,long){}template<class T>B(const T*,int){}};struct D:B{using B::B;};int main(){int n;D d(&n,0);}',
}
if __name__=='__main__':sys.exit(0 if runner.run(Path(sys.argv[1]).resolve(),Path(sys.argv[2])) else 1)
