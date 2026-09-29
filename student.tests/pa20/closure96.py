#!/usr/bin/env python3
"""Captureless closure ABI, defaults, shared body facts and storage. Run CC WORK."""
from pathlib import Path
import sys
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'pa18'))
import ordering_controls as runner
runner.GOOD={
 'immediate':'int main(){return [](int x){return x+1;}(3)!=4;}',
 'variable_and_pointer':'int main(){auto f=[](int n){return n+1;};int(*p)(int)=f;return f(3)!=4||p(4)!=5;}',
 'pointer_only':'int main(){int(*p)(int)=[](int n){return n+1;};return p(3)!=4;}',
 'pointer_shared_static':'int main(){auto f=[](){static int n;return ++n;};int(*p)()=f;return f()!=1||p()!=2||f()!=3;}',
 'specialization_statics':'template<class T>int f(){auto a=[](){static int n;return ++n;};auto b=[](){static int n;return ++n;};int(*p)()=a;return a()+p()+b();}int main(){return f<int>()!=4||f<long>()!=4||f<int>()!=9;}',
 'pointer_reference':'int main(){auto f=[](int&x)->int&{return x;};int&(*p)(int&)=f;int n=3;p(n)=4;return n!=4;}',
 'default_variable':'int main(){auto f=[](int x,int y=2){return x+y;};return f(3)!=5||f(3,4)!=7;}',
 'default_immediate':'int main(){return [](int x,int y=2){return x+y;}(3)!=5;}',
 'default_effects':'int n;int next(){return ++n;}int main(){auto f=[](int x=next()){return x;};return f()!=1||f()!=2||f(7)!=7||n!=2;}',
 'default_template':'template<class T>int f(){auto g=[](int x=sizeof(T)){return x;};return g();}int main(){return f<char>()!=1||f<long>()!=8;}',
 'parameter_adjustment':'int main(){auto f=[](const int n){return n;};int(*p)(int)=f;return p(3)!=3;}',
 'parameter_array':'int main(){auto f=[](int a[2]){return a[0]+a[1];};int(*p)(int*)=f;int a[2]={3,4};return p(a)!=7;}',
 'pointer_aggregate_result':'struct V{int n;int m;};int main(){auto f=[](int n){return V{n,n+1};};V(*p)(int)=f;V v=p(3);return v.n!=3||v.m!=4;}',
 'pointer_indirect_result':'struct V{int n;V(int n):n(n){}V(const V&v):n(v.n){}};int main(){auto f=[](int n){return V(n);};V(*p)(int)=f;V v=p(3);return v.n!=3;}',
 'friend_access':'class S{static int f(){return 7;}friend int g();};int g(){auto f=[](){return S::f();};int(*p)()=f;return p();}int main(){return g()!=7;}',
 'copy_move':'int main(){auto f=[](int n){return n;};auto g=f;auto h=static_cast<decltype(f)&&>(f);return g(3)!=3||h(4)!=4;}',
 'closure_receiver_effects':'int n;int main(){auto f=[](){return 7;};int(*p)()=(++n,f);return n!=1||p()!=7;}',
 'two_entries_calls':'int n;int g(int x){++n;return x;}int main(){auto f=[](int x){return g(x);};int(*p)(int)=f;return p(3)!=3||f(4)!=4||n!=2;}',
 'two_entries_labels':'int main(){auto f=[](int n){int x=0;again:if(n){x+=n;--n;goto again;}return x;};int(*p)(int)=f;return f(3)!=6||p(4)!=10;}',
 'two_entries_cleanup':'int alive;struct V{V(){++alive;}~V(){--alive;}};int main(){auto f=[](){V v;return alive;};int(*p)()=f;return f()!=1||alive||p()!=1||alive;}',
}
runner.GOOD.update({
 'two_entries_forward_labels':'int main(){auto f=[](int n){if(n)goto done;n=3;done:return n;};int(*p)(int)=f;return f(0)!=3||p(4)!=4;}',
 'two_entries_switch':'int main(){auto f=[](int n){switch(n){case 3:return 7;case 4:return 8;default:return 9;}};int(*p)(int)=f;return f(3)!=7||p(4)!=8||p(5)!=9;}',
 'two_entries_range':'int main(){auto f=[](int n){int a[2]={n,n+1};int sum=0;for(auto x:a)sum+=x;return sum;};int(*p)(int)=f;return f(3)!=7||p(4)!=9;}',
 'two_entries_reference_local':'int main(){auto f=[](int n){const int&r=n+1;return r;};int(*p)(int)=f;return f(3)!=4||p(4)!=5;}',
 'two_entries_member_range':'struct R{int a[2];int*begin(){return a;}int*end(){return a+2;}};int main(){auto f=[](int n){R r={{n,n+1}};int sum=0;for(auto x:r)sum+=x;return sum;};int(*p)(int)=f;return f(3)!=7||p(4)!=9;}',
 'pointer_reference_binding':'int main(){auto f=[](){return 7;};int(*const&r)()=f;return r()!=7;}',
 'explicit_conversion':'int main(){auto f=[](){return 7;};using P=int(*)();P p=f.operator P();return p()!=7;}',
 'pointer_class_parameter':'int copies;struct V{int n;V(int n):n(n){}V(const V&v):n(v.n){++copies;}};int main(){auto f=[](V v){return v.n;};int(*p)(V)=f;V v(3);return f(v)!=3||p(v)!=3||copies!=2;}',
 'default_lookup':'int val=3;template<class T>int f(){auto g=[](int n=val){return n;};int val=4;return g();}int main(){return f<int>()!=3;}',
})
runner.BAD={
 'unused_template_bad_default':'template<class T>void f(){auto g=[](int n=missing){return n;};}int main(){}',
 'default_uses_parameter':'int main(){auto f=[](int x,int y=x){return y;};}',
 'default_constructor':'int main(){auto f=[](){};decltype(f) g;}',
 'assignment':'int main(){auto f=[](){};f=f;}',
 'pointer_default_arity':'int main(){auto f=[](int x=1){return x;};int(*p)(int)=f;return p();}',
 'uncaptured_local':'int main(){int x=1;return [](){return x;}();}',
 'private_member':'class S{int f(){return 7;}};int main(){S s;return [](S&v){return v.f();}(s);}',
}
if __name__=='__main__':sys.exit(0 if runner.run(Path(sys.argv[1]).resolve(),Path(sys.argv[2])) else 1)
