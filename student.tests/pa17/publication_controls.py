#!/usr/bin/env python3
"""Class member publication, constructor roles, and explicit-condition controls."""
from pathlib import Path
import sys
import entity_controls as runner

runner.GOOD = {
    'anonymous_nested_storage': '''template<class T>struct S{char before;struct{T x;struct{int y;};};char after;};int main(){S<long>s={};s.before=1;s.x=7;s.y=9;s.after=2;return s.before+s.x+s.y+s.after-19;}''',
    'anonymous_union_struct': '''template<class T>struct S{union{struct{T x;int y;};long a;};};int main(){S<int>s={};s.x=3;s.y=8;return s.x+s.y-11;}''',
    'anonymous_alignment': '''template<class T>const unsigned long alignment=alignof(T);template<class T>struct S{struct{alignas(alignment<T>)char a;};char b;};static_assert(alignof(S<long>)==8 && sizeof(S<long>)==16,"layout");int main(){S<long>s={};s.a=4;s.b=8;return s.a+s.b-12;}''',
    'named_anonymous_type_not_injected': '''template<class T>struct S{struct{T value;}field;T value;};int main(){S<int>s={};s.field.value=3;s.value=6;return s.field.value+s.value-9;}''',
    'anonymous_cv_access': '''template<class T>struct S{struct{T value;};int get()const{return value;}};int main(){S<int>s={};s.value=11;return s.get()-11;}''',
    'constructor_conversion_declarations': '''struct X{template<class T>X(T);template<class T>operator T()const;};template<class U>X::X(U){} template<class U>X::operator U()const{return U();}int main(){X x(2);return 0;}''',
    'constructor_conversion_qualified_target': '''namespace N{struct X{using I=int;template<class T>X(T);template<class T>operator T*()const;};template<class U>X::X(U){} template<class U>X::operator U*()const{return 0;}}int main(){N::X x(2);return 0;}''',
    'constructor_template_body_and_conversion_name': '''struct X{int n;template<class T>X(T x);template<class T>operator T()const;};template<class U>X::X(U x):n(x){} template<class U>X::operator U()const{return U(n);}int main(){X x(17);return x.n-17;}''',
    'conditional_false_copy': '''struct X{int n;template<class T>explicit(false)X(T x):n(x){}};int f(X x){return x.n;}int main(){return f(7)-7;}''',
    'conditional_true_direct': '''struct X{int n;template<class T>explicit(true)X(T x):n(x){}};int main(){X x(7);return x.n-7;}''',
    'conditional_parameter_copy': '''struct X{int n;template<class T>explicit(sizeof(T)>4)X(T x):n(x){}};int f(X x){return x.n;}int main(){X x(7L);return f(3)+x.n-10;}''',
    'conditional_outer_parameter': '''template<class T>struct X{int n;template<class U>explicit(sizeof(T)>4)X(U x):n(x){}};int f(X<int>x){return x.n;}int main(){X<long>x(7);return f(3)+x.n-10;}''',
    'conditional_fixed_non_template': '''struct X{int n;explicit(false)X(int x):n(x){}};int f(X x){return x.n;}int main(){return f(8)-8;}''',
    'conditional_default_constructor': '''template<int N>struct X{int n;template<class T=int>explicit(sizeof(T)==N)X():n(N){}};int main(){X<4>a;X<8>b;return a.n+b.n-12;}''',
    'conditional_renamed_definition': '''struct X{int n;template<class T>explicit(sizeof(T)>4)X(T);};template<class U>X::X(U x):n(x){} int f(X x){return x.n;}int main(){X x(9L);return f(3)+x.n-12;}''',
    'conditional_constexpr_function': '''constexpr bool choose(int n){return n>4;}struct X{int n;template<class T>explicit(choose(sizeof(T)))X(T x):n(x){}};int f(X x){return x.n;}int main(){X x(4L);return f(7)+x.n-11;}''',
    'conditional_contextual_bool': '''struct Flag{bool b;constexpr explicit operator bool()const{return b;}};constexpr Flag flag={false};struct X{int n;template<class T>explicit(flag)X(T x):n(x){}};int f(X x){return x.n;}int main(){return f(7)-7;}''',
    'conditional_nullptr': '''struct X{int n;template<class T>explicit(nullptr)X(T x):n(x){}};int f(X x){return x.n;}int main(){return f(7)-7;}''',
    'conditional_short_circuit': '''template<class T>struct X{int n;template<class U>explicit(false && (1/0))X(U x):n(x){}};int f(X<int>x){return x.n;}int main(){return f(7)-7;}''',
    'conditional_template_template': '''template<class T>struct Test{static const bool value=sizeof(T)>4;};template<template<class>class F,class T>struct Apply{static const bool value=F<T>::value;};struct X{int n;template<class T>explicit(Apply<Test,T>::value)X(T x):n(x){}};int f(X x){return x.n;}int main(){X x(5L);return f(6)+x.n-11;}''',
}
runner.BAD = {
    'conditional_true_copy': '''struct X{template<class T>explicit(true)X(T){}};void f(X);int main(){f(2);}''',
    'conditional_parameter_copy_reject': '''struct X{template<class T>explicit(sizeof(T)>4)X(T){}};void f(X);int main(){f(2L);}''',
    'conditional_outer_copy_reject': '''template<class T>struct X{template<class U>explicit(sizeof(T)>4)X(U){}};void f(X<long>);int main(){f(2);}''',
    'conditional_renamed_copy_reject': '''struct X{template<class T>explicit(sizeof(T)>4)X(T);};template<class U>X::X(U){} void f(X);int main(){f(2L);}''',
    'conditional_nonconstant': '''int b;struct X{template<class T>explicit(b)X(T){}};int main(){X x(2);}''',
    'conditional_scoped_enum': '''enum class Flag{off};struct X{template<class T>explicit(Flag::off)X(T){}};int main(){X x(2);}''',
    'anonymous_const_write': '''template<class T>struct S{struct{T x;};};int main(){const S<int>s={};s.x=2;}''',
    'anonymous_private': '''template<class T>class S{struct{T x;};};int main(){S<int>s;s.x=2;}''',
}
runner.GOOD.update({
    'anonymous_fixed_field_body': '''template<class T>struct S{struct{int x;struct{int y;};};int sum()const{return x+y;}};int main(){S<char>s={};s.x=3;s.y=4;return s.sum()-7;}''',
    'anonymous_hides_base': '''struct Base{int x;};template<class T>struct S:Base{struct{struct{T x;};};};int main(){S<int>s;s.x=3;s.Base::x=7;return s.x+s.Base::x-10;}''',
    'anonymous_protected': '''template<class T>struct Base{protected:struct{T x;};};struct S:Base<int>{int run(){x=7;return x;}};int main(){S s;return s.run()-7;}''',
    'array_functional_pack': '''int count;struct F{template<class T>void operator()(T){count+=sizeof(T);}};template<class...T>void run(){using A=int[sizeof...(T)];(void)A{(F()(T()),0)...};}int main(){run<char,short,int>();return count-7;}''',
    'array_functional_fixed': '''int count;int f(){return ++count;}template<class T>void run(){using A=int[3];(void)A{f(),f(),f()};}int main(){run<char>();return count-3;}''',
    'array_functional_values': '''using A=int[3];int total(int*p){return p[0]+p[1]+p[2];}int main(){return total(A{2,3,5})-10;}''',
    'array_functional_zero_tail': '''using A=int[3];int total(int*p){return p[0]+p[1]+p[2];}int main(){return total(A{7})-7;}''',
    'array_functional_class_cleanup': '''int count;struct X{X(){++count;}~X(){--count;}};using A=X[3];int main(){(void)A{};return count;}''',
})
runner.BAD.update({
    'array_functional_excess': 'using A=int[2];int main(){(void)A{1,2,3};}',
    'array_functional_narrow': 'using A=char[2];int main(){(void)A{1,300};}',
    'anonymous_protected_reject': 'template<class T>struct S{protected:struct{T x;};};int main(){S<int>s;s.x=2;}',
})
runner.GOOD.update({
    'conditional_nested_renamed_definition': '''template<class T>struct X{int n;template<class U>explicit(sizeof(T)+sizeof(U)>8)X(U);};template<class A>template<class B>X<A>::X(B x):n(x){} int f(X<int>x){return x.n;}int main(){X<long>x(5L);return f(6)+x.n-11;}''',
    'conditional_pack': '''struct X{int n;template<class...T>explicit(sizeof...(T)>1)X(T...):n(sizeof...(T)){}};int f(X x){return x.n;}int main(){X x(1,2);return f(3)+x.n-3;}''',
    'conditional_conversion_function': '''template<bool B>struct X{explicit(B)constexpr operator int()const{return 7;}};int main(){X<false>a;X<true>b;int n=a;return n+static_cast<int>(b)-14;}''',
    'anonymous_inherited_offsets': '''struct P{long padding;};template<class T>struct S{char first;struct{struct{T x;};};};struct D:P,S<long>{int f(){x=9;return x;}};int main(){D d;d.first=3;return d.f()+d.first-12;}''',
    'anonymous_volatile': '''template<class T>struct S{struct{volatile T x;};};int main(){S<int>s={};s.x=7;return s.x-7;}''',
    'union_large_zero_fallback': '''struct R{long x[20];};union U{R r;long a;};int main(){U u=U();return u.r.x[0]+u.r.x[19];}''',
})
runner.BAD.update({
    'conditional_conversion_reject': '''template<bool B>struct X{explicit(B)constexpr operator int()const{return 7;}};int main(){X<true>a;int n=a;}''',
    'anonymous_duplicate_lookup': '''template<class T>struct S{struct{T x;};struct{int x;};};int main(){S<int>s;return s.x;}''',
})
if __name__ == '__main__':
    sys.exit(0 if runner.run(Path(sys.argv[1]).resolve(), Path(sys.argv[2])) else 1)
