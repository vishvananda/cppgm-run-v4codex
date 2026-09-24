#!/usr/bin/env python3
"""Retained assignment/destructor/list query controls: CC WORK."""
from pathlib import Path
import sys
import ordering_controls as runner
runner.GOOD={}
runner.BAD={}
assignment='template<class T>T&& v(); template<class T,class U,class=decltype(v<T>() OP v<U>())>char f(int);template<class,class>long f(...);'
for op in ('=','+=','-=','*=','/=','%=','&=','|=','^=','<<=','>>='):
    probe=assignment.replace('OP',op)
    runner.GOOD['assignment_'+op.replace('=','assign').replace('/','div')]=probe+'static_assert(sizeof(f<int&,int>(0))==1,"");static_assert(sizeof(f<const int&,int>(0))==sizeof(long),"");static_assert(sizeof(f<int,int>(0))==sizeof(long),"");int main(){}'
for typ,valid in [('int*',True),('int(*)[3]',True),('void*',False),('int(*)()',False),('Incomplete*',False),('int(*)[]',False)]:
    runner.GOOD['pointer_sum_'+str(len(runner.GOOD))]='struct Incomplete;using P='+typ+';'+assignment.replace('OP','+=')+f'static_assert((sizeof(f<bool&,P>(0))==1)=={str(valid).lower()},"");int main(){{}}'
runner.GOOD.update({
 'assign_overload':'struct A{int&operator+=(int);};'+assignment.replace('OP','+=')+'static_assert(sizeof(f<A&,int>(0))==1,"");static_assert(sizeof(f<const A&,int>(0))==sizeof(long),"");int main(){}',
 'assign_unrelated':'struct A{};int operator+=(A&,A);struct X{};'+assignment.replace('OP','+=')+'static_assert(sizeof(f<X&,X>(0))==sizeof(long),"");int main(){}',
 'assign_deleted':'struct A{int&operator+=(int)=delete;};'+assignment.replace('OP','+=')+'static_assert(sizeof(f<A&,int>(0))==sizeof(long),"");int main(){}',
 'assign_ref_conversion':'struct A{operator int&();};'+assignment.replace('OP','+=')+'static_assert(sizeof(f<A&,int>(0))==1,"");int main(){}',
 'assign_result':'template<class T>T&&v();template<class A,class B>struct Same{static const bool value=false;};template<class T>struct Same<T,T>{static const bool value=true;};static_assert(Same<decltype(v<volatile int&>()+=2),volatile int&>::value,"");int main(){}',
 'assign_pointer_runtime':'int main(){int a[2];bool b=false;b+=a;return !b;}',
 'assign_runtime':'template<class T>auto add(T&x)->decltype(x+=2){return x+=2;}int main(){int n=3;int&r=add(n);return n!=5||&r!=&n;}',
 'assign_bitfield':'struct A{unsigned n:3;};template<class T>T&v();template<class T,class=decltype(v<T>().n+=1)>char f(int);template<class>long f(...);static_assert(sizeof(f<A>(0))==1,"");int main(){}',
 'assign_ref_runtime':'int calls;struct A{int n;operator int&(){++calls;return n;}};int main(){A a{3};int&r=(a+=2);return calls!=1||a.n!=5||&r!=&a.n;}',
 'assign_ref_float_runtime':'int calls;struct A{short n;operator short&(){++calls;return n;}};int main(){A a{3};a*=2.5;return calls!=1||a.n!=7;}',
 'assign_ref_template_runtime':'int calls;struct A{int n;operator int&(){++calls;return n;}};template<class T>void f(T&a){a+=2;}int main(){A a{3};f(a);return calls!=1||a.n!=5;}',
 'assign_ref_fixed_runtime':'int calls;struct A{short n;operator short&(){++calls;return n;}};template<class T>void f(A&a){a*=2.5;}int main(){A a{3};f<int>(a);return calls!=1||a.n!=7;}',
 'assign_implicit_class':'struct A{};'+assignment.replace('OP','=')+'static_assert(sizeof(f<A&,A>(0))==1,"");int main(){}',
 'assign_nullptr':'using N=decltype(nullptr);'+assignment.replace('OP','=')+'static_assert(sizeof(f<N&,N>(0))==1,"");int main(){}',
 'assign_address_bitfield':'struct A{int n:3;};template<class T>T&v();template<class T,class=decltype(&(v<T>().n+=1))>char f(int);template<class>long f(...);static_assert(sizeof(f<A>(0))==sizeof(long),"");int main(){}',
})
destructor='template<class T>T&& v();template<class T,class=decltype(v<T&>().~T())>char f(int);template<class>long f(...);'
for typ,decl,valid in [('int','',True),('const int','',True),('int*','',True),('int[2]','',False),('int()','',False),('A','struct A{};',True),('A','struct A{~A()=delete;};',False),('A','class A{~A();};',False),('A','struct A;',False),('A','struct B{~B()=delete;};struct A{B b;};',False)]:
    runner.GOOD['destructor_'+str(len(runner.GOOD))]=decl+destructor+f'static_assert((sizeof(f<{typ}>(0))==1)=={str(valid).lower()},"");int main(){{}}'
runner.GOOD.update({
 'destructor_noexcept':'template<class T>T*v()noexcept;template<class T>struct A{static const bool value=noexcept(v<T>()->~T());};struct X{~X()noexcept(false);};static_assert(A<int>::value,"");static_assert(!A<X>::value,"");int main(){}',
 'destructor_receiver_throws':'template<class T>T*v();template<class T>struct A{static const bool value=noexcept(v<T>()->~T());};static_assert(!A<int>::value,"");int main(){}',
 'destructor_qualified_scalar':'template<class T>T*v()noexcept;template<class T>struct A{static const bool value=noexcept(v<T>()->T::~T());};static_assert(A<int>::value,"");int main(){}',
 'destructor_wrong_target':'template<class T>T&v();using I=int;template<class T,class=decltype(v<T>().~I())>char f(int);template<class>long f(...);static_assert(sizeof(f<double>(0))==sizeof(long),"");static_assert(sizeof(f<int>(0))==1,"");int main(){}',
 'destructor_args':'template<class T>T&v();template<class T,class=decltype(v<T>().~T(1))>char f(int);template<class>long f(...);static_assert(sizeof(f<int>(0))==sizeof(long),"");int main(){}',
 'destructor_requires_call':'template<class T>T&v();template<class T,class=decltype(v<T>().~T)>char f(int);template<class>long f(...);static_assert(sizeof(f<int>(0))==sizeof(long),"");int main(){}',
 'destructor_body_dormant':'template<class T>struct A{~A(){T::missing();}};'+destructor+'static_assert(sizeof(f<A<int>>(0))==1,"");int main(){}',
 'destructor_template_id':'template<class T>struct A{};template<class T>T&v();template<class T,class=decltype(v<A<T>>().~A<T>())>char f(int);template<class>long f(...);static_assert(sizeof(f<int>(0))==1,"");int main(){}',
 'destructor_trailing_abi':'template<class T>auto destroy(T*p)->decltype(p->~T()){p->~T();}int main(){int n=1;destroy(&n);return n!=1;}',
 'destructor_class_abi':'int n;struct A{~A(){++n;}};template<class T>auto destroy(T*p)->decltype(p->~T()){p->~T();}int main(){A*a=new A;destroy(a);return n!=1;}',
 'destructor_fixed_alias_abi':'using I=int;template<class T>auto destroy(T*p)->decltype(p->~I()){p->~I();}int main(){int n=1;destroy(&n);return n!=1;}',
 'destructor_template_id_abi':'template<class T>struct A{};template<class T>auto destroy(A<T>*p)->decltype(p->~A<T>()){p->~A<T>();}int main(){A<int>a;destroy(&a);return 0;}',
 'destructor_completion':destructor+'struct A;static_assert(sizeof(f<A>(0))==sizeof(long),"");struct A{};static_assert(sizeof(f<A>(0))==1,"");int main(){}',
 'destructor_base':'struct B{};struct D:B{};template<class T>T&v();template<class T,class=decltype(v<T>().B::~B())>char f(int);template<class>long f(...);static_assert(sizeof(f<D>(0))==1,"");int main(){}',
 'destructor_base_runtime':'int n;struct B{~B(){++n;}};struct D:B{};template<class T>auto destroy(T*p)->decltype(p->B::~B()){p->B::~B();}int main(){D*p=new D;destroy(p);return n!=1;}',
 'call_object_const':'template<class T>T&&v();struct C{int operator()(int);};template<class T,class=decltype(v<T>()(1))>char f(int);template<class>long f(...);static_assert(sizeof(f<C&>(0))==1,"");static_assert(sizeof(f<const C&>(0))==sizeof(long),"");int main(){}',
})
if __name__=='__main__':sys.exit(0 if runner.run(Path(sys.argv[1]).resolve(),Path(sys.argv[2])) else 1)
