#!/usr/bin/env python3
"""Closure environments: source declarations, nested forwarding and specializations."""
from pathlib import Path
import sys
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'pa18'))
import ordering_controls as runner
runner.GOOD={
 'reference':'int main(){int n=3;auto f=[&n](int x){n+=x;return n;};return f(4)!=7||n!=7;}',
 'default_reference':'int main(){int n=3,m=4;auto f=[&](){return n+m;};n=8;return f()!=12;}',
 'reference_alias':'int main(){int n=3;int&r=n;auto f=[&](){++r;};f();return n!=4;}',
 'array':'int main(){int a[2]={3,4};auto f=[&](){a[1]+=a[0];};f();return a[1]!=7;}',
 'class':'struct S{int n;};int main(){S s={3};auto f=[&](){s.n+=4;};f();return s.n!=7;}',
 'const_address':'int main(){const int n=3;auto f=[&]()->const int*{return &n;};return f()!=&n;}',
 'const_no_capture':'int main(){const int n=3;auto f=[](){return n;};return f()!=3;}',
 'this_field':'struct S{int n;int f(){auto g=[this](){return ++n;};return g();}};int main(){S s={3};return s.f()!=4||s.n!=4;}',
 'this_explicit':'struct S{int n;int f(){return [this](){return this->n;}();}};int main(){S s={3};return s.f()!=3;}',
 'this_default':'struct S{int n;int read()const{return n;}int f()const{auto g=[&](){return read();};return g();}};int main(){S s={3};return s.f()!=3;}',
 'this_equal_default':'struct S{int n;int f()const{return [=](){return n;}();}};int main(){S s={3};return s.f()!=3;}',
 'nested_reference':'int main(){int n=3;auto f=[&](){auto g=[&](){return ++n;};return g();};return f()!=4||n!=4;}',
 'nested_own_local':'int main(){auto f=[](){int n=3;auto g=[&](){return ++n;};return g();};return f()!=4;}',
 'nested_parameter':'int main(){auto f=[](int n){auto g=[&](){return ++n;};return g();};return f(3)!=4;}',
 'nested_this':'struct S{int n;int f(){auto a=[&](){auto b=[&](){return ++n;};return b();};return a();}};int main(){S s={3};return s.f()!=4||s.n!=4;}',
 'nested_this_and_local':'struct S{int n;int f(int x){auto a=[&,this](){int y=2;auto b=[&](){return n+x+y;};return b();};return a();}};int main(){S s={3};return s.f(4)!=9;}',
 'copy':'int main(){int n=3;auto a=[&](){return ++n;};auto b=a;auto c=static_cast<decltype(a)&&>(a);return b()!=4||c()!=5||a()!=6;}',
 'by_value_argument':'template<class F>int f(F g){return g();}int main(){int n=3;auto a=[&](){return ++n;};return f(a)!=4||n!=4;}',
 'immediate_argument':'template<class F>int f(F g){return g();}int main(){int n=3;return f([&](){return ++n;})!=4||n!=4;}',
 'function_specializations':'template<class T>int f(T x){int n=3;auto a=[&](){return n+x;};return a();}int main(){return f(4)!=7||f(5L)!=8;}',
 'member_specializations':'template<class T>struct S{T n;int f(){auto a=[&](){return n;};return a();}};int main(){S<int>s={3};S<long>t={4};return s.f()!=3||t.f()!=4;}',
 'static_no_capture':'int main(){static int n=3;auto a=[&](){return ++n;};return a()!=4;}',
 'enum_no_capture':'enum E{N=3};int main(){auto a=[&](){return N;};return a()!=3;}',
 'shadow':'int main(){int n=3;auto a=[&n](){int n=4;auto b=[&](){return n;};return b();};return a()!=4||n!=3;}',
 'repeated_evaluation':'int f(int n){auto a=[&](){return n;};return a();}int main(){return f(3)!=3||f(4)!=4;}',
 'friend':'class S{friend struct R;int n;public:S():n(3){}};struct R{int f(S&s){auto a=[&](){return s.n;};return a();}};int main(){S s;R r;return r.f(s)!=3;}',
}
runner.GOOD.update({
 'returned_closure':'auto make(int&n){return [&n](){return ++n;};}int main(){int n=3;auto f=make(n);return f()!=4||n!=4;}',
 'explicit_pack':'int sink(int&a,long&b){++a;++b;return a+b;}template<class...T>int f(T&...args){auto a=[&args...](){return sink(args...);};return a();}int main(){int n=3;long m=4;return f(n,m)!=9||n!=4||m!=5;}',
 'explicit_empty_pack':'template<class...T>int f(T...args){auto a=[&args...](){return sizeof...(args);};return a();}int main(){return f();}',
 'explicit_nested_pack':'int sink(int&a,long&b){++a;++b;return a+b;}template<class...T>int f(T&...args){auto a=[&args...](){auto b=[&args...](){return sink(args...);};return b();};return a();}int main(){int n=3;long m=4;return f(n,m)!=9||n!=4||m!=5;}',

 'template_constructor_closure_type':'template<class T>struct pointer{static const bool value=false;};template<class T>struct pointer<T*>{static const bool value=true;};struct S{int n;template<class F>S(F f):n(f(6)){static_assert(!pointer<F>::value,"closure must remain a class");}};int main(){S s=[](int x){return x+1;};return s.n!=7;}',
 'wrapper_direct':'struct W{int(*p)(int);W(int(*p)(int)):p(p){}};int main(){W w([](int n){return n+1;});return w.p(3)!=4;}',
 'wrapper_explicit_pointer':'struct W{int(*p)(int);W(int(*p)(int)):p(p){}};int f(const W&w){return w.p(3);}int main(){return f(+[](int n){return n+1;})!=4;}',
 'captured_constructor':'struct S{int n;template<class F>S(F f):n(f()){}};int main(){int n=3;S s=[&](){return n;};return s.n!=3;}',

 'fixed_member_specializations':'template<class T>struct S{int n;int f(){auto a=[&](){return n;};return a();}};int main(){S<int>s={3};S<long>t={4};return s.f()!=3||t.f()!=4;}',
 'nested_fixed_member':'template<class T>struct S{int n;int read()const{return n;}int f()const{auto a=[&](){auto b=[&](){return read();};return b();};return a();}};int main(){S<int>s={3};return s.f()!=3;}',
 'nested_const_address':'int main(){const int n=3;auto a=[&](){auto b=[&]()->const int*{return &n;};return b();};return a()!=&n;}',
 'late_outer_capture':'int main(){int a=1,b=2,c=3;auto f=[&](){auto g=[&](){return b;};return a+g()+c;};return f()!=6;}',
 'outer_reference_parameter':'template<class T>int f(T&n){auto a=[&](){auto b=[&](){return ++n;};return b();};return a();}int main(){int n=3;return f(n)!=4||n!=4;}',
 'empty_default_parameter':'template<class F>int f(F g){return g();}int main(){return f([&](){return 3;})!=3;}',
 'static_member_in_default':'struct S{static int read(){return 3;}int f(){auto a=[&](){return read();};return a();}};int main(){S s;return s.f()!=3;}',
 'this_two_objects':'struct S{int n;int f(){auto a=[this](){return n;};return a();}};int main(){S s={3},t={4};return s.f()!=3||t.f()!=4;}',
})
runner.BAD={
 'namespace_capture_default':'auto f=[&](){return 3;};int main(){return f();}',
 'value_default_explicit_this':'struct S{int n;int f(){return [=,this](){return n;}();}};',

 'redundant_reference':'int main(){int n;auto a=[&,&n](){};}',
 'capture_parameter':'int main(){int n;auto a=[&n](int n){return n;};}',
 'unexpanded_capture_pack':'template<class...T>void f(T...n){auto a=[&n](){};}int main(){f(3);}',
 'nonpack_expansion':'int main(){int n;auto a=[&n...](){};}',

 'wrapper_two_conversions':'struct W{W(int(*)());};void f(const W&);int main(){f([](){return 1;});}',

 'missing_local':'int main(){int n=3;auto a=[](){return n;};}',
 'missing_this':'struct S{int n;int f(){return [](){return n;}();}};',
 'missing_nested':'int main(){int n=3;auto a=[](){auto b=[&](){return n;};return b();};}',
 'capturing_pointer':'int main(){int n=3;int(*p)()=[&](){return n;};}',
 'default_capture_pointer':'int main(){int(*p)()=[&](){return 3;};}',
 'default_construct':'int main(){int n=3;auto a=[&](){return n;};decltype(a) b;}',
 'assignment':'int main(){int n=3;auto a=[&](){return n;};a=a;}',
 'this_nonmember':'int main(){auto a=[this](){};}',
 'duplicate':'int main(){int n;auto a=[&n,&n](){};}',
}
if __name__=='__main__':sys.exit(0 if runner.run(Path(sys.argv[1]).resolve(),Path(sys.argv[2])) else 1)
