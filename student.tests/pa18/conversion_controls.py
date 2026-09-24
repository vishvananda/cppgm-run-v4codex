#!/usr/bin/env python3
"""Target-driven conversion deduction, selection and demand; CC WORK."""
from pathlib import Path
import sys
import ordering_controls as runner
GOOD={
 'scalar':'struct S{template<class T>operator T()const{return T(7);}};int main(){S s;int i=s;long l=s;return i!=7||l!=7;}',
 'pointer_identities':'struct S{template<class T>explicit operator T*()const{return nullptr;}};int main(){S s;int*p=static_cast<int*>(s);const int*q=static_cast<const int*>(s);return p!=q;}',
 'object_result':'struct A{int n;};struct S{template<class T>operator T()const{return T{7};}};int main(){S s;A a=s;return a.n!=7;}',
 'reference':'int n=7;struct S{template<class T>operator T&()const{return n;}};int main(){S s;int&r=s;r=9;return n!=9;}',
 'const_reference':'int n=7;struct S{template<class T>operator const T&()const{return n;}};int main(){S s;const int&r=s;return &r!=&n;}',
 'rvalue_reference':'int n=7;struct S{template<class T>operator T&&()const{return static_cast<T&&>(n);}};int main(){S s;int&&r=s;r=9;return n!=9;}',
 'conditional_reference':'int n=7;struct S{template<class T>operator T&()const{return n;}};int main(){S s;int&r=true?s:n;r=9;return n!=9;}',
 'ordinary_wins':'struct S{operator int()const{return 7;}template<class T>operator T()const{return T::missing;}};int main(){S s;int n=s;return n!=7;}',
 'result_rank_before_ordinary':'struct S{operator long()const{return 3;}template<class T>operator T()const{return T(7);}};int main(){S s;int n=s;return n!=7;}',
 'object_rank_before_result':'struct S{operator long(){return 3;}template<class T>operator T()const{return T(7);}};int main(){S s;int n=s;return n!=3;}',
 'pointer_ordering':'int chosen;struct S{template<class T>operator T()const{chosen=1;return T();}template<class T>operator T*()const{chosen=2;return nullptr;}};int main(){S s;int*p=s;return p!=nullptr||chosen!=2;}',
 'reference_ordering':'int n;int chosen;struct S{template<class T>operator T&()const{chosen=1;return n;}template<class T>operator const T&()const{chosen=2;return n;}};int main(){S s;const int&r=s;return &r!=&n||chosen!=2;}',
 'head_default_ignored_ordering':'int chosen;struct S{template<class T,class U=void>operator T()const{chosen=1;return T();}template<class T>operator T*()const{chosen=2;return nullptr;}};int main(){S s;int*p=s;return p!=nullptr||chosen!=2;}',
 'unused_body':'struct S{template<class T>operator T*()const{return T::missing;}template<class T>operator T()const{return T(7);}};int main(){S s;int n=s;return n!=7;}',
 'sfinae_default':'struct S{template<class T,class U=typename T::missing>operator T()const{return T::bad;}operator int()const{return 7;}};int main(){S s;int n=s;return n!=7;}',
 'sfinae_result':'template<class T>struct Id{typedef T type;};struct S{template<class T,class=typename T::missing>operator T*()const{return 0;}operator int*()const{return 0;}};int main(){S s;int*p=s;return p!=0;}',
 'default_only':'struct S{template<class U=int>operator int()const{return sizeof(U);}};int main(){S s;int n=s;return n!=sizeof(int);}',
 'inherited':'struct B{template<class T>operator T()const{return T(7);}};struct S:B{};int main(){S s;int n=s;return n!=7;}',
 'ref_qualified':'struct S{template<class T>operator T()&{return T(3);}template<class T>operator T()&&{return T(7);}};int main(){S s;int a=s;int b=S();return a!=3||b!=7;}',
 'member_owner':'template<class A>struct S{A n;template<class T>operator T()const{return T(n);}};int main(){S<long>s={7};int n=s;return n!=7;}',
 'nested_pointer_cv':'template<class T,class U>struct Same{static const bool value=false;};template<class T>struct Same<T,T>{static const bool value=true;};int chosen;struct S{template<class T>operator T***()const{chosen=Same<T,int>::value;return 0;}};int main(){S s;const int*const*const*p=s;return p!=0||chosen!=1;}',
 'pointer_cv_exact_first':'template<class T,class U>struct Same{static const bool value=false;};template<class T>struct Same<T,T>{static const bool value=true;};int chosen;struct S{template<class T>operator T*()const{chosen=Same<T,const int>::value;return 0;}};int main(){S s;const int*p=s;return p!=0||chosen!=1;}',
 'array_reference_decay':'int n[3];struct S{template<class T>using Arr=T[3];template<class T>operator Arr<T>&()const{return n;}};int main(){S s;int*p=s;return p!=n;}',
 'explicit_call':'struct S{template<class T>operator T*()const{return nullptr;}};int main(){S s;return s.operator int*()!=nullptr;}',
 'explicit_call_ordering':'int chosen;struct S{template<class T>operator T()const{chosen=1;return T();}template<class T>operator T*()const{chosen=2;return nullptr;}};int main(){S s;return s.operator int*()!=nullptr||chosen!=2;}',
 'constructor_beats_template':'struct S{template<class T>operator T()const{return T::missing;}};struct A{int n;A(S):n(7){}};A f(){return S();}int main(){return f().n!=7;}',
 'template_beats_constructor':'struct S;struct A{int n;A(int x):n(x){}template<class T>A(T):n(3){}};struct S{operator A()const{return A(7);}};int main(){const S s={};A a=s;return a.n!=7;}',
 'class_value_reference_targets':'struct A{int n;};struct S{template<class T>operator T()const{return T{7};}};int main(){S s;A a={};a=s;const A&r=s;return a.n!=7||r.n!=7;}',
 'constexpr_conditional_reference':'constexpr int n=7;struct S{constexpr S(){}template<class T>constexpr operator const T&()const{return n;}};constexpr const int&r=true?S():n;static_assert(&r==&n,"");int main(){return r!=7;}',
 'class_trailing_qualifier':'struct A{int n;};struct S{template<class T>operator T()const{return T{7};}}const s={};int main(){A a=s;return a.n!=7;}',
 'constructor_local_relational':'struct S{int n;template<class T>S(T x):n(0){for(int i=0;i<3;++i)n+=x;}};int main(){S s(2);return s.n!=6;}',
 'inherited_template_hidden':'struct B{template<class T>operator T()const{return T::missing;}};struct S:B{template<class U>operator U()const{return U(7);}};int main(){S s;int n=s;return n!=7;}',
 'inherited_ordinary_preferred':'struct B{operator int()const{return 7;}};struct S:B{template<class T>operator T()const{return T::missing;}};int main(){S s;int n=s;return n!=7;}',
 'inherited_fixed_template_hides':'struct B{operator int()const{return 3;}};struct S:B{template<class T=int>operator int()const{return 7;}};int main(){S s;int n=s;return n!=7;}',
 'inherited_explicit_not_viable':'struct B{operator int()const{return 7;}};struct S:B{template<class T>explicit operator T()const{return T::missing;}};int main(){S s;int n=s;return n!=7;}',
 'member_nontype_head':'template<class A>struct S{template<class T,T N=7>int f(){return N;}};int main(){S<void>s;return s.f<int>()!=7;}',
 'member_nontype_head_shadow':'template<class T>struct S{template<class U,U N=7>int f(){return N;}};int main(){S<float>s;return s.f<int>()!=7;}',
 'member_nontype_outer_head':'template<class T>struct S{template<T N=7>int f(){return N;}};int main(){S<int>s;return s.f<>()!=7;}',
}
BAD={
 'explicit_implicit':'struct S{template<class T>explicit operator T()const{return T();}};int main(){S s;int n=s;}',
 'no_promotion':'struct S{template<class U=int>operator short()const{return 1;}};int main(){S s;int n=s;}',
 'no_base_conversion':'struct B{};struct D:B{};struct S{template<class U=int>operator D*()const{return 0;}};int main(){S s;B*p=s;}',
 'no_void_conversion':'struct S{template<class U=int>operator int*()const{return 0;}};int main(){S s;void*p=s;}',
 'no_cv_drop':'struct S{template<class T>operator const T*()const{return 0;}};int main(){S s;int*p=s;}',
 # N3485 [temp.deduct.conv]/7 deduces the unqualified terminal int in the
 # fallback, then [conv.qual] rejects this missing intermediate const. CWG
 # 2384 later removed /7; modern host behavior is not this C++11 oracle.
 'no_nested_cv_drop':'struct S{template<class T>operator T***()const{return 0;}};int main(){S s;const int**const*p=s;}',
 'no_reference_cv_drop':'int n;struct S{template<class T>operator const T&()const{return n;}};int main(){S s;int&r=s;}',
 'head_cannot_deduce':'struct S{template<class T>operator int()const{return 7;}};int main(){S s;int n=s;}',
 'selected_body_error':'struct S{template<class T>operator T()const{return T::missing;}};int main(){S s;int n=s;}',
 'deleted_selected':'struct S{template<class T>operator T()const=delete;};int main(){S s;int n=s;}',
 'private_selected':'class S{template<class T>operator T()const{return T();}};int main(){S s;int n=s;}',
 'ambiguous_heads':'struct S{template<class T>operator T()const{return T();}template<class T,class U=void>operator T()const{return T();}};int main(){S s;int n=s;}',
 'const_receiver':'struct S{template<class T>operator T(){return T();}};int main(){const S s={};int n=s;}',
}
if __name__=='__main__':
 runner.GOOD=GOOD;runner.BAD=BAD
 sys.exit(0 if runner.run(Path(sys.argv[1]).resolve(),Path(sys.argv[2])) else 1)
