#!/usr/bin/env python3
"""Specialized transfers and automatic initializers, with live runtime checks.

N3485 [class.copy]/15,28; [conv.integral]; [expr.ref]; [dcl.init.aggr].
Keep evaluated initializer operands and nontrivial/volatile subobject actions.
"""
from pathlib import Path
import sys
import entity_controls as runner
runner.GOOD = {
 'specialized_signed_store': 'template<class T>struct X{T n;};int main(){X<long>x;x.n=-7;return x.n!=-7;}',
 'specialized_unsigned_store': 'template<class T>struct X{T n;};int main(){X<unsigned long>x;x.n=-1;return x.n!=~0UL;}',
 'specialized_narrow_store': 'template<class T>struct X{T n;};int main(){X<unsigned char>x;x.n=259;return x.n!=3;}',
 'specialized_volatile_store': 'template<class T>struct X{volatile T n;};int main(){X<long>x;x.n=-3;return x.n!=-3;}',
 'ordinary_store_boundary': 'struct X{long n;void set(){n=7;n=0;}};int main(){X x;x.n=7;x.n=0;x.set();return x.n;}',
 'empty_member_assignment': 'template<class>struct E{};struct X{E<int>e;int n;};int main(){X a,b;a.n=19;b.n=3;b=a;return b.n!=19;}',
 'empty_member_move': 'template<class>struct E{};struct X{int a;E<int>e;long b;};int main(){X a,b;a.a=19;a.b=31;b=static_cast<X&&>(a);return b.a!=19||b.b!=31;}',
 'empty_base_payload': 'struct E{};template<class T>struct X:E{T n;};int main(){X<int>a,b;a.n=123;b.n=4;b=a;return b.n!=123;}',
 'nontrivial_empty_assignment': 'int count;struct E{E&operator=(const E&){++count;return *this;}};template<class T>struct X{T e;int n;};int main(){X<E>a,b;a.n=7;b=a;return count!=1||b.n!=7;}',
 'volatile_sparse_assignment': 'struct E{};template<class T>struct X{E e;volatile T n;};int main(){X<int>a,b;a.n=11;b=a;return b.n!=11;}',
 'reference_copy_constructor': 'struct E{};template<class T>struct X{E e;T&r;X(T&v):r(v){}};int main(){int n=7;X<int>a(n);X<int>b=a;b.r=13;return n!=13;}',
 'qualified_base_receiver': 'struct Pad{long n;};struct B{int n;int f(){return n;}};template<class T>struct Mid:Pad,T{};template<class T>struct D:Mid<T>{int f(){return Mid<T>::f()+1;}};int main(){D<B>x;x.Pad::n=8;x.B::n=13;return x.f()!=14;}',
 'unqualified_base_receiver': 'struct Pad{long n;};struct B{int n;int f(){return n;}};struct M:Pad,B{};template<class T>struct D:T{};int main(){D<M>x;x.Pad::n=8;x.B::n=13;return x.f()!=13;}',
 'qualified_explicit_object': 'struct Pad{long n;};struct B{int n;int f(){return n;}};struct M:Pad,B{};template<class T>struct D:T{};int main(){D<M>x;x.B::n=13;return x.M::f()!=13;}',
 'ambiguous_conversion_fallback': 'struct B{};struct L:B{};struct R:B{};template<class T>struct D:L,T{};int f(B*){return 1;}int f(void*){return 2;}int main(){D<R>x;return f(&x)!=2;}',
 'base_pointer_null_and_live': 'struct Pad{long n;};struct B{int n;};struct M:Pad,B{};template<class T>struct D:T{};B*up(D<M>*p){return p;}int main(){D<M>x;x.B::n=13;return up(0)!=0||up(&x)->n!=13;}',
 'constant_base_address': 'struct Pad{long n;};struct B{int n;};struct M:Pad,B{};template<class T>struct D:T{};D<M>x;B*const p=&x;int main(){x.B::n=13;return p->n!=13;}',
 'array_operand_order': 'int n;int next(){return ++n;}template<class T>int f(){int a[]={next(),next(),next()};return a[0]+a[1]*10+a[2]*100;}int main(){return f<int>()!=321||n!=3;}',
 'array_temporary_effect': 'int n;struct E{E(){++n;}~E(){n+=10;}};template<class T>int f(){int a[]={(T(),7),(T(),8)};return a[0]+a[1];}int main(){int r=f<E>();return r!=15||n!=22;}',
 'array_short_long_lanes': 'template<class T>int f(){T a[8]={1,2,3,4,5,6,7,8};T b[9]={1,2,3,4,5,6,7,8,9};a[7]+=b[8];return a[7];}int main(){return f<long>()!=17;}',
 'array_multidimensional': 'template<class T>int f(){T a[2][5]={{1,2,3},{4,5}};a[1][4]=7;return a[0][2]+a[1][1]+a[1][4];}int main(){return f<int>()!=15;}',
 'array_volatile_lanes': 'template<class T>int f(){volatile T a[2]={3,4};return a[0]+a[1];}int main(){return f<int>()!=7;}',
 'array_constexpr_materialization': 'struct E{constexpr E(){}};int main(){constexpr int a[]={(E(),3),4};static_assert(a[0]==3,"value");return a[1]!=4;}',
 'array_constexpr_identity': 'int main(){constexpr int a[2]={3,4};constexpr int b[2]={3,4};static_assert(a[1]==4,"value");return a==b||b[0]!=3;}',
 'array_empty_pack_operands': 'template<class T>struct E{};template<class...T>int f(){int a[]={(E<T>(),sizeof(T))...};return a[0]+a[1];}int main(){return f<char,int>()!=5;}',
}
runner.BAD = {
 'deleted_sparse_assignment': 'struct E{};template<class T>struct X{E e;const T n;};int main(){X<int>a={E(),1},b={E(),2};a=b;}',
 'ambiguous_base_receiver': 'struct B{int f(){return 1;}};struct L:B{};struct R:B{};template<class T>struct D:L,T{};int main(){D<R>x;return x.f();}',
}
if __name__ == '__main__':
 sys.exit(0 if runner.run(Path(sys.argv[1]).resolve(),Path(sys.argv[2])) else 1)
