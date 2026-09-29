#!/usr/bin/env python3
"""Independent PA19 cross-handoff controls. Run CC WORK; execute successes."""
from pathlib import Path
import sys
sys.path.insert(0, str(Path(__file__).resolve().parents[1] / 'pa18'))
import ordering_controls as runner

runner.GOOD = {
    'alias_conversion_value': '''
template<class T>using Alias=T;
template<int N>struct Box{int n;constexpr Box(int x):n(x){}};
template<class T>struct Outer{
 template<class U>using Rebind=Alias<U>;
 template<int N>static constexpr Rebind<Box<N>> value=Box<N>(sizeof(T)+N);
 template<int N>static int get(){return value<N>.n;}
};
int main(){return Outer<int>::get<3>()!=7||Outer<char>::get<3>()!=4;}
''',
    'alias_template_argument_and_pack': '''
template<class T,class U=int,class V=long>struct Tuple{};
template<template<class...>class F>struct Apply{typedef F<char> type;};
template<class T>struct Outer{template<class U>using Rebind=Tuple<T,U>;
 typedef typename Apply<Rebind>::type type;};
template<class A,class...B>int count(Tuple<A,B...>){return sizeof...(B);}
int main(){Outer<int>::type a;Outer<char>::type b;return count(a)!=2||count(b)!=2;}
''',
    'alias_scalar_call_query': '''
template<class T>using A=T;
template<class T>auto f(T v)->decltype(A<T>(v)){return A<T>(v);}
int main(){return f(7)!=7||f(9L)!=9;}
''',
    'selection_before_default_effect': '''
int effects;int tick(){return ++effects;}
template<class T>int f(T,int=T::missing){return 1;}
template<class T,class...A>int f(T*,A...){return 2;}
template<class T>int g(T,int=tick()){return 3;}
int main(){int n;return f(&n)!=2||effects||g(n)!=3||g(n)!=3||effects!=2;}
''',
    'constructor_selection_before_default': '''
struct X{int n;template<class T>X(T,int=T::missing):n(1){}
 template<class T,class...A>X(T*,A...):n(2){}};
int main(){int n;X x(&n);return x.n!=2;}
''',
    'constant_conversion_receiver_effect': '''
int effects;template<int N>struct X{static const int value=N;
 X(){++effects;}~X(){effects+=2;}operator int()const{return value;}};
template<int N>int f(){return X<N>();}
int main(){int n=f<7>();return n!=7||effects!=3;}
''',
    'constant_conversion_volatile_fallback': '''
volatile int value=7;template<class T>struct X{operator T()const{return value;}};
int main(){X<int>x;int a=x;value=9;int b=x;return a!=7||b!=9;}
''',
    'constant_conversion_effectful_fallback': '''
int effects;template<int N>struct X{static const int value=N;
 operator int()const{++effects;return value;}};
int main(){X<7>x;int a=x;int b=x;return a!=7||b!=7||effects!=2;}
''',
    'alias_reference_cast_identity': '''
int conversions;template<class T>using Ref=T&&;
struct X{int n;template<class T>operator T(){++conversions;return T();}};
template<class T>Ref<T> move(T&x){return static_cast<Ref<T>>(x);}
int main(){X x;x.n=3;auto&&y=move(x);y.n=9;return &y!=&x||x.n!=9||conversions;}
''',
    'adl_pack_explicit_value': '''
namespace N{struct X{};template<int K,class...A>int f(X,A...){return K+sizeof...(A);}}
template<class T,class...A>int g(T x,A...a){return f<3>(x,a...);}
int main(){return g(N::X())!=3||g(N::X(),1,2L)!=5;}
''',
    'dependent_sizeof_signed_conversion': '''
template<class T,int N>unsigned long f(){return N+sizeof(T);}
template<class T,int N>unsigned long g(){return sizeof(T)+N;}
int main(){return f<char,-3>()!=static_cast<unsigned long>(-2)||
 g<long,-3>()!=5||f<int,3>()!=7;}
''',
    'variable_unevaluated_then_address': '''
struct X{int n;constexpr X(int v):n(v){}};
template<int N>struct O{template<int M>static constexpr X v=X(N+M);
 template<class U>static constexpr int dormant=U::missing;};
template<class T>using V=decltype(T::template v<4>);
V<O<3>> x(7);
int main(){const X*a=&O<3>::v<4>;const X*b=&O<3>::v<4>;
 return a!=b||a->n!=x.n||sizeof(O<3>::dormant<int>)!=sizeof(int);}
''',
}
runner.BAD = {
    'selected_default_invalid': 'template<class T>int f(T,int=T::missing){return 1;}int main(){return f(1);}',
    'dependent_body_failure_is_hard': 'template<class T>int f(T){return T::missing;}int f(...){return 2;}int main(){return f(1);}',
    'alias_cast_const_rejection': 'template<class T>using Ref=T&&;struct X{};int main(){const X x{};Ref<X> y=static_cast<Ref<X>>(x);}',
    'variable_constructor_access': 'struct X{int n;private:constexpr X(int v):n(v){}};template<class T>struct O{template<int N>static constexpr X v=X(N);};int main(){return O<int>::v<3>.n;}',
}
if __name__ == '__main__':
    sys.exit(0 if runner.run(Path(sys.argv[1]).resolve(), Path(sys.argv[2])) else 1)
