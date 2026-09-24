#!/usr/bin/env python3
"""Immediate-context queries, recursive candidates and hard side effects.
N3485 [temp.deduct]/8, [temp.deduct.type], [over.match], [basic.lookup.argdep].
"""
from pathlib import Path
import sys
import entity_controls as runner
PREFIX = '''template<class T>T&& val();
template<class,class=void>struct probe{static const int n=0;};
'''
runner.GOOD = {
 'ambiguous_operator_then_valid': PREFIX+'''struct X{};struct L{L(X);};struct R{R(X);};int operator+(L,int);int operator+(R,int);struct Y{};int operator+(Y,int);
template<class T>struct probe<T,decltype(val<T>()+1,void())>{static const int n=1;};
static_assert(probe<X>::n==0 && probe<Y>::n==1 && probe<X>::n==0,"query ownership");int main(){return 0;}''',
 'no_viable_operator': PREFIX+'''struct X{};template<class T>struct probe<T,decltype(val<T>()+1,void())>{static const int n=1;};static_assert(probe<X>::n==0 && probe<int>::n==1,"viability");int main(){return 0;}''',
 'deleted_operator': PREFIX+'''struct X{};int operator+(X,int)=delete;template<class T>struct probe<T,decltype(val<T>()+1,void())>{static const int n=1;};static_assert(probe<X>::n==0 && probe<int>::n==1,"deleted");int main(){return 0;}''',
 'ambiguous_call': PREFIX+'''struct X{};struct L{L(X);};struct R{R(X);};int f(L);int f(R);int f(int);template<class T>struct probe<T,decltype(f(val<T>()),void())>{static const int n=1;};static_assert(probe<X>::n==0 && probe<int>::n==1,"call ambiguity");int main(){return 0;}''',
 'no_viable_call': PREFIX+'''struct X{};int f(int);template<class T>struct probe<T,decltype(f(val<T>()),void())>{static const int n=1;};static_assert(probe<X>::n==0 && probe<int>::n==1,"call viability");int main(){return 0;}''',
 'deleted_call': PREFIX+'''struct X{};int f(X)=delete;int f(int);template<class T>struct probe<T,decltype(f(val<T>()),void())>{static const int n=1;};static_assert(probe<X>::n==0 && probe<int>::n==1,"deleted call");int main(){return 0;}''',
 'return_sfinae_operator': '''template<class T>T&& val();struct X{};template<class T>auto f(T)->decltype(val<T>()[0]+1){return 7;}int f(...){return 3;}int main(){X x;int y;return f(&x)!=3||f(&y)!=7;}''',
 'default_sfinae_operator': '''template<class T>T&& val();struct X{};template<class T,class=decltype(val<T>()[0]+1)>int f(T){return 7;}int f(...){return 3;}int main(){X x;int y;return f(&x)!=3||f(&y)!=7;}''',
 'repeated_cv_probe': PREFIX+'''struct X{};int operator+(X&,int);template<class T>struct probe<T,decltype(val<T>()+1,void())>{static const int n=1;};static_assert(probe<X&>::n==1 && probe<const X&>::n==0 && probe<X&>::n==1,"cv key");int main(){return 0;}''',
 'adl_namespace_isolation': PREFIX+'''namespace a{struct X{};int operator+(X,int);}namespace b{struct X{};}template<class T>struct probe<T,decltype(val<T>()+1,void())>{static const int n=1;};static_assert(probe<a::X>::n==1 && probe<b::X>::n==0,"adl key");int main(){return 0;}''',
 'missing_member_query': PREFIX+'''struct X{};struct Y{int field;};template<class T>struct probe<T,decltype(val<T>().field,void())>{static const int n=1;};static_assert(probe<X>::n==0 && probe<Y>::n==1,"member query");int main(){return 0;}''',
 'callable_object_probe': PREFIX+'''struct X{};struct Y{int operator()()const;};template<class T>struct probe<T,decltype(val<T>()(),void())>{static const int n=1;};static_assert(probe<X>::n==0 && probe<Y>::n==1,"call operator");int main(){return 0;}''',
}
runner.GOOD.update({
 'deduced_after_default_hole': 'template<class T=int,class U>int f(U){return sizeof(T)+sizeof(U);}int main(){return f(char())!=5||f(1L)!=12;}',
 'candidate_default_after_later_declaration': 'template<class T,class U>int f(T){return sizeof(U);}int f(...){return 3;}int before(){return f(1);}template<class T,class U=int>int f(T);int main(){return before()!=3||f(1)!=4;}',
 'deleted_template_probe': PREFIX+'template<class T>int f(T)=delete;int f(int);template<class T>struct probe<T,decltype(f(val<T>()),void())>{static const int n=1;};static_assert(probe<char>::n==0 && probe<int>::n==1,"deleted template");int main(){return 0;}',
 'deleted_not_selected': 'int f(int){return 0;}int f(double)=delete;int main(){return f(1);}',
})
# Distinct namespace/type graphs exercise nested ADL and candidate deduplication.
RECURSIVE = r"""
template<class T>T&& val();
template<bool,class T=void>struct enable{};
template<class T>struct enable<true,T>{using type=T;};
template<class S,class T,class=void>struct streams{static const bool ok=false;};
template<class S,class T>struct streams<S,T,decltype(val<S&>() << val<T const&>(),void())>{static const bool ok=true;};
template<class S,class T,typename enable<streams<S,T>::ok,int>::type=0>
S&& operator<<(S&& s,T const& t){s<<t;return static_cast<S&&>(s);}
"""
for namespace in ('one','two'):
 runner.GOOD['recursive_hidden_friend_'+namespace] = RECURSIVE+f"""
 namespace {namespace}{{struct base{{}};struct sink:base{{int n;}};
 struct value{{int n;friend sink&operator<<(sink&s,value const&v){{s.n+=v.n;return s;}}}};}}
 int main(){{{namespace}::sink s;s.n=0;{namespace}::value a={{3}},b={{5}};
 s<<a;s<<b;s<<a;return s.n!=11;}}
 """
runner.GOOD['recursive_multiple_ordinary_paths'] = RECURSIVE+r"""
 struct sink{int n;};struct value{enum E{zero,one};E n;operator E()const{return n;}};
 sink&operator<<(sink&s,value v){s.n+=static_cast<int>(v.n);return s;}
 int main(){sink s;s.n=0;value v={value::one};s<<v;s<<v;return s.n!=2;}
 """
runner.GOOD['distinct_default_hole_bindings'] = 'template<class T=int,class U,class V>int f(U,V){return sizeof(T)+sizeof(U)*10+sizeof(V);}int main(){return f(char(),long())!=22||f(long(),char())!=85;}'
# A query's class definition side effect is outside the immediate context.
runner.BAD = {
 'late_deleted_definition': 'int f();int f()=delete;int main(){}',
 'deleted_redefinition': 'int f()=delete;int f(){return 0;}int main(){}',
 'hard_deleted_address': 'int f()=delete;auto p=&f;int main(){}',
 'hard_operator_ambiguity': '''struct X{};struct L{L(X);};struct R{R(X);};int operator+(L,int);int operator+(R,int);using T=decltype(X()+1);int main(){}''',
 'hard_deleted_operator': '''struct X{};int operator+(X,int)=delete;using T=decltype(X()+1);int main(){}''',
 'hard_missing_operator': '''struct X{};using T=decltype(X()+1);int main(){}''',
 'hard_call_ambiguity': '''struct X{};struct L{L(X);};struct R{R(X);};int f(L);int f(R);using T=decltype(f(X()));int main(){}''',
 'fixed_error_in_unused_template': '''struct X{};template<class T>struct Bad{using U=decltype(X()+1);};int main(){}''',
 'class_body_side_effect': '''template<class T>struct X{using bad=typename T::missing;int f();};template<class T>auto pick(T)->decltype(X<T>().f());int pick(...);int main(){return pick(1);}''',
 'class_body_operator_side_effect': '''template<class T>struct X{using bad=decltype(T()+1);int f();};struct S{};template<class T>auto pick(T)->decltype(X<T>().f());int pick(...);int main(){return pick(S());}''',
 'selected_body_error': '''template<class T>int f(T){return T::missing;}int f(...){return 0;}int main(){return f(1);}''',
}
if __name__=='__main__':sys.exit(0 if runner.run(Path(sys.argv[1]).resolve(),Path(sys.argv[2])) else 1)
