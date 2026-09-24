#!/usr/bin/env python3
"""Static storage demand, constant initialization and publication ordering."""
from pathlib import Path
import sys
import entity_controls as runner
runner.GOOD = {
 'reference_before_dynamic': '''int value;extern int& ref;int check(){return &ref==&value;}int observed=check();int& ref=value;int main(){return observed!=1;}''',
 'template_reference_before_dynamic': '''template<class T>struct X{static int n;};template<class T>int X<T>::n=7;extern int& ref;int check(){return &ref==&X<int>::n;}int observed=check();int&ref=X<int>::n;int main(){return observed!=1||ref!=7;}''',
 'reference_to_dynamic_class': '''struct X{int n;X():n(7){}};extern X object;extern X&ref;int check(){return &ref==&object;}int observed=check();X object;X&ref=object;int main(){return observed!=1||ref.n!=7;}''',
 'base_reference_offset': '''struct A{int a;};struct B{int b;};struct D:A,B{};D value;B&ref=value;int main(){ref.b=7;return value.b!=7||&ref!=static_cast<B*>(&value);}''',
 'local_static_function_table': '''template<int N>int f(){return N;}struct Table{int(*f)();};template<int N>int run(){static const Table t={&f<N>};return t.f();}int main(){return run<3>()!=3||run<5>()!=5||run<3>()!=3;}''',
 'constexpr_function_table': '''template<int N>int f(){return N;}struct Table{int(*f)();constexpr Table(int(*p)()):f(p){}};template<int N>struct X{static constexpr Table t=Table(&f<N>);};template<int N>constexpr Table X<N>::t;int main(){return X<7>::t.f()!=7||X<9>::t.f()!=9;}''',
 'dynamic_local_per_specialization': '''int calls;int init(int n){++calls;return n;}template<int N>int& get(){static int n=init(N);return n;}int main(){get<3>()++;return get<3>()!=4||get<7>()!=7||calls!=2;}''',
 'local_static_reference': '''template<int N>int&get(){static int value=N;static int&ref=value;return ref;}int main(){get<3>()++;return get<3>()!=4||get<7>()!=7;}''',
 'unused_invalid_initializer': '''template<class T>struct X{static int n;};template<class T>int X<T>::n=T::missing;int main(){X<int>x;(void)x;return 0;}''',
 'unused_effectful_initializer': '''int calls;int init(){++calls;return 1;}template<class T>struct X{static int n;};template<class T>int X<T>::n=init();int main(){X<int>x;(void)x;return calls;}''',
 'late_template_definition': '''template<class T>struct X{static const int n;};int before(){return X<int>::n;}template<class T>const int X<T>::n=7;int after(){return X<int>::n;}int main(){return before()!=7||after()!=7;}''',
 'explicit_static_specialization': '''template<class T>struct X{static const int n;};template<class T>const int X<T>::n=3;template<>const int X<int>::n=7;int main(){return X<char>::n!=3||X<int>::n!=7;}''',
 'explicit_static_declaration': '''template<class T>struct X{static int n;};template<class T>int X<T>::n=3;template<>int X<int>::n;template<>int X<int>::n=7;int main(){return X<char>::n!=3||X<int>::n!=7;}''',
 'static_member_object_read': '''template<class T>struct X{static const int n;};template<class T>const int X<T>::n=7;int main(){X<int>x;return x.n!=7;}''',
 'static_member_late_object_read': '''struct X{static const int n;};int before(X&x){return x.n;}const int X::n=7;int main(){X x;return before(x)!=7;}''',
 'mutable_dynamic_table': '''int calls;int one(){return 1;}int two(){return 2;}typedef int(*F)();F init(){++calls;return &one;}template<int N>F&get(){static F p=init();return p;}int main(){get<1>()=&two;return get<1>()()!=2||get<2>()()!=1||calls!=2;}''',
 'empty_value_initialization': '''struct X{};int main(){X x=X();return *reinterpret_cast<unsigned char*>(&x)!=0;}''',
 'static_mutable_read': '''template<class T>struct X{static int n;};template<class T>int X<T>::n=3;int main(){X<int>::n=9;return X<int>::n!=9;}''',
 'const_volatile_read': '''template<class T>struct X{static const volatile int n;};template<class T>const volatile int X<T>::n=3;int main(){return X<int>::n!=3;}''',
 'local_class_table': '''int f(){return 7;}template<class T>int g(){struct Table{int(*f)();};static Table t={&f};return t.f();}int main(){return g<int>()!=7||g<char>()!=7;}''',
}
runner.GOOD.update({
 'local_table_overloaded_address': 'int f(){return 7;}int f(int n){return n;}template<class T>int g(){struct Table{int(*p)();};static const Table t={&f};return t.p();}int main(){return g<int>()!=7;}',
 'local_table_template_address': 'template<class T>int f(){return sizeof(T);}template<class T>int g(){struct Table{int(*p)();};static const Table t={&f<T>};return t.p();}int main(){return g<char>()!=1||g<long>()!=8;}',
 'address_decltype': 'int f(){return 7;}template<class T>struct X{using F=decltype(&f);static F p;};template<class T>typename X<T>::F X<T>::p=&f;int main(){return X<int>::p()!=7;}',
 'nested_static_decltype': 'template<class T,int N>struct X{template<class D>struct Y{static constexpr decltype(D::n) n=D::n;};};template<class T,int N>template<class D>constexpr decltype(D::n) X<T,N>::Y<D>::n;struct D{static constexpr int n=7;};int main(){return X<char,1>::Y<D>::n!=7;}',
 'nested_static_renamed_heads': 'template<class T,int N>struct X{template<class D>struct Y{static D n;};};template<class A,int M>template<class B>B X<A,M>::Y<B>::n=7;int main(){return X<char,1>::Y<int>::n!=7;}',
 'constexpr_static_redeclaration': 'struct X{static constexpr int n=3;};constexpr int X::n;int main(){return X::n!=3;}',
 'static_dependent_array': 'template<class T>struct X{static T a[2];};template<class U>U X<U>::a[2]={3,7};int main(){return X<int>::a[0]!=3||X<int>::a[1]!=7;}',
})
runner.BAD = {
 'demanded_invalid_initializer': '''template<class T>struct X{static int n;};template<class T>int X<T>::n=T::missing;int main(){return X<int>::n;}''',
 'duplicate_static_definition': '''struct X{static int n;};int X::n;int X::n;int main(){return 0;}''',
 'duplicate_template_static_definition': '''template<class T>struct X{static int n;};template<class T>int X<T>::n;template<class T>int X<T>::n;int main(){return X<int>::n;}''',
 'duplicate_unused_static_definition': 'template<class T>struct X{static int n;};template<class T>int X<T>::n;template<class T>int X<T>::n;int main(){}',
 'mismatched_static_definition': 'template<class T>struct X{static int n;};template<class T>long X<T>::n;int main(){}',
 'mismatched_dependent_static_definition': 'template<class T>struct X{static T n;};template<class T>T* X<T>::n;int main(){}',
 'nonstatic_out_of_class_definition': 'template<class T>struct X{int n;};template<class T>int X<T>::n;int main(){}',
 'deleted_address_in_table': 'int f()=delete;template<class T>int g(){struct Table{int(*p)();};static Table t={&f};return t.p();}int main(){return g<int>();}',
 'duplicate_specialized_static_definition': '''template<class T>struct X{static int n;};template<>int X<int>::n=3;template<>int X<int>::n=7;int main(){return 0;}''',
 'nonconstant_before_definition': '''struct X{static const int n;};static_assert(X::n==7,"not initialized yet");const int X::n=7;int main(){return 0;}''',
}
if __name__=='__main__':sys.exit(0 if runner.run(Path(sys.argv[1]).resolve(),Path(sys.argv[2])) else 1)
