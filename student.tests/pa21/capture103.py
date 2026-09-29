#!/usr/bin/env python3
"""PA21 checked closure storage, conversion demand, and lifetime controls."""
from pathlib import Path
import sys
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'pa18'))
import ordering_controls as runner
runner.GOOD={
'copy':'int main(){int n=3;auto f=[n](){return n;};n=7;return f()!=3;}',
'default_copy':'int main(){int n=3,m=4;auto f=[=](){return n+m;};n=9;return f()!=7;}',
'mutable':'int main(){int n=3;auto f=[n]()mutable{return ++n;};return f()!=4||f()!=5||n!=3;}',
'mixed':'int main(){int n=3,m=4;auto f=[=,&m](){m+=n;};n=9;f();return m!=7;}',
'mixed_reference':'int main(){int n=3,m=4;auto f=[&,n](){m+=n;};n=9;f();return m!=7;}',
'reference_copy':'int main(){int n=3;int&r=n;auto f=[r]()mutable{return ++r;};return f()!=4||n!=3;}',
'nested_copy':'int main(){int n=3;auto f=[n]()mutable{auto g=[n]()mutable{return ++n;};++n;return g()+n;};return f()!=8||n!=3;}',
'nested_reference':'int main(){int n=3;auto f=[n]()mutable{auto g=[&n](){return ++n;};return g();};return f()!=4||n!=3;}',
'nested_copy_reference':'int main(){int n=3;auto f=[&n](){auto g=[n](){return n;};++n;return g();};return f()!=3||n!=4;}',
'nested_default':'int main(){int n=3;auto f=[=](){auto g=[=](){return n;};return g();};n=9;return f()!=3;}',
'nested_shadow':'int main(){int n=3;auto f=[n](){int n=7;auto g=[n](){return n;};return g();};return f()!=7;}',
'default_this':'struct S{int n;int f(){auto g=[=](){return ++n;};return g();}};int main(){S s={3};return s.f()!=4||s.n!=4;}',
'class_trivial':'struct S{int n;};int main(){S s={3};auto f=[s](){return s.n;};s.n=9;return f()!=3;}',
'class_mutable':'struct S{int n;};int main(){S s={3};auto f=[s]()mutable{return ++s.n;};return f()!=4||s.n!=3;}',
'copy_dependency':'int count;struct S{S(){}S(const S&){++count;}};int main(){S s;auto f=[s](){};return count!=1;}',
'copy_template_dependency':'int count;template<class T>struct S{S(){}S(const S&){++count;}};int main(){S<int>s;auto f=[s](){};return count!=1;}',
'copy_default_argument':'int count;int tick(){return ++count;}struct S{S(){}S(const S&,int n=tick()){count+=n;}};int main(){S s;auto f=[s](){};return count!=2;}',
'destruction':'int count;struct S{int n;S(int x):n(x){}S(const S&s):n(s.n){++count;}~S(){count+=n;}};int main(){{S s(3);{auto f=[s](){};}if(count!=4)return 1;}return count!=7;}',
'closure_copy_destruction':'int count;struct S{S(){}S(const S&){++count;}~S(){count+=10;}};int main(){{S s;auto f=[s](){};{auto g=f;}}return count!=32;}',
'class_reference':'int count;struct S{S(){}S(const S&){++count;}~S(){count+=10;}};int main(){{S s;auto f=[&s](){};}return count!=10;}',
'const_source':'int main(){const int n=3;auto f=[n]()mutable{return n;};return f()!=3;}',
'const_address':'int main(){const int n=3;auto f=[n](){return &n;};return f()==&n||*f()!=3;}',
'array':'int main(){int a[2]={3,4};auto f=[a]()mutable{a[0]+=a[1];return a[0];};return f()!=7||f()!=11||a[0]!=3;}',
'array_nested':'int main(){int a[2][2]={{1,2},{3,4}};auto f=[a](){return a[1][1];};a[1][1]=9;return f()!=4;}',
'array_large':'int main(){int a[32]={};a[31]=7;auto f=[a](){return a[31];};a[31]=9;return f()!=7;}',
'array_class':'int count;struct S{int n;S(int n=3):n(n){}S(const S&s):n(s.n){++count;}};int main(){S a[2];auto f=[a](){return a[1].n;};return count!=2||f()!=3;}',
'array_class_defaults':'int count;int tick(){return ++count;}struct S{S(){}S(const S&,int n=tick()){count+=n;}};int main(){S a[2];auto f=[a](){};return count!=6;}',
'function_reference':'int f(){return 7;}int main(){int(&r)()=f;auto g=[r](){return r();};return g()!=7;}',
'function_template':'template<class T>int run(T n){auto f=[n](){return n+1;};return f();}int main(){return run(3)!=4||run(5L)!=6;}',
'member_template':'struct S{template<class T>int run(T n){auto f=[n](){return n+1;};return f();}};int main(){S s;return s.run(3)!=4||s.run(5L)!=6;}',
'pack':'int sum(int a,long b){return a+b;}template<class...T>int f(T...args){auto g=[args...](){return sum(args...);};return g();}int main(){return f(3,4L)!=7;}',
'pack_empty':'template<class...T>int f(T...args){auto g=[args...](){return sizeof...(args);};return g();}int main(){return f();}',
'decltype':'template<class A,class B>struct same{static const bool value=false;};template<class A>struct same<A,A>{static const bool value=true;};int main(){int n=3;auto f=[n](){static_assert(same<decltype(n),int>::value,"declared");static_assert(same<decltype((n)),const int&>::value,"expression");return n;};return f()!=3;}',
'reference_parameter_shadow':'int main(){int n=3;auto f=[n](){auto g=[&](int n){return n;};return g(7);};return f()!=7;}',
}
runner.GOOD.update({
'destructible_argument':'int count;struct S{S(){}S(const S&){++count;}~S(){count+=10;}};template<class F>void run(F f){f();}int main(){{S s;run([s](){});}return count!=21;}',
'destructible_lvalue_argument':'int count;struct S{S(){}S(const S&){++count;}~S(){count+=10;}};template<class F>void run(F f){f();}int main(){{S s;auto f=[s](){};run(f);}return count!=32;}',
'class_template_destruction':'int count;template<class T>struct S{S(){}S(const S&){++count;}~S(){count+=10;}};int main(){{S<int>s;auto f=[s](){};}return count!=21;}',
'nested_default_const_copy':'int main(){int n=3;auto f=[n](){auto g=[n]()mutable{return n;};return g();};return f()!=3;}',
'copy_reference_field':'struct S{int&n;};int main(){int n=3;S s={n};auto f=[s](){return ++s.n;};return f()!=4||n!=4;}',
'mutable_field':'struct S{mutable int n;};int main(){S s={3};auto f=[s](){return ++s.n;};return f()!=4||f()!=5||s.n!=3;}',
'volatile_copy':'int main(){volatile int n=3;auto f=[n]()mutable{return ++n;};return f()!=4||n!=3;}',
'decltype_without_capture':'template<class A,class B>struct same{static const bool value=false;};template<class A>struct same<A,A>{static const bool value=true;};int main(){int n=3;auto f=[=](){static_assert(same<decltype((n)),const int&>::value,"expression");return 0;};return sizeof(f)!=1||f();}',
'decltype_template':'template<class A,class B>struct same{static const bool value=false;};template<class A>struct same<A,A>{static const bool value=true;};template<class T>int f(T n){auto g=[n](){static_assert(same<decltype((n)),const T&>::value,"expression");return n;};return g();}int main(){return f(3)!=3;}',
'decltype_nested_reference':'template<class A,class B>struct same{static const bool value=false;};template<class A>struct same<A,A>{static const bool value=true;};int main(){int n=3;auto f=[n](){auto g=[&n](){static_assert(same<decltype((n)),const int&>::value,"expression");return n;};return g();};return f()!=3;}',
'decltype_local_shadow':'template<class A,class B>struct same{static const bool value=false;};template<class A>struct same<A,A>{static const bool value=true;};int main(){int n=3;auto f=[n](){int n=4;static_assert(same<decltype((n)),int&>::value,"expression");return n;};return f()!=4;}',
'array_default_temporaries':'int count,seen;struct G{G(){++count;}~G(){--count;}};struct S{S(){}S(const S&,const G& = G()){seen+=count;}};int main(){S s[3];auto f=[s](){};return seen!=3||count!=0;}',
'copy_ctor_template':'int count;struct S{S(){}S(S&){count+=1;}template<class T>S(const T&){count+=10;}};int main(){S s;auto f=[s](){};return count!=1;}',
'copy_deleted_move':'struct S{int n;S(int v):n(v){}S(const S&)=default;S(S&&)=delete;};int main(){S s(3);auto f=[s](){return s.n;};return f()!=3;}',
})
runner.GOOD.update({
'closure_copy_defaults':'int count;int tick(){return ++count;}struct S{S(){}S(const S&,int n=tick()){count+=n;}};int main(){S s;auto f=[s](){};auto g=f;return count!=6;}',
'closure_move_template':'int count;struct S{S(){}S(S&){count+=1;}template<class T>S(const T&){count+=10;}};int main(){S s;auto f=[s](){};auto g=static_cast<decltype(f)&&>(f);return count!=11;}',
})
runner.GOOD['closure_copy_default_temp']='int count,seen;struct G{G(){++count;}~G(){--count;}};struct S{S(){}S(const S&,const G& = G()){seen+=count;}};int main(){S s;auto f=[s](){};auto g=f;return seen!=2||count!=0;}'
runner.BAD={
'const_operator':'int main(){int n=3;auto f=[n](){return ++n;};}',
'const_source_mutable':'int main(){const int n=3;auto f=[n]()mutable{return ++n;};}',
'const_class_operator':'struct S{int n;};int main(){S s={3};auto f=[s](){++s.n;};}',
'nested_const':'int main(){int n=3;auto f=[n](){auto g=[&n](){return ++n;};};}',
'deleted_copy':'struct S{S(){}S(const S&)=delete;};int main(){S s;auto f=[s](){};}',
'private_copy':'class S{S(const S&);public:S(){}};int main(){S s;auto f=[s](){};}',
'duplicate':'int main(){int n;auto f=[n,n](){};}',
'duplicate_mixed':'int main(){int n;auto f=[n,&n](){};}',
'redundant_copy':'int main(){int n;auto f=[=,n](){};}',
'parameter_conflict':'int main(){int n;auto f=[n](int n){};}',
'global_capture':'int n;int main(){auto f=[n](){};}',
'static_capture':'int main(){static int n;auto f=[n](){};}',
'missing_outer':'int main(){int n;auto f=[](){auto g=[n](){};};}',
}
if __name__=='__main__':sys.exit(0 if runner.run(Path(sys.argv[1]).resolve(),Path(sys.argv[2])) else 1)
