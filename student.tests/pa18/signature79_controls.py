#!/usr/bin/env python3
"""First declaration signature lookup and later body lookup: CC WORK."""
from pathlib import Path
import sys
import ordering_controls as runner
runner.GOOD={
 'dependent_first':'long g(double);template<class T>auto h(T)->decltype(g(T()));int g(int);template<class U>auto h(U)->decltype(g(U()));static_assert(sizeof(h(0))==sizeof(long),"");int main(){}',
 'dependent_definition':'long g(double){return 7;}template<class T>auto h(T)->decltype(g(T()));int g(int){return 3;}template<class U>auto h(U)->decltype(g(U())){return g(U());}static_assert(sizeof(h(0))==sizeof(long),"");int main(){return h(0)!=3;}',
 'dependent_three':'long g(double){return 7;}template<class T>auto h(T)->decltype(g(T()));int g(int){return 3;}template<class U>auto h(U)->decltype(g(U()));template<class V>auto h(V)->decltype(g(V())){return g(V());}static_assert(sizeof(h(0))==sizeof(long),"");int main(){return h(0)!=3;}',
 'dependent_parameter':'long g(double);template<class T>int h(T,decltype(g(T())) p);int g(int);template<class U>int h(U,decltype(g(U())) p){static_assert(sizeof(p)==sizeof(long),"");return p;}int main(){return h(0,7)!=7;}',
 'dependent_const_parameter':'long g(double);template<class T>int h(T,const decltype(g(T())) p);int g(int);template<class U>int h(U,const decltype(g(U())) p){static_assert(sizeof(p)==sizeof(long),"");return p;}int main(){return h(0,7)!=7;}',
 'dependent_default':'long g(double);template<class T>auto h(T,int p=7)->decltype(g(T()));int g(int);template<class U>auto h(U,int p)->decltype(g(U())){return p;}static_assert(sizeof(h(0))==sizeof(long),"");int main(){return h(0)!=7;}',
 'dependent_constexpr':'constexpr long g(double){return 7;}template<class T>constexpr auto h(T)->decltype(g(T()));constexpr int g(int){return 3;}template<class U>constexpr auto h(U)->decltype(g(U())){return g(U());}static_assert(sizeof(h(0))==sizeof(long)&&h(0)==3,"");int main(){}',
 'dependent_address':'long g(double);template<class T>auto h(T)->decltype(g(T()));int g(int);template<class U>auto h(U x)->decltype(g(U())){return x;}int main(){long(*p)(int)=h;return p(7)!=7;}',
 'dependent_adl':'template<class T>auto h(T t)->decltype(g(t));int g(int);template<class U>auto h(U t)->decltype(g(t)){return g(t);}namespace N{struct X{};int g(X){return 7;}}int main(){return h(N::X())!=7;}',
 'distinct_fixed_results':'long g(double);template<class T>auto h(T x)->decltype(g(0)){return x;}int g(int);template<class T>auto h(T x)->decltype(g(0)){return x+1;}int main(){long(*a)(int)=h;int(*b)(int)=h;return a(7)!=7||b(7)!=8;}',
 'dependent_member':'long g(double);struct M{template<class T>auto h(T)->decltype(g(T()));};int g(int);template<class U>auto M::h(U x)->decltype(g(U())){return x;}int main(){M m;static_assert(sizeof(m.h(0))==sizeof(long),"");return m.h(7)!=7;}',
 'dependent_member_class':'long g(double);template<class X>struct M{template<class T>auto h(T)->decltype(g(T()));};int g(int);template<class X>template<class U>auto M<X>::h(U x)->decltype(g(U())){return x;}int main(){M<char> m;static_assert(sizeof(m.h(0))==sizeof(long),"");return m.h(7)!=7;}',
 'dependent_pack':'long g(double);template<class...T>auto h(T...t)->decltype(g(t...));int g(int);template<class...U>auto h(U...u)->decltype(g(u...)){return g(u...);}int g(int){return 7;}int main(){static_assert(sizeof(h(0))==sizeof(long),"");return h(0)!=7;}',
}
runner.BAD={
 'fixed_call_ambiguous':'long g(double);template<class T>auto h(T)->decltype(g(0));int g(int);template<class T>auto h(T)->decltype(g(0));int main(){h(0);}',
 'dependent_missing':'template<class T>auto h(T)->decltype(g(T()));int g(int);template<class U>auto h(U)->decltype(g(U()));int main(){h(0);}',
 'dependent_missing_definition':'template<class T>auto h(T)->decltype(g(T()));int g(int);template<class U>auto h(U)->decltype(g(U())){return g(U());}int main(){h(0);}',
 'distinct_names':'long f(double);long g(double);template<class T>auto h(T)->decltype(f(T()));template<class T>auto h(T)->decltype(g(T()));int main(){h(0);}',
 'parameter_const_preserved':'long g(double);template<class T>int h(T,const decltype(g(T())) p);int g(int);template<class U>int h(U,const decltype(g(U())) p){p=3;return p;}int main(){h(0,7);}',
}
runner.GOOD.update({
 'fixed_sizeof_argument':'long g(double);template<class T>auto h(T x)->decltype(g(sizeof(T))){return x;}int g(unsigned long);template<class T>auto h(T x)->decltype(g(sizeof(T))){return x+1;}int main(){long(*a)(int)=h;int(*b)(int)=h;return a(7)!=7||b(7)!=8;}',
 'dependent_nontype_explicit':'template<int N>long g(double);template<int N>auto h()->decltype(g<N>(0));template<int N>int g(int);template<int M>auto h()->decltype(g<M>(0)){return 7;}static_assert(sizeof(h<3>())==sizeof(long),"");int main(){return h<3>()!=7;}',
 'member_parameter':'long g(double);template<class X>struct M{template<class T>int h(T,const decltype(g(T())) p);};int g(int);template<class X>template<class U>int M<X>::h(U,const decltype(g(U())) p){static_assert(sizeof(p)==sizeof(long),"");return p;}int main(){M<char>m;return m.h(0,7)!=7;}',
 'member_three':'long g(double);template<class X>struct M{template<class T>auto h(T)->decltype(g(T()));};int g(int);template<class X>template<class U>auto M<X>::h(U x)->decltype(g(U())){return x;}int main(){M<char>a;M<long>b;static_assert(sizeof(a.h(0))==sizeof(long)&&sizeof(b.h(0))==sizeof(long),"");return a.h(7)!=7||b.h(8)!=8;}',
})
runner.GOOD.update({
 'definition_drops_const':'long g(double);template<class T>int h(T,const decltype(g(T())) p);int g(int);template<class U>int h(U,decltype(g(U())) p){p=7;static_assert(sizeof(p)==sizeof(long),"");return p;}int main(){return h(0,3)!=7;}',
 'definition_adds_const':'long g(double);template<class T>int h(T,decltype(g(T())) p);int g(int);template<class U>int h(U,const decltype(g(U())) p){static_assert(sizeof(p)==sizeof(long),"");return p;}int main(){return h(0,7)!=7;}',
 'definition_array':'long g(double);template<class T>int h(T,decltype(g(T()))*p);int g(int);template<class U>int h(U,decltype(g(U())) p[3]){return p[1];}int main(){long a[3]={1,7,3};return h(0,a)!=7;}',
 'definition_pointer':'long g(double);template<class T>int h(T,decltype(g(T())) p[3]);int g(int);template<class U>int h(U,decltype(g(U()))*p){return p[1];}int main(){long a[3]={1,7,3};return h(0,a)!=7;}',
 'definition_function':'long g(double);template<class T>int h(T,decltype(g(T()))(*p)(int));int g(int);template<class U>int h(U,decltype(g(U())) p(int)){return p(7);}long a(int x){return x;}int main(){return h(0,a)!=7;}',
 'member_drops_const':'long g(double);template<class X>struct M{template<class T>int h(T,const decltype(g(T())) p);};int g(int);template<class X>template<class U>int M<X>::h(U,decltype(g(U())) p){p=7;static_assert(sizeof(p)==sizeof(long),"");return p;}int main(){M<char>m;return m.h(0,3)!=7;}',
})
runner.BAD['definition_const_assignment']='long g(double);template<class T>int h(T,decltype(g(T())) p);int g(int);template<class U>int h(U,const decltype(g(U())) p){p=7;return p;}int main(){h(0,3);}'
runner.GOOD['fixed_sizeof_template']='template<class X>long g(X);template<class T>auto h(T x)->decltype(g(sizeof(T))){return x;}int g(unsigned long);template<class T>auto h(T x)->decltype(g(sizeof(T))){return x+1;}int main(){long(*a)(int)=h;int(*b)(int)=h;return a(7)!=7||b(7)!=8;}'
runner.BAD['fixed_sizeof_missing']='template<class T>auto h()->decltype(g(sizeof(T)));int main(){}'
runner.BAD['fixed_sizeof_deleted']='long g(unsigned long)=delete;template<class T>auto h()->decltype(g(sizeof(T)));int main(){}'
ROOT=Path(__file__).resolve().parents[2]
fixture=(ROOT/'pa18/tests/general/300-function-template-result-first-lookup.t').read_text()
runner.GOOD['qualified_result_prefix']=fixture[:fixture.index('long selected(double);')]+fixture[fixture.index('static_assert(sizeof(nested::rooted'):fixture.index('static_assert(sizeof(selected_at_first_declaration')]+ 'int main(){}'
if __name__=='__main__':sys.exit(0 if runner.run(Path(sys.argv[1]).resolve(),Path(sys.argv[2])) else 1)
