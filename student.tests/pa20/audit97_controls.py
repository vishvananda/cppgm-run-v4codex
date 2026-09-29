#!/usr/bin/env python3
"""Cross-handoff storage/sequencing and callable/range controls. Run CC WORK."""
from pathlib import Path
import sys
sys.path.insert(0, str(Path(__file__).resolve().parents[1]/'pa18'))
import ordering_controls as runner

runner.GOOD = {
 'aggregate_self_scalar': 'struct S{int a;int b;};int main(){S s={3,s.a};return s.b!=3;}',
 'aggregate_self_array': 'struct S{int a[2];int b;};int main(){S s={{3,4},s.a[0]};return s.b!=3;}',
 'aggregate_array_elements': 'struct S{int a[2];int b;};int main(){S s[]={{{3,4},s[0].a[0]},{{5,6},s[0].b}};return s[0].b!=3||s[1].b!=3;}',
 'aggregate_array_inner': 'struct S{int a[2];int b;};int main(){S s[]={{{3,s[0].a[0]+1},5}};return s[0].a[1]!=4;}',
 'aggregate_scalar_elements': 'struct S{int a;int b;};int main(){S s[]={{3,s[0].a}};return s[0].b!=3;}',
 'aggregate_opaque_call': 'struct S{int a[2];int b;};S*p;int read(){return p->a[0];}int main(){S s[]={{{(p=s,3),4},read()}};return s[0].b!=3;}',
 'aggregate_reference_alias': 'struct S{int a[2];int b;};int main(){S s[]={{{3,4},s[0].a[0]}};int&v=s[0].b;S t[]={{{v,5},6}};return t[0].a[0]!=3;}',
 'aggregate_template_order': 'struct S{int a[2];int b;};template<int N>int f(){S s[]={{{N,s[0].a[0]+1},s[0].a[1]}};return s[0].b;}int main(){return f<3>()!=4||f<5>()!=6||f<3>()!=4;}',
 'aggregate_scalar_braces': 'struct S{int a;int b;};int main(){S s[]={{3,{s[0].a}}};return s[0].b!=3;}',
 'aggregate_single_argument': 'struct S{int a;};int main(){S s[]={{3},{s[0].a+1}};return s[1].a!=4;}',
 'aggregate_constructor_observes_prefix': 'int*p;struct M{int v;M(int n):v(*p+n){}M(M&&o):v(o.v){}};struct S{int first;M m;};int main(){S s[]={{3,M((p=&s[0].first,7))}};return s[0].m.v!=10;}',
 'range_scalar_temp': 'int main(){int a[2]={3,4};int n=0;for(const long&x:a)n+=x;return n!=7;}',
 'range_conversion_defaults': 'int alive;struct V{int n;V(int n,int a=2):n(n+a){++alive;}~V(){--alive;}};int main(){int a[2]={3,4};int n=0;for(const V&x:a){if(alive!=1)return 2;n+=x.n;}return alive||n!=11;}',
 'range_repeated_conversion': 'int alive;struct V{int n;V(int n):n(n){++alive;}~V(){--alive;}};struct P{int n;operator V(){return V(n);}};template<class T>int f(){P a[2]={{3},{4}};int n=0;for(const V&x:a){if(alive!=1)return 20;n+=x.n;}return n;}int main(){return f<int>()!=7||alive||f<long>()!=7||alive;}',
 'closure_static_range': 'int main(){auto f=[](){static int calls;int a[2]={3,4};int n=0;for(auto x:a)n+=x;return n+ ++calls;};int(*p)()=f;return f()!=8||p()!=9||f()!=10;}',
 'closure_return_range': 'struct V{int n;V(int n):n(n){}~V(){}};int main(){auto f=[](int n){int a[2]={3,4};for(auto x:a)if(n==x)return V(x);return V(7);};V(*p)(int)=f;return f(3).n!=3||p(4).n!=4||p(5).n!=7;}',
 'closure_nested_entries': 'int main(){auto f=[](int n){auto g=[](int x){return x+1;};int(*q)(int)=g;return g(n)+q(n);};int(*p)(int)=f;return f(3)!=8||p(4)!=10;}',
 'range_cv_array': 'int main(){const int a[2]={3,4};int n=0;for(auto x:a)n+=x;return n!=7;}',
}
runner.BAD = {}
if __name__ == '__main__':
    sys.exit(0 if runner.run(Path(sys.argv[1]).resolve(),Path(sys.argv[2])) else 1)
