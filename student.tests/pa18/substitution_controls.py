#!/usr/bin/env python3
"""Immediate substitution controls: CC WORK; execute successes via PA8 backend."""
from pathlib import Path
import sys
import ordering_controls as runner

runner.GOOD = {
    'completed_query_alias': 'struct A;template<class T,int=sizeof(T)>char f(int);template<class>long f(...);using Before=decltype(f<A>(0));struct A{};using After=decltype(f<A>(0));static_assert(sizeof(Before)==sizeof(long), "");static_assert(sizeof(After)==sizeof(char), "");int main(){}',
    'multiple_completion_candidates': 'struct A;struct B;template<class T,class U,int=sizeof(T)>char f(int);template<class T,class U,int=sizeof(U)>int f(long);template<class,class>long f(...);static_assert(sizeof(f<A,B>(0))==sizeof(long), "");struct A{};static_assert(sizeof(f<A,B>(0))==sizeof(char), "");struct B{};static_assert(sizeof(f<A,B>(0))==sizeof(char), "");int main(){}',
    'completion_other_order': 'struct A;struct B;template<class T,class U,int=sizeof(T)>char f(int);template<class T,class U,int=sizeof(U)>int f(long);template<class,class>long f(...);static_assert(sizeof(f<A,B>(0))==sizeof(long), "");struct B{};static_assert(sizeof(f<A,B>(0))==sizeof(int), "");struct A{};static_assert(sizeof(f<A,B>(0))==sizeof(char), "");int main(){}',
    'completed_size_default': 'struct A;template<class T,int=sizeof(T)>constexpr int f(int){return 1;}template<class>constexpr int f(long){return 2;}static_assert(f<A>(0)==2, "");struct A{};static_assert(f<A>(0)==1, "");int main(){}',
    'completed_size_result': 'struct A;template<class T>auto f(int)->decltype(sizeof(T),char());template<class>long f(...);static_assert(sizeof(f<A>(0))==sizeof(long), "");struct A{};static_assert(sizeof(f<A>(0))==sizeof(char), "");int main(){}',
    'completed_alias_result': 'struct A;template<class T>using arr=int[sizeof(T)];template<class T>arr<T>*f(int);template<class>long f(...);static_assert(sizeof(f<A>(0))==sizeof(long), "");struct A{};static_assert(sizeof(*f<A>(0))==sizeof(int), "");int main(){}',
    'completed_construction': 'struct A;template<class T,class=decltype(T())>char f(int);template<class>long f(...);static_assert(sizeof(f<A>(0))==sizeof(long), "");struct A{};static_assert(sizeof(f<A>(0))==sizeof(char), "");int main(){}',
    'completed_increment': 'struct A;template<class T>T&& val();template<class T,class=decltype(++val<T*&>())>char f(int);template<class>long f(...);static_assert(sizeof(f<A>(0))==sizeof(long), "");struct A{};static_assert(sizeof(f<A>(0))==sizeof(char), "");int main(){}',
    'sizeof_default': 'struct Incomplete;template<class T,int=sizeof(T)>int f(int){return 1;}template<class>int f(...){return 2;}int main(){return f<int>(0)!=1||f<void>(0)!=2||f<Incomplete>(0)!=2||f<int[]>(0)!=2;}',
    'sizeof_alias': 'template<class T>using arr=int[sizeof(T)];template<class T>auto f(int)->decltype(sizeof(arr<T>),int()){return 1;}template<class>int f(...){return 2;}int main(){return f<int>(0)!=1||f<void>(0)!=2;}',
    'invalid_array_formation': 'template<class T,int N>using arr=T[N];template<class T,int N,class=arr<T,N>>int f(int){return 1;}template<class,int>int f(...){return 2;}int main(){return f<int,3>(0)!=1||f<int,0>(0)!=2||f<int,-1>(0)!=2||f<void,2>(0)!=2||f<int&,2>(0)!=2;}',
    'abstract_template_array': 'template<class T>struct A{virtual void g()=0;};template<class T>int f(T(*)[2]){return 1;}template<class>int f(...){return 2;}int main(){return f<A<int>>(0)!=2;}',
    'abstract_array': 'struct A{virtual void g()=0;};struct B{virtual void g(){}};template<class T>int f(T(*)[2]){return 1;}template<class>int f(...){return 2;}int main(){return f<A>(0)!=2||f<B>(0)!=1;}',
    'construction_default': 'struct A{virtual void g()=0;};struct B{B(int){}};struct C{C()=delete;};template<class T,class=decltype(T())>int f(int){return 1;}template<class>int f(...){return 2;}int main(){return f<int>(0)!=1||f<A>(0)!=2||f<B>(0)!=2||f<C>(0)!=2;}',
    'new_default': 'struct A{virtual void g()=0;};template<class T,class=decltype(::new T)>int f(int){return 1;}template<class>int f(...){return 2;}int main(){return f<int>(0)!=1||f<A>(0)!=2||f<void>(0)!=2||f<int&>(0)!=2;}',
    'abstract_call': 'template<class T>T&& val();template<class T>void take(T);struct A{virtual void g()=0;};template<class T,class=decltype(take<T>(val<T&>()))>int f(int){return 1;}template<class>int f(...){return 2;}int main(){return f<int>(0)!=1||f<A>(0)!=2;}',
    'missing_qualified_value': 'struct A{static int x;};struct B{typedef int x;};template<class T,class=decltype(T::x)>int f(int){return 1;}template<class>int f(...){return 2;}int main(){return f<A>(0)!=1||f<B>(0)!=2||f<int>(0)!=2;}',
    'incomplete_pointer_arithmetic': 'struct A;template<class T>T&& val();template<class T,class=decltype(val<T*>()+1)>int f(int){return 1;}template<class>int f(...){return 2;}int main(){return f<int>(0)!=1||f<void>(0)!=2||f<A>(0)!=2;}',
    'incomplete_pointer_increment': 'struct A;template<class T>T&& val();template<class T,class=decltype(++val<T*&>())>int f(int){return 1;}template<class>int f(...){return 2;}int main(){return f<int>(0)!=1||f<void>(0)!=2||f<A>(0)!=2;}',
    'explicit_default_expression': 'template<class T>T&& val();template<class T,class=decltype(val<T>()++)>int f(int){return 1;}template<class>int f(...){return 2;}int main(){return f<int&>(0)!=1||f<const int&>(0)!=2||f<void*&>(0)!=2;}',
    'explicit_result_expression': 'template<class T>T&& val();template<class T>auto f(int)->decltype(val<T>()++,int()){return 1;}template<class>int f(...){return 2;}int main(){return f<int&>(0)!=1||f<const int&>(0)!=2;}',
    'deduced_result_expression': 'template<class T>auto f(T&t,int)->decltype(t++,int()){return 1;}int f(...){return 2;}int main(){int i=0;const int j=0;return f(i,0)!=1||f(j,0)!=2;}',
    'explicit_alias_failure': 'template<class T>using ptr=T*;template<class T>ptr<T> f(int){return 0;}template<class>int f(...){return 7;}int main(){return f<int>(0)!=0||f<int&>(0)!=7;}',
    'explicit_missing_result': 'struct A{typedef int type;};template<class T>typename T::type f(int){return 1;}template<class>int f(...){return 2;}int main(){return f<A>(0)!=1||f<int>(0)!=2;}',
    'failed_query_reuse': 'template<class T>T&& val();template<class T,class=decltype(val<T>()++)>int f(int){return 1;}template<class>int f(...){return 2;}int main(){return f<const int&>(0)+f<const int&>(0)+f<int&>(0)!=5;}',
    'unselected_body': 'template<class T>typename T::type f(int){return T::missing;}template<class>int f(...){return 2;}int main(){return f<int>(0)!=2;}',
}
runner.BAD = {
    'sizeof_class_side_effect': 'template<class T>struct A{typename T::bad x;};template<class T,int=sizeof(A<T>)>int f(int);template<class>int f(...);int main(){return f<int>(0);}',
    'fixed_invalid_size': 'template<class T,int=sizeof(void)>int f(T);int main(){}',
    'fixed_invalid_constructor': 'struct A{A(int);};template<class T,class=decltype(A())>int f(T);int main(){}',
    'class_side_effect': 'template<class T>struct A{typedef typename T::missing type;};template<class T>typename A<T>::type f(int);template<class>int f(...);int main(){return f<int>(0);}',
    'selected_body': 'struct A{typedef int type;};template<class T>typename T::type f(int){return T::missing;}template<class>int f(...){return 2;}int main(){return f<A>(0);}',
    'fixed_invalid_expression': 'template<class T>auto f(T)->decltype(*1);int main(){}',
    'no_surviving_candidate': 'template<class T>typename T::missing f(int);int main(){return f<int>(0);}',
}
if __name__ == '__main__':
    sys.exit(0 if runner.run(Path(sys.argv[1]).resolve(),Path(sys.argv[2])) else 1)
