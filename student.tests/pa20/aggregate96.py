#!/usr/bin/env python3
"""Aggregate member construction, representation and identity. Run CC WORK."""
from pathlib import Path
import sys
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'pa18'))
import ordering_controls as runner
runner.GOOD = {
 'array_member_enum':'enum E{a,b,c};struct S{E x[4];int n;};int main(){S s[]={{{b},1},{{c,b},2},{}};return s[0].x[0]!=b||s[0].x[3]!=a||s[1].x[1]!=b||s[2].n;}',
 'array_member_mixed':'struct S{int a;long x[3];int b;};int main(){S s[]={ {2,{3,4},5},{} };return s[0].a!=2||s[0].x[2]||s[0].b!=5||s[1].a||s[1].x[0]||s[1].b;}',
 'array_member_string':'struct S{char s[5];int n;};int main(){S a[]={{"abc",4},{}};return a[0].s[2]!=\'c\'||a[0].s[4]||a[1].s[0]||a[0].n!=4;}',
 'array_member_nested':'struct S{int x[2][2];int n;};int main(){S a[]={{{{1,2},{3}},4},{}};return a[0].x[1][0]!=3||a[0].x[1][1]||a[1].x[0][0]||a[0].n!=4;}',
 'array_member_volatile':'struct S{volatile int x[2];int n;};int main(){S a[]={{{3},4},{}};return a[0].x[0]!=3||a[0].x[1]||a[1].n;}',
 'array_member_large_tail':'struct S{int a[64];int n;};int main(){S a[]={{{3},4},{}};return a[0].a[0]!=3||a[0].a[63]||a[1].a[63]||a[0].n!=4;}',
 'array_member_list_result':'struct S{int x[3];int n;};S f(int n){return {{n,n+1},n+2};}int main(){S s=f(3);return s.x[0]!=3||s.x[1]!=4||s.x[2]||s.n!=5;}',
 'array_member_order':'int n;int f(){return ++n;}struct S{int a[2];int n;};int main(){S s[]={{{f(),f()},f()},{{f()},f()}};return n!=5||s[0].a[0]!=1||s[0].a[1]!=2||s[0].n!=3||s[1].n!=5;}',
 'array_member_template':'template<class T>struct S{T a[3];int n;};template<class T>S<T>f(T n){return {{n},4};}int main(){S<long>s=f(3L);return s.a[0]!=3||s.a[2]||s.n!=4;}',
 'omitted_identity':'struct V{V*p;V():p(this){}~V(){}};struct S{int n;V v;};S f(int n){return {n};}int main(){S s=f(3);return s.n!=3||s.v.p!=&s.v;}',
 'copy_member_identity':'int copies,moves;struct V{V*p;int n;V(int n):p(this),n(n){}V(const V&o):p(this),n(o.n){++copies;}V(V&&o):p(this),n(o.n){++moves;}};struct S{int n;V a;V b;};S f(V&a,V&b){return {1,a,b};}int main(){V a(3),b(4);S s=f(a,b);return copies!=2||moves||s.a.p!=&s.a||s.b.p!=&s.b||s.a.n!=3||s.b.n!=4;}',
 'class_member_prvalue_order':'int log;struct V{int n;V(int n):n(n){log=log*10+n;}V(V&&o):n(o.n){log=log*10+9;}};struct S{V a;V b;};int main(){S s=S{V(1),V(2)};return log!=12&&log!=192&&log!=129&&log!=1929;}',
}
runner.GOOD['scalar_holes']='struct S{int a;int b;int c;};S f(int x){return {{},x,{}};}S g(int x){return {x};}int main(){S a=f(3),b=g(4);return a.a||a.b!=3||a.c||b.a!=4||b.b||b.c;}'
runner.BAD={
 'array_member_excess':'struct S{int a[2];};int main(){S s[]={{{1,2,3}}};}',
 'array_member_narrow':'struct S{int a[2];};int main(){S s[]={{{1,2.5}}};}',
 'class_member_deleted_copy':'struct V{V(){}V(const V&)=delete;};struct S{V v;};S f(V&v){return {v};}int main(){}',
}
if __name__=='__main__':sys.exit(0 if runner.run(Path(sys.argv[1]).resolve(),Path(sys.argv[2])) else 1)
