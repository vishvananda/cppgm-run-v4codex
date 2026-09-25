#!/usr/bin/env python3
"""Accumulated signature/cast/result ownership controls: CC WORK."""
from pathlib import Path
import sys
import ordering_controls as runner

runner.GOOD = {
 'unqualified_conversion': 'struct X{static const int n=7;operator int(){return n;}int f(){return operator int();}};int main(){X x;int a=x;return a!=7||x.f()!=7;}',
 'unqualified_conversion_alias': 'struct X{using T=int;static const int n=7;operator T(){return n;}int f(){using T=int;return operator T();}};int main(){X x;return x.f()!=7;}',
 'inherited_conversion_call': 'struct B{static const int n=7;operator int(){return n;}};struct X:B{int f(){return operator int();}};int main(){X x;int n=x;return n!=7||x.f()!=7;}',
 'template_conversion_call': 'template<class T>struct X{static const int n=7;operator T(){return n;}T f(){return operator T();}};int main(){X<long>x;long n=x;return n!=7||x.f()!=7;}',
 'summary_mutable_global': 'int n=7;struct X{operator int(){return n;}};int main(){X x;int a=x;n=9;int b=x;return a!=7||b!=9;}',
 'summary_static_reference': 'int n=7;struct X{static const int&r;operator int(){return r;}};const int&X::r=n;int main(){X x;int a=x;n=9;int b=x;return a!=7||b!=9;}',
 'summary_negative_float': 'struct X{static constexpr double n=-0.;operator double(){return n;}};int main(){X x;return double(x)!=0.;}',
 'conversion_query_address': 'struct X{static const int n=7;operator int(){return n;}};using P=decltype(&X::operator int);int main(){X x;P p=&X::operator int;int n=x;return n!=7||(x.*p)()!=7;}',
 'conversion_query_call': 'struct X{static const int n=7;operator int(){return n;}auto f()->decltype(operator int()){return operator int();}};int main(){X x;int n=x;return n!=7||x.f()!=7;}',
 'template_conversion_query': 'template<class T>struct X{static const int n=7;operator T(){return n;}auto f()->decltype(operator T()){return operator T();}};int main(){X<int>a;X<long>b;static_assert(sizeof(a.f())==sizeof(int)&&sizeof(b.f())==sizeof(long),"");return a.f()!=7||b.f()!=7;}',
 'conversion_fixed_member_query': 'template<int N>struct X{static const int value=N;operator int(){return value;}auto f()->decltype(operator int()){return operator int();}};int main(){X<7>a;X<9>b;int n=a;return n!=7||a.f()!=7||b.f()!=9;}',
 'conversion_fixed_base_query': 'struct B{operator int(){return 7;}};template<class T>struct X:B{auto f()->decltype(operator int()){return operator int();}};int main(){X<long>x;return x.f()!=7;}',
 'conversion_local_pattern_query': 'template<class T>int f(){struct X{operator int(){return 7;}auto g()->decltype(operator int()){return operator int();}};X x;return x.g();}int main(){return f<long>()!=7;}',
 'conversion_query_dormant_body': 'template<class T>struct X{operator int(){return T::missing;}auto f()->decltype(operator int());};static_assert(sizeof(X<int>)==1,"");int main(){}',
 'conversion_template_call': 'struct X{template<class T>operator T(){return T(7);}int f(){return operator int();}};int main(){X x;return x.f()!=7;}',
 'conversion_member_template_query': 'template<class U>struct X{template<class T>operator T(){return T(7);}auto f()->decltype(operator int()){return operator int();}};int main(){X<long>x;return x.f()!=7;}',
 'conversion_source_cv_candidates': 'template<class U>struct X{operator int(){return 9;}template<class T>operator T()const{return T(7);}auto f()const->decltype(operator int()){return operator int();}};int main(){X<long>x;return x.f()!=7;}',
 'fixed_base_ordinary_query': 'struct B{int g(){return 7;}};template<class T>struct X:B{auto f()->decltype(g()){return g();}};int main(){X<long>x;return x.f()!=7;}',
 'unqualified_local_alias': 'struct X{using T=long;operator int(){return 7;}operator T(){return 9;}int f(){using T=int;return operator T();}};int main(){X x;return x.f()!=7;}',
 'conditional_reference_storage': 'struct X{static const bool n=true;operator bool(){return n;}};int main(){X x;const int&a=x?7:9;const int&b=x?7:9;return a!=7||b!=7||&a==&b;}',
 'constant_reference_distinct': 'constexpr bool f(const int&a,const int&b){return &a!=&b;}static_assert(f(static_cast<const int&>(7),static_cast<const int&>(7)),"");int main(){}',
 'signature_renamed_pack_parameter': 'long g(double);template<class...T>int h(decltype(g(T()))...p);int g(int);template<class...U>int h(decltype(g(U()))...p){return sizeof...(p);}int main(){return h<int,long>(7,8)!=2;}',
 'signature_two_parameter_heads': 'long g(double);template<class T,class U>auto f(T, U)->decltype(g(T()));int g(int);template<class V,class W>auto f(V x,W)->decltype(g(V())){return x;}int main(){static_assert(sizeof(f(7,1))==sizeof(long),"");return f(7,1)!=7;}',
}
runner.BAD = {
 'unqualified_deleted_conversion': 'struct X{operator int()=delete;int f(){return operator int();}};int main(){X x;return x.f();}',
 'qualified_query_mismatch': 'using T=int;struct X{using T=long;operator T(){return 7;}};using P=decltype(&X::operator T);int main(){}',
 'query_deleted_conversion': 'struct X{operator int()=delete;auto f()->decltype(operator int());};int main(){}',
 'query_private_base_conversion': 'class B{operator int(){return 7;}};template<class T>struct X:B{auto f()->decltype(operator int());};int main(){}',
}
if __name__ == '__main__':
 sys.exit(0 if runner.run(Path(sys.argv[1]).resolve(),Path(sys.argv[2])) else 1)
