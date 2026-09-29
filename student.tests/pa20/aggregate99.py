#!/usr/bin/env python3
"""Class member transport must preserve identity, effects and list order. Run CC WORK."""
from pathlib import Path
import sys
sys.path.insert(0, str(Path(__file__).resolve().parents[1]/'pa18'))
import ordering_controls as runner

runner.GOOD = {
 'omitted_tail': 'struct V{int n;V():n(7){}};struct S{int n;V v;};S f(int n){return {n};}int main(){S a=f(3),b=S{};return a.n!=3||a.v.n!=7||b.n||b.v.n!=7;}',
 'multiple_omitted': 'struct V{int n;V():n(7){}};struct S{int a;V x;int b;V y;};S f(int n){return {n};}int main(){S a=f(3);return a.a!=3||a.x.n!=7||a.b||a.y.n!=7;}',
 'class_holes': 'struct V{int n;V():n(7){}};struct S{V x;int a;V y;};S f(int n){return {{},n,{}};}int main(){S a=f(3);return a.a!=3||a.x.n!=7||a.y.n!=7;}',
 'explicit_class_list': 'struct V{int n;V(int n):n(n){}};struct S{int a;V x;int b;V y;};S f(int n){return {n,{4},5,{6}};}int main(){S a=f(3);return a.a!=3||a.x.n!=4||a.b!=5||a.y.n!=6;}',
 'template_instances': 'template<class T>struct V{T n;V():n(7){}};template<class T>struct S{T a;V<T>x;};template<class T>S<T>f(T n){return {n};}int main(){S<int>a=f(3);S<long>b=f(4L);S<int>c=f(5);return a.a!=3||b.a!=4||c.a!=5||a.x.n!=7||b.x.n!=7||c.x.n!=7;}',
 'default_argument': 'struct V{int n;V(int n=7):n(n){}};struct S{int a;V v;};S f(int n){return {n};}int main(){S a=f(3);return a.a!=3||a.v.n!=7;}',
 'default_argument_effect': 'int n;int next(){return ++n;}struct V{int n;V(int n=next()):n(n){}};struct S{int a;V x;int b;V y;};S f(){return {next()};}int main(){S a=f();return a.a!=1||a.x.n!=2||a.b||a.y.n!=3||n!=3;}',
 'body_effect': 'int n;struct V{int x;V():x(++n){++n;}};struct S{int a;V x;V y;};S f(){return {++n};}int main(){S a=f();return a.a!=1||a.x.x!=2||a.y.x!=4||n!=5;}',
 'self_address': 'struct V{V*p;V():p(this){}};struct S{int a;V v;};S f(int n){return {n};}int main(){S s=S{3};return s.a!=3||s.v.p!=&s.v;}',
 'member_address': 'struct V{int n;int*p;V():n(7),p(&n){}};struct S{int a;V v;};int main(){S s=S{3};return s.a!=3||s.v.p!=&s.v.n||*s.v.p!=7;}',
 'escaped_address': 'int*p;struct V{int n;V():n(7){p=&n;}};struct S{int a;V v;};int main(){S s=S{3};return p!=&s.v.n||*p!=7;}',
 'destructor': 'int n;struct V{int x;V():x(7){}~V(){++n;}};struct S{int a;V v;};S f(){return {3};}int main(){{S s=f();if(n||s.a!=3||s.v.x!=7)return 1;}return n!=1;}',
 'nontrivial_copy': 'int copies;struct V{int n;V():n(7){}V(const V&o):n(o.n){++copies;}};struct S{int a;V v;};S f(){return {3};}int main(){S s=f();return copies||s.a!=3||s.v.n!=7;}',
 'deleted_copy': 'struct V{int n;V():n(7){}V(const V&)=delete;V(V&&)=default;};struct S{int a;V v;};S f(){return {3};}int main(){S s=f();return s.a!=3||s.v.n!=7;}',
 'volatile_member': 'struct V{volatile int n;V():n(7){}};struct S{int a;V v;};int main(){S s=S{3};return s.a!=3||s.v.n!=7;}',
 'prior_destination_read': 'struct V{int n;V(int n):n(n){}};struct S{int a;V v;};int main(){S s=S{3,{s.a}};return s.a!=3||s.v.n!=3;}',
 'class_then_destination_read': 'struct V{int n;V():n(7){}};struct S{V v;int a;};int main(){S s=S{{},s.v.n};return s.a!=7||s.v.n!=7;}',
 'array_composition': 'struct V{int n;V():n(7){}};struct S{int a[2];V v;};S f(int n){return {{n}};}int main(){S s=f(3);return s.a[0]!=3||s.a[1]||s.v.n!=7;}',
 'class_array': 'struct V{int n;V():n(7){}};struct S{int a;V v[2];};S f(int n){return {n};}int main(){S s=f(3);return s.a!=3||s.v[0].n!=7||s.v[1].n!=7;}',
 'nested_aggregate': 'struct V{int n;V():n(7){}};struct I{int a;V v;};struct S{int a;I i;};S f(){return {3};}int main(){S s=f();return s.a!=3||s.i.a||s.i.v.n!=7;}',
 'lambda_composition': 'struct V{int n;V():n(7){}};struct S{int a;V v;};int main(){auto f=[](int n)->S{return {n};};S a=f(3);return a.a!=3||a.v.n!=7;}',
 'references_preserved': 'struct V{int&n;V(int&n):n(n){}};struct S{int a;V v;};int main(){int n=7;S s=S{3,{n}};s.v.n=9;return n!=9||s.a!=3;}',
}
runner.BAD = {
 'explicit_omitted': 'struct V{explicit V(){}};struct S{int a;V v;};S f(){return {3};}',
 'private_omitted': 'class V{V(){}};struct S{int a;V v;};S f(){return {3};}',
 'narrow_class_list': 'struct V{V(int){}};struct S{int a;V v;};S f(){return {3,{1.5}};}',
}
if __name__ == '__main__':
    sys.exit(0 if runner.run(Path(sys.argv[1]).resolve(),Path(sys.argv[2])) else 1)
