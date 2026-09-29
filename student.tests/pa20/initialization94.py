#!/usr/bin/env python3
"""Array-bound and conversion composition controls. Run CC WORK."""
from pathlib import Path
import sys
sys.path.insert(0, str(Path(__file__).resolve().parents[1] / 'pa18'))
import ordering_controls as runner

runner.GOOD = {
 'array_boolean': 'using A=bool[];int main(){return A{true,false}[1]||!A{true}[0];}',
 'array_size': 'using A=int[];static_assert(sizeof(A{1,2,3})==3*sizeof(int),"");int main(){return A{2,4,6}[2]!=6;}',
 'array_string': 'using A=char[];static_assert(sizeof(A{"abc"})==4,"");int main(){return A{"abc"}[2]!=\'c\'||A{"abc"}[3]!=0;}',
 'array_effects': 'int calls;int f(){return ++calls;}using A=int[];int main(){int n=A{f(),f(),f()}[1];return calls!=3||n!=2;}',
 'array_pack': 'template<class...T>int f(T...n){int sum=0;using A=int[];(void)A{0,(sum+=n,0)...};return sum;}int main(){return f(1,2,3)!=6||f()!=0;}',
 'array_nested': 'using A=int[][2];int main(){return A{{1,2},{3}}[1][0]!=3||A{{1,2},{3}}[1][1]!=0;}',
 'array_elision': 'using A=int[][2];int main(){return A{1,2,3,4}[1][1]!=4;}',
 'array_aggregate': 'struct E{int a;int b;};using A=E[];int main(){return A{{1,2},{3}}[1].a!=3||A{{1,2},{3}}[1].b!=0;}',
 'array_empty_class_elements': 'struct E{};using A=E[];static_assert(sizeof(A{{},{}})==2,"");int main(){}',
 'aggregate_parentheses': 'struct P{int a;int b;};P f(int a,int b){return P(a,b);}int main(){return f(3,4).b!=4;}',
 'aggregate_parentheses_omitted': 'struct P{int a;int b;};P f(int a){return P(a);}int main(){return f(3).a!=3||f(3).b!=0;}',
 'aggregate_template_parentheses': 'template<class T>struct P{T a;T b;};template<class T>P<T>f(T a){return P<T>(a);}int main(){return f(3L).a!=3||f(3L).b!=0;}',
 'increment_unsigned': 'int calls;struct C{unsigned n;operator unsigned&(){++calls;return n;}};int main(){C c={3};unsigned before=c++;unsigned&after=++c;return before!=3||&after!=&c.n||after!=5||calls!=2;}',
 'increment_volatile': 'int calls;struct C{volatile short n;operator volatile short&(){++calls;return n;}};int main(){C c={3};short before=c++;volatile short&after=--c;return before!=3||&after!=&c.n||after!=3||calls!=2;}',
 'increment_selection': 'struct C{int a;int b;operator int&(){return a;}operator const int&()const{return b;}};int main(){C c={3,7};++c;return c.a!=4||c.b!=7;}',
 'increment_pointer': 'int a[3]={1,2,3};struct C{int*p;operator int*&(){return p;}};int main(){C c={a};int*p=c++;--c;return p!=a||c.p!=a;}',
 'increment_float': 'struct C{double n;operator double&(){return n;}};int main(){C c={1.5};double before=c++;return before!=1.5||c.n!=2.5;}',
 'increment_bool': 'struct C{bool n;operator bool&(){return n;}};int main(){C c={false};bool before=c++;return before||!c.n;}',
 'increment_template': 'template<class T>auto f(T&v){return v++;}struct C{long n;operator long&(){return n;}};int main(){C c={3};return f(c)!=3||c.n!=4||sizeof(f(c))!=sizeof(long);}',
 'increment_user_preferred': 'struct C{int n;operator int&(){return n;}long operator++(){n+=3;return n;}};int main(){C c={1};return ++c!=4||c.n!=4;}',
 'conversion_using_alias': 'struct B{operator long()const{return 7;}};struct D:B{using value=long;using B::operator value;};int main(){D d;return static_cast<long>(d)!=7;}',
 'conversion_using_template_alias': 'template<class T>struct B{operator T()const{return 7;}};template<class T>struct D:B<T>{using value=T;using B<T>::operator value;};int main(){D<long>d;return static_cast<long>(d)!=7;}',
 'conversion_using_fixed_base': 'struct B{operator long()const{return 7;}};template<class T>struct D:B{using value=long;using B::operator value;};int main(){D<int>d;return static_cast<long>(d)!=7;}',
 'aggregate_copy_control': 'struct P{int n;};P f(P p){return P(p);}int main(){P p={7};return f(p).n!=7;}',
 'aggregate_derived_copy_control': 'struct B{int n;};struct D:B{};B f(D p){return B(p);}int main(){D d;d.n=7;return f(d).n!=7;}',
 'aggregate_distinct_prefixes': 'struct P{int a;int b;int c;};P a(int v){return P{v};}P b(int v){return P{v,v};}P c(int v){return P{v,v,v};}int main(){return a(7).b||a(7).c||b(8).b!=8||b(8).c||c(9).c!=9;}',
}
runner.BAD = {
 'array_empty_unknown': 'using A=int[];int main(){(void)A{};}',
 'array_narrow': 'using A=int[];int main(){(void)A{1.5};}',
 'array_empty_elision': 'struct E{};using A=E[];int main(){(void)A{1};}',
 'increment_const_reference': 'struct C{operator const int&()const;};int main(){C c;++c;}',
 'increment_value': 'struct C{operator int();};int main(){C c;++c;}',
 'decrement_bool': 'struct C{operator bool&();};int main(){C c;--c;}',
 'increment_ambiguous': 'struct C{operator int&();operator long&();};int main(){C c;++c;}',
 'increment_explicit': 'struct C{explicit operator int&();};int main(){C c;++c;}',
}

if __name__ == '__main__':
    sys.exit(0 if runner.run(Path(sys.argv[1]).resolve(),Path(sys.argv[2])) else 1)
