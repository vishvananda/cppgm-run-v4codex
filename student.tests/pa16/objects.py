#!/usr/bin/env python3
"""Explicit PA16 object/address controls, including native consumers."""
from pathlib import Path
import os,subprocess,json,sys
ROOT=Path(__file__).resolve().parents[2]
CC=Path(sys.argv[1]).resolve() if len(sys.argv)>1 else ROOT/'dev/cppgm++'
WORK=Path(sys.argv[2]) if len(sys.argv)>2 else Path(os.environ['RALPH_ARTIFACT_DIR'])/'pa16-object/controls'
WORK.mkdir(parents=True,exist_ok=True)
GOOD={
'cache_mutation': 'constexpr int read(int const&x){return x;}constexpr int f(){int x=1;int a=read(x);x=2;return a+read(x);}static_assert(f()==3, "");int main(){return f()-3;}',
'self_address': 'struct S{S const*p;constexpr S():p(this){}};constexpr S s;static_assert(s.p==&s, "");int main(){return s.p==&s?0:1;}',
'known_receiver': 'struct P{constexpr int f()const{return 1;}};P p;static_assert(p.f()==1, "");int main(){return p.f()-1;}',

'fields': 'struct V{int x,y;constexpr V(int a,int b):x(a),y(b){}constexpr int sum()const{return x+y;}};constexpr V v(3,8);static_assert(v.sum()==11, "");int main(){return v.sum()!=11;}',
'aggregate': 'struct P{int a[3];double b;};constexpr P p={{2,3,5},0.5};static_assert(p.a[2]==5&&p.b==0.5, "");int main(){return p.a[1]!=3;}',
'identity': 'int x,y;constexpr bool same(int const&a,int const&b){return &a==&b;}static_assert(same(x,x)&&!same(x,y), "");int main(){return same(x,x)&&!same(x,y)?0:1;}',
'reference_read': 'constexpr int read(int const&a){return a;}static_assert(read(13)==13, "");int main(){return read(13)-13;}',
'pointer_loop': 'constexpr int a[]={0,0,9};constexpr int f(int const*p,int const*q){int i=0;while(p!=q&&!*p){++p;++i;}return i;}static_assert(f(a,a+3)==2, "");int main(){return f(a,a+3)-2;}',
'function_address': 'constexpr int f(int x){return x+3;}constexpr int g(int(*p)(int),int x){return p(x);}static_assert(g(f,4)==7, "");int main(){return g(f,4)-7;}',
'function_reference': 'constexpr int f(int x){return x+3;}constexpr int g(int(&p)(int),int x){return p(x);}static_assert(g(f,4)==7, "");int main(){return g(f,4)-7;}',
'copy_identity': 'struct X{int n;};constexpr bool f(X a,X const&b){return &a!=&b&&a.n==b.n;}constexpr X x={7};static_assert(f(x,x), "");int main(){return f(x,x)?0:1;}',
'base_paths': 'struct B{int x;constexpr B(int n):x(n){}constexpr int get()const{return x;}};struct L:B{constexpr L():B(3){}};struct R:B{constexpr R():B(9){}};struct D:L,R{constexpr D():L(),R(){}};static_assert(static_cast<L const&>(D()).get()==3&&static_cast<R const&>(D()).get()==9, "");int main(){D d;return static_cast<L const&>(d).get()+static_cast<R const&>(d).get()-12;}',
'constructor_dependency': 'struct C{int x,y;constexpr C(int n):x(n),y(x+2){}};static_assert(C(5).y==7, "");int main(){return C(5).y-7;}',
'reference_member': 'struct R{int const&r;constexpr R(int const&x):r(x){}};constexpr int x=7;constexpr R r(x);static_assert(r.r==7&&&r.r==&x, "");int main(){return r.r-7;}',
'local_scope': 'constexpr int f(){int n=4;{int n=7;}return n;}static_assert(f()==4, "");int main(){return f()-4;}',
'one_object_bounds': 'constexpr int x=9;constexpr int const*p=&x;static_assert((p+1)-1==p, "");int main(){return 0;}',
'operator_conversion': (ROOT/'student.tests/pa16/objects/conversions.cpp').read_text(),
}
BAD={
'bad_member': 'int g();struct P{int x,y;};constexpr P p={1,g()};',
'bad_temporary': 'constexpr int const*f(int const&x){return &x;}constexpr int const*p=f(9);',
'bad_read': 'int x=7;constexpr int f(int const&x){return x;}static_assert(f(x)==7, "");',
'bad_bound': 'constexpr int x[]={1,2};constexpr auto p=x+3;',
'bad_onepast_read': 'constexpr int x[]={1,2};static_assert(*(x+2)==0, "");',
'bad_dead_member': 'struct P{int const*p;};constexpr P f(){int n=9;return P{&n};}static_assert(f().p!=0, "");',
'bad_mutable': 'struct P{mutable int n;};constexpr P p={4};static_assert(p.n==4, "");',
'bad_local_lifetime': 'constexpr int f(){int const*p=0;{int n=7;p=&n;}return *p;}static_assert(f()==7, "");',

}
rows=[]
for name,source in {**GOOD,**BAD}.items():
 src=WORK/(name+'.cpp');src.write_text(source);ir=src.with_suffix('.lowir');exe=src.with_suffix('.exe')
 try:
  r=subprocess.run([CC,'--emit-lowir','-O0','--validate-lowir','-o',ir,src],capture_output=True,text=True,timeout=30)
  row=dict(name=name,exit=r.returncode,stderr=r.stderr,expected=name in GOOD)
  ok=(r.returncode==0)==(name in GOOD)
  if ok and name in GOOD:
   r=subprocess.run([ROOT/'dev/lowir2native-ref','-O0','-o',exe,ir],capture_output=True,text=True,timeout=30)
   row['backend_exit']=r.returncode;row['backend_stderr']=r.stderr;ok=r.returncode==0
   if ok:
    r=subprocess.run([exe],capture_output=True,text=True,timeout=10);row['native_exit']=r.returncode;ok=r.returncode==0
 except subprocess.TimeoutExpired:row=dict(name=name,error='timeout');ok=False
 row['passed']=ok;rows.append(row);print(name,'PASS' if ok else 'FAIL',flush=True)
(WORK/'results.json').write_text(json.dumps(rows,indent=2)+'\n')
assert all(r['passed'] for r in rows),[r['name'] for r in rows if not r['passed']]
