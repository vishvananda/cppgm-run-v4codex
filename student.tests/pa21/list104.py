#!/usr/bin/env python3
"""Explicit PA21 initializer-list semantic, lifetime and rejection controls."""
from pathlib import Path
import sys
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'pa18'))
import ordering_controls as runner
LIB='namespace std{template<class T>class initializer_list{const T*p;unsigned long n;public:constexpr initializer_list():p(0),n(0){}constexpr const T*begin()const{return p;}constexpr const T*end()const{return p+n;}constexpr unsigned long size()const{return n;}};}'
FORWARD='namespace std{template<class T>class initializer_list;}'
SAME='template<class A,class B>struct same{static const bool value=false;};template<class A>struct same<A,A>{static const bool value=true;};'
OBJ='int live,sequence;struct S{int n;S(int v):n(v){++live;}S(const S&s):n(s.n){++live;}~S(){--live;sequence=sequence*10+n;}};'
GOOD={
 'scalar_call':'int f(std::initializer_list<int>x){int s=0;for(auto n:x)s+=n;return s;}int main(){return f({2,3,4})!=9;}',
 'scalar_reference':'int f(const std::initializer_list<int>&x){return x.size();}int main(){return f({2,3})!=2;}',
 'empty':'int f(std::initializer_list<int>x){return x.size();}int main(){std::initializer_list<int>x{};return f({})||x.size();}',
 'auto':'int main(){auto x={2,3,4};return x.size()!=3||x.begin()[1]!=3;}',
 'auto_reference':'int main(){const auto&x={2,3,4};return x.size()!=3||x.begin()[2]!=4;}',
 'auto_rvalue_reference':'int main(){auto&&x={2,3,4};return x.size()!=3;}',
 'two_phase':'struct A{int n;A(int,int):n(9){}A(std::initializer_list<int>x):n(x.size()){}};int main(){A a{2,3};return a.n!=2;}',
 'phase_two':'struct A{int n;A(int,int):n(9){}A(std::initializer_list<const char*>):n(2){}};int main(){A a{2,3};return a.n!=9;}',
 'empty_default':'struct A{int n;A():n(9){}A(std::initializer_list<int>):n(2){}};int main(){A a{};return a.n!=9;}',
 'empty_list_only':'struct A{int n;A(std::initializer_list<int>x):n(x.size()){}};int main(){A a{};return a.n;}',
 'rank':'int f(std::initializer_list<int>){return 1;}int f(std::initializer_list<long>){return 2;}int main(){return f({1})!=1||f({1L})!=2;}',
 'rank_pointer':'int f(const char*){return 1;}int f(std::initializer_list<char>){return 2;}int main(){return f({\'a\'})!=2;}',
 'template_deduction':'template<class T>int f(std::initializer_list<T>x){return sizeof(T)+x.size();}int main(){return f({1L,2L})!=10;}',
 'template_reference':'template<class T>int f(const std::initializer_list<T>&x){return sizeof(T)+x.size();}int main(){return f({1,2,3})!=7;}',
 'template_empty_explicit':'template<class T>int f(std::initializer_list<T>x){return x.size();}int main(){return f<int>({});}',
 'template_fixed_body':'template<class T>int f(T){std::initializer_list<int>x{1,2};return x.size();}int main(){return f(1)!=2||f(1L)!=2;}',
 'template_dependent_body':'template<class T>int f(T n){std::initializer_list<T>x{n,n};return x.size();}int main(){return f(1)!=2||f(1L)!=2;}',
 'pack':'template<class...T>int f(T...n){auto x={n...};return x.size();}int main(){return f(1,2,3)!=3;}',
 'nested':'int main(){std::initializer_list<std::initializer_list<int>>x{{1,2},{3,4}};return x.size()!=2||x.begin()[1].begin()[0]!=3;}',
 'class_call':OBJ+'int f(std::initializer_list<S>x){return live+x.size();}int main(){if(f({1,2})!=4)return 1;return live||sequence!=21;}',
 'class_local':OBJ+'int main(){{std::initializer_list<S>x{1,2};if(live!=2)return 1;}return live||sequence!=21;}',
 'class_reference':OBJ+'int main(){{const std::initializer_list<S>&x={1,2};if(live!=2)return 1;}return live||sequence!=21;}',
 'class_copy':OBJ+'int main(){{std::initializer_list<S>x{1,2};{auto y=x;if(live!=2||y.size()!=2)return 1;}if(live!=2)return 2;}return live||sequence!=21;}',
 'class_global':OBJ+'std::initializer_list<S>x{1,2};int main(){return live!=2||x.begin()[1].n!=2;}',
 'class_local_static':OBJ+'int f(){static std::initializer_list<S>x{1,2};return x.size();}int main(){return f()!=2||f()!=2||live!=2;}',
 'scalar_global':'std::initializer_list<int>x{2,3};int main(){return x.begin()[1]!=3;}',
 'alias':'template<class T>using list=std::initializer_list<T>;int main(){list<int>x{1,2};return x.size()!=2;}',
 'capture':'int main(){auto x={1,2};auto f=[x](){int n=0;for(auto v:x)n+=v;return n;};return f()!=3;}',
 'const_elements':SAME+'int main(){std::initializer_list<int>x{1};static_assert(same<decltype(*x.begin()),const int&>::value,"const");return 0;}',
 'query_constructor':SAME+'struct A{A(std::initializer_list<int>);};static_assert(same<decltype(A{1,2}),A>::value,"query");int main(){}',
 'scope_name':'namespace other{template<class T>struct initializer_list{T n;};}int main(){other::initializer_list<int>x{3};return x.n!=3;}',
}
BAD={
 'auto_mismatch':'int main(){auto x={1,2L};}',
 'auto_empty':'int main(){auto x={};}',
 'narrowing':'int main(){std::initializer_list<int>x{1.5};}',
 'narrowing_selected':'struct A{A(int,int);A(std::initializer_list<char>);};int main(){A x{1000,2000};}',
 'explicit_copy':'struct A{explicit A(std::initializer_list<int>);};int main(){A x={1,2};}',
 'deleted_selected':'struct A{A(std::initializer_list<int>)=delete;A(int,int);};int main(){A x{1,2};}',
 'private_selected':'class A{A(std::initializer_list<int>);public:A(int,int);};int main(){A x{1,2};}',
 'nonconst_reference':'void f(std::initializer_list<int>&);int main(){f({1,2});}',
 'ambiguous':'int f(std::initializer_list<long>);int f(std::initializer_list<unsigned>);int main(){return f({1});}',
 'deduction_mismatch':'template<class T>void f(std::initializer_list<T>);int main(){f({1,2L});}',
 'deduction_empty':'template<class T>void f(std::initializer_list<T>);int main(){f({});}',
 'nonlist_deduction':'template<class T>void f(T);int main(){f({1,2});}',
 'const_element_write':'int main(){std::initializer_list<int>x{1};*x.begin()=2;}',
}
runner.GOOD={n:LIB+s for n,s in GOOD.items()}
runner.GOOD.update({
 'constexpr_global':LIB+'constexpr std::initializer_list<int>x{2,3};static_assert(x.size()==2,"size");static_assert(x.begin()[1]==3,"element");int main(){return x.begin()[0]!=2;}',
 'constexpr_empty':LIB+'constexpr std::initializer_list<int>x{};static_assert(x.size()==0,"size");int main(){return x.size();}',
 'constexpr_call':LIB+'constexpr int f(std::initializer_list<int>x){return x.begin()[0]+x.size();}static_assert(f({3,4})==5,"list call");int main(){}',
 'nested_class':LIB+OBJ+'int main(){{std::initializer_list<std::initializer_list<S>>x{{1,2},{3,4}};if(live!=4||x.begin()[1].begin()[0].n!=3)return 1;}return live||sequence!=4321;}',
})
# The sectionless execution adapter has no atexit import. The host-linked
# lifetime harness below owns this unchanged runtime control.
runner.GOOD.pop('class_local_static')
runner.GOOD['forward_range']=FORWARD+GOOD['scalar_call']
runner.BAD={n:LIB+s for n,s in BAD.items()}
if __name__=='__main__':sys.exit(0 if runner.run(Path(sys.argv[1]).resolve(),Path(sys.argv[2])) else 1)
