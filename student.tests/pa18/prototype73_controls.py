#!/usr/bin/env python3
"""Member prototype object and using-exposure facts: CC WORK."""
from pathlib import Path
import sys
import ordering_controls as runner
SAME='template<class A,class B>struct Same{static const bool value=false;};template<class A>struct Same<A,A>{static const bool value=true;};'
runner.GOOD={
 'this_pointer':SAME+'struct X{auto self()->decltype(this){return this;}};static_assert(Same<decltype(((X*)0)->self()),X*>::value,"");int main(){X x;return x.self()!=&x;}',
 'this_const':SAME+'struct X{auto self()const->decltype(this){return this;}};static_assert(Same<decltype(((X*)0)->self()),const X*>::value,"");int main(){X x;return x.self()!=&x;}',
 'this_volatile':SAME+'struct X{auto self()volatile->decltype(this){return this;}};static_assert(Same<decltype(((X*)0)->self()),volatile X*>::value,"");int main(){X x;return x.self()!=&x;}',
 'class_this':SAME+'template<class T>struct X{auto self()->decltype(this){return this;}};int main(){X<int>x;X<long>y;static_assert(Same<decltype(x.self()),X<int>*>::value,"");return x.self()!=&x||y.self()!=&y;}',
 'member_this':'template<class T>int tag(T*p){return p!=0;}struct X{template<class T>auto f(T)->decltype(tag(this)){return tag(this);}};int main(){X x;return x.f(0)!=1||x.f(0L)!=1;}',
 'dependent_member_this':'template<class T>int tag(T*p){return p!=0;}template<class U>struct X{template<class T>auto f(T)->decltype(tag(this)){return tag(this);}};int main(){X<int>x;X<long>y;return x.f(0)!=1||y.f(0L)!=1;}',
 'prototype_cv_overload':SAME+'struct X{char g();long g()const;template<class T>auto f(T)const->decltype(g()){return 3;}};int main(){X x;static_assert(Same<decltype(x.f(0)),long>::value,"");return x.f(0)!=3;}',
 'prototype_class_cv':SAME+'template<class T>struct X{char g();long g()const;template<class U>auto f(U)const->decltype(g()){return 3;}};int main(){X<int>x;static_assert(Same<decltype(x.f(0)),long>::value,"");return x.f(0)!=3;}',
 'prototype_const_field':SAME+'template<class T>struct X{T n;template<class U>auto f(U)const->decltype((n)){return n;}};int main(){X<int>x={3};static_assert(Same<decltype(x.f(0)),const int&>::value,"");return x.f(0)!=3;}',
 'prototype_decltype_field':SAME+'struct X{int n;auto f()const->decltype(n){return n;}};int main(){X x={3};static_assert(Same<decltype(x.f()),int>::value,"");return x.f()!=3;}',
 'prototype_member_set':'template<class T>struct E{};template<class T>struct X{char g(E<int>);long g(E<long>);template<class U>auto f(E<U>u)->decltype(g(u)){return 0;}};int main(){X<char>x;return sizeof(x.f(E<int>()))!=1||sizeof(x.f(E<long>()))!=sizeof(long);}',
 'using_template_public':'struct B{template<class T>int f(T n){return n;}};class D:private B{public:using B::f;};int main(){D d;return d.f(3)!=3||d.f(4L)!=4;}',
 'using_template_query':'struct B{template<class T>char f(T);};class D:private B{public:using B::f;};template<class T>T&dv();template<class T>auto f(int)->decltype(dv<T>().f(0));template<class>long f(...);int main(){return sizeof(f<D>(0))!=1;}',
 'using_template_operator':'struct B{template<class T>int operator()(T n){return n;}};class D:private B{public:using B::operator();};int main(){D d;return __builtin_invoke(d,3)!=3;}',
 'using_class_template':'template<class T>struct B{template<class U>int f(U n){return n;}};template<class T>class D:private B<T>{public:using B<T>::f;};int main(){D<int>d;D<long>e;return d.f(3)!=3||e.f(4L)!=4;}',
 'using_ref_qualification':'struct B{template<class T>int f(T)&{return 3;}template<class T>int f(T)const&{return 4;}};class D:private B{public:using B::f;};int main(){D d;const D c;return d.f(0)!=3||c.f(0)!=4;}',
}
runner.GOOD.update({
 'explicit_this_field':SAME+'template<class T>struct X{T n;template<class U>auto f(U)const->decltype((this->n)){return n;}};int main(){X<int>x={3};static_assert(Same<decltype(x.f(0)),const int&>::value,"");return x.f(0)!=3;}',
 'explicit_this_decltype':SAME+'struct X{int n;auto f()const->decltype(this->n){return n;}};int main(){X x={3};static_assert(Same<decltype(x.f()),int>::value,"");return x.f()!=3;}',
 'constexpr_this_cv':SAME+'struct X{constexpr auto self()->decltype(this){return this;}};int main(){X x;static_assert(Same<decltype(x.self()),const X*>::value,"");return x.self()!=&x;}',
 'out_of_class_this':'struct X{template<class T>auto f(T)->decltype(this);};template<class T>auto X::f(T)->decltype(this){return this;}int main(){X x;return x.f(0)!=&x;}',
 'out_of_class_const_this':'template<class T>struct X{template<class U>auto f(U)const->decltype(this);};template<class V>template<class U>auto X<V>::f(U)const->decltype(this){return this;}int main(){X<int>x;X<long>y;return x.f(0)!=&x||y.f(0)!=&y;}',
 'static_fixed_member':SAME+'struct X{static char g();template<class T>static auto f(T)->decltype(g()){return 3;}};int main(){return X::f(0)!=3;}',
})

runner.GOOD.update({
 'using_template_hides_after':'struct B{template<class T>int f(const T&){return 1;}};struct D:B{template<class U>int f(const U&){return 2;}using B::f;};int main(){D d;return d.f(0)!=2;}',
 'using_template_hides_before':'struct B{template<class T>int f(const T&){return 1;}};struct D:B{using B::f;template<class U>int f(const U&){return 2;}};int main(){D d;return d.f(0)!=2;}',
 'using_template_distinct':'struct B{template<class T>int f(T*){return 1;}};class D:private B{public:using B::f;template<class T>int f(T){return 2;}};int main(){D d;int n;return d.f(&n)!=1||d.f(0)!=2;}',
 'using_nontemplate_distinct':'struct B{int f(int){return 1;}};struct D:B{template<class T>int f(T){return 2;}using B::f;};int main(){D d;return d.f(0)!=1||d.f(0L)!=2;}',
 'using_ref_shapes':'struct B{template<class T>int f(T)&{return 1;}};struct D:B{template<class T>int f(T)&&{return 2;}using B::f;};int main(){D d;return d.f(0)!=1||D().f(0)!=2;}',
 'nested_fixed_base':'int f(){return 1;}struct B{int f(){return 2;}};template<class T>struct Other{};template<class T>struct X:Other<T>{struct Y:B{int run(){return f();}};};int main(){X<int>::Y y;return y.run()!=2;}',
 'nested_this':'struct B{int n;};template<class T>struct X{struct Y:B{template<class U>auto f(U)->decltype((this->n)){return n;}};};int main(){X<int>::Y y;y.n=7;return y.f(0)!=7;}',
})

runner.BAD={
 'static_this':'struct X{static auto f()->decltype(this);};int main(){}',
 'static_template_this':'struct X{template<class T>static auto f(T)->decltype(this);};int main(){}',
 'static_class_template_this':'template<class T>struct X{static auto f()->decltype(this);};int main(){X<int>x;}',
 'parameter_this':'struct X{int f(decltype(this));};int main(){}',
 'free_this':'auto f()->decltype(this);int main(){}',
 'using_template_private':'struct B{template<class T>int f(T n){return n;}};class D:private B{using B::f;};int main(){D d;return d.f(3);}',
 'using_template_deleted':'struct B{template<class T>int f(T)=delete;};class D:private B{public:using B::f;};int main(){D d;return d.f(3);}',
}
if __name__=='__main__':sys.exit(0 if runner.run(Path(sys.argv[1]).resolve(),Path(sys.argv[2])) else 1)
