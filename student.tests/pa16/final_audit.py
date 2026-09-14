#!/usr/bin/env python3
"""Independent whole-stage PA16 ownership probes; run explicitly."""
from pathlib import Path
import hashlib, json, os, subprocess, sys
ROOT = Path(__file__).resolve().parents[2]
CC = Path(sys.argv[1]).resolve() if len(sys.argv)>1 else ROOT/'dev/cppgm++'
WORK = Path(sys.argv[2]) if len(sys.argv)>2 else Path(os.environ['RALPH_ARTIFACT_DIR'])/'pa16-final-audit/controls'
WORK.mkdir(parents=True,exist_ok=True)
GOOD = {
 'query_reinterpret_noexcept': 'template<class T>constexpr bool f()noexcept(noexcept(reinterpret_cast<T*>(0))){return true;}static_assert(noexcept(f<int>()), "");int main(){return 0;}',
 'query_reinterpret_unselected': 'template<int N>struct A{char a[N];};static_assert(sizeof(A<true?7:(reinterpret_cast<int*>(0)==nullptr)>)==7, "");int main(){return 0;}',
 'query_nested_cast': 'constexpr int n=7;constexpr int const*p=&n;template<int N>struct A{char a[N];};static_assert(sizeof(A<*static_cast<int const*>(const_cast<int*>(p))>)==7, "");int main(){return 0;}',
 'query_pointer_cast': 'struct X{int n;constexpr int f()const{return n;}};constexpr X x={7};constexpr X const*p=&x;template<int N>struct A{char a[N];};static_assert(sizeof(A<static_cast<X const*>(p)->f()>)==7, "");int main(){return 0;}',
 'query_reference_cast': 'struct X{int n;};constexpr X x={7};template<int N>struct A{char a[N];};static_assert(sizeof(A<static_cast<X const&>(x).n>)==7, "");int main(){return 0;}',
 'reference_aggregate_temporary': 'struct X{int const&r;};constexpr X const&x={7};static_assert(x.r==7, "");int main(){return x.r-7;}',
 'architecture_trace': 'struct Record{unsigned n:3;double weight;constexpr Record(unsigned x):n(x),weight(x*0.5){}};constexpr Record const&sample=Record(9);template<class T>constexpr double step(T const&x)noexcept(sizeof(T)>1){return x.n+x.weight;}static_assert(noexcept(step(sample)), "");static_assert(step(sample)==5.5, "");int calls;int seed(){return ++calls;}double run(int i){static Record local(seed());constexpr double a[]={step(sample),step(sample)+1};constexpr double b[]={step(sample),step(sample)+1};return a!=b?a[i]+b[i]+local.n:0;}int main(){return run(0)==12&&run(1)==14&&calls==1?0:1;}',
 'class_reference_list': 'struct X{X const*p;constexpr X():p(this){}};constexpr X const&r={};static_assert(r.p==&r, "");int main(){return r.p==&r?0:1;}',
 'class_reference_conversion': 'struct X{X const*p;int n;constexpr X(int n):p(this),n(n){}};constexpr X const&r=7;static_assert(r.p==&r&&r.n==7, "");int main(){return r.p==&r&&r.n==7?0:1;}',
 'late_address_definition': 'extern const int n;constexpr int read(int const*p){return *p;}int early=read(&n);const int n=7;static_assert(read(&n)==7, "");int main(){return early==7?0:1;}',
 'late_pointer_definition': 'extern const int n;constexpr int const*p=&n;int early=*p;const int n=7;static_assert(*p==7, "");int main(){return early==7?0:1;}',
 'query_string_argument': 'constexpr int f(char const*p){return p[0];}template<int N>struct A{char x[N];};static_assert(sizeof(A<f("a")>)==97, "");int main(){return 0;}',
 'query_false_conversion': 'struct X{constexpr explicit operator bool()const{return false;}};constexpr X x={};template<int N>struct A{char a[N];};static_assert(sizeof(A<x?3:7>)==7, "");int main(){return 0;}',
 'query_free_operator': 'struct X{int n;};constexpr int operator+(X x,int y){return x.n+y;}constexpr X x={3};template<int N>struct A{char a[N];};static_assert(sizeof(A<x+4>)==7, "");int main(){return 0;}',
 'query_surrogate': 'constexpr int f(int n){return n+1;}typedef int(*F)(int);struct X{constexpr operator F()const{return f;}};constexpr X x={};template<int N>struct A{char a[N];};static_assert(sizeof(A<x(6)>)==7, "");int main(){return 0;}',
 'bitfield_layout': 'struct B{unsigned a:3;unsigned b:5;signed c:4;char d;unsigned e:6;};struct O{char pad;B b;B array[2];};constexpr O x={2,{9,35,-2,4,65},{{2,3,-1,5,6},{7,8,-4,9,10}}};static_assert(x.b.a==1&&x.b.b==3&&x.b.c==-2&&x.array[1].e==10, "");int main(){return x.pad==2&&x.b.a==1&&x.b.b==3&&x.b.c==-2&&x.b.d==4&&x.b.e==1&&x.array[0].c==-1&&x.array[1].e==10?0:1;}',
 'bitfield_template': 'template<class T>struct B{T n:3;constexpr B(T x):n(x){}};constexpr B<unsigned> b(9);template<int N>struct A{char a[N];};static_assert(sizeof(A<b.n>)==1, "");int main(){return b.n-1;}',
 'ordinary_aggregate_reference': 'struct X{int const&r;};X x={7};X y={7};int main(){return x.r==7&&y.r==7&&&x.r!=&y.r?0:1;}',
 'dynamic_aggregate_reference': 'int calls;int f(){return ++calls+6;}struct X{int const&r;};X x={f()};int main(){return x.r==7&&calls==1?0:1;}',
 'nested_aggregate_reference': 'struct X{int const&r;};struct Y{X x[2];};constexpr Y y={{{3},{7}}};static_assert(y.x[0].r==3&&y.x[1].r==7&&&y.x[0].r!=&y.x[1].r, "");int main(){return y.x[0].r+y.x[1].r-10;}',
 'class_reference_subobject': 'struct X{int n;constexpr X(int x):n(x){}};constexpr int const&r=X(7).n;static_assert(r==7, "");int main(){return r-7;}',
 'class_reference_self': 'struct X{X const*p;constexpr X():p(this){}};constexpr X const&r=X();static_assert(r.p==&r, "");int main(){return r.p==&r?0:1;}',
 'query_array': 'constexpr int a[]={3,7};template<int N>struct A{char x[N];};static_assert(sizeof(A<a[1]>)==7, "");int main(){return 0;}',
 'query_dereference': 'constexpr int n=7;constexpr int const*p=&n;template<int N>struct A{char x[N];};static_assert(sizeof(A<*p>)==7, "");int main(){return 0;}',
 'query_address_argument': 'constexpr int n=7;constexpr int f(int const*p){return *p;}template<int N>struct A{char x[N];};static_assert(sizeof(A<f(&n)>)==7, "");int main(){return 0;}',
 'query_unary_operator': 'struct X{int n;constexpr int operator-()const{return n;}};constexpr X x={7};template<int N>struct A{char a[N];};static_assert(sizeof(A<-x>)==7, "");int main(){return 0;}',
 'query_binary_operator': 'struct X{int n;constexpr int operator+(int x)const{return n+x;}};constexpr X x={3};template<int N>struct A{char a[N];};static_assert(sizeof(A<x+4>)==7, "");int main(){return 0;}',
 'query_functor': 'struct X{int n;constexpr int operator()(int x=4)const{return n+x;}};constexpr X x={3};template<int N>struct A{char a[N];};static_assert(sizeof(A<x()>)==7, "");int main(){return 0;}',
 'query_condition_conversion': 'struct X{constexpr explicit operator bool()const{return true;}};constexpr X x={};template<int N>struct A{char a[N];};static_assert(sizeof(A<x?7:3>)==7, "");int main(){return 0;}',
 'query_arrow_overload': 'struct X{int n;constexpr int f()const{return n;}};struct P{X x;constexpr X const*operator->()const{return &x;}};constexpr P p={{7}};template<int N>struct A{char a[N];};static_assert(sizeof(A<p->f()>)==7, "");int main(){return 0;}',
 'bitfield_aggregate': 'struct X{unsigned n:3;};constexpr X x={9};static_assert(x.n==1, "");int main(){X y={9};return x.n==y.n&&x.n==1?0:1;}',
 'bitfield_constructor': 'struct X{unsigned n:3;constexpr X(unsigned v):n(v){}constexpr unsigned get()const{return n;}};constexpr X x(9);static_assert(x.get()==1, "");int main(){volatile unsigned n=9;return X(n).get()==x.get()?0:1;}',
 'bitfield_dependency': 'struct X{unsigned n:3;unsigned m;constexpr X(unsigned v):n(v),m(n){} };constexpr X x(9);static_assert(x.n==1&&x.m==1, "");int main(){volatile unsigned n=9;return X(n).m==x.m?0:1;}',
 'static_class_reference': 'struct X{int n;constexpr X(int x):n(x){}};constexpr X const&r=X(7);static_assert(r.n==7, "");int main(){return r.n-7;}',
 'aggregate_reference_temporary': 'struct X{int const&r;};constexpr X x={7};static_assert(x.r==7, "");int main(){return x.r-7;}',
 'query_arrow': 'struct X{int n;constexpr int f()const{return n;}};constexpr X x={7};constexpr X const*p=&x;template<int N>struct A{char a[N];};static_assert(sizeof(A<p->f()>)==7, "");int main(){return 0;}',
 'query_inherited': 'struct B{int n;constexpr int f()const{return n;}};struct D:B{constexpr D():B{7}{}};constexpr D d;template<int N>struct A{char a[N];};static_assert(sizeof(A<d.f()>)==7, "");int main(){return 0;}',
 'query_default_reference': 'constexpr int f(int const&x=7){return x;}template<int N>struct A{char a[N];};static_assert(sizeof(A<f()>)==7, "");int main(){return 0;}',
 'query_function_pointer': 'constexpr int f(int n){return n+1;}constexpr int(*p)(int)=f;template<int N>struct A{char a[N];};static_assert(sizeof(A<p(6)>)==7, "");int main(){return 0;}',
 'nested_constructor_reference': 'struct I{int n;constexpr I(int x):n(x){}};struct O{I i;int const&r;constexpr O(int x):i(x),r(i.n){}};constexpr O o(7);static_assert(&o.r==&o.i.n&&o.r==7, "");int main(){return &o.r==&o.i.n&&o.r==7?0:1;}',
 'constructor_base_reference': 'struct B{int n;constexpr B(int x):n(x){}};struct D:B{int const&r;constexpr D():B(7),r(n){}};constexpr D d;static_assert(&d.r==&d.n&&d.r==7, "");int main(){return &d.r==&d.n&&d.r==7?0:1;}',
 'recursive_objects': 'struct X{int n;constexpr X(int n):n(n){}};constexpr X f(int n){return n?X(f(n-1).n+1):X(0);}static_assert(f(30).n==30, "");int main(){return f(30).n-30;}',
 'construction_mutable_read': 'struct X{mutable int n;int m;constexpr X(int x):n(x),m(n){}};constexpr X x(7);static_assert(x.m==7, "");int main(){return x.m-7;}',
}
BAD = {
 'query_reinterpret_required': 'template<int N>struct A{char a[N];};A<reinterpret_cast<int*>(0)==nullptr> x;',
 'query_void_pointer_cast': 'constexpr int n=7;constexpr void const*p=&n;template<int N>struct A{char a[N];};A<*static_cast<int const*>(p)> x;',
 'reinterpret_array': 'int n;constexpr char const*p[]={reinterpret_cast<char const*>(&n)};',
 'void_pointer_cast': 'constexpr int n=7;constexpr void const*p=&n;constexpr int const*q=static_cast<int const*>(p);',
 'reinterpret_null': 'constexpr int*p=reinterpret_cast<int*>(0);',
 'reinterpret_address': 'constexpr int n=7;constexpr char const*p=reinterpret_cast<char const*>(&n);',
 'function_reference_dangling': 'struct X{int n;constexpr X(int x):n(x){}};constexpr X const&id(X const&x){return x;}constexpr X const&r=id(X(7));',
 'inactive_union_address_read': 'union X{int a;double b;};constexpr X x={7};constexpr double const*p=&x.b;static_assert(*p==0, "");',
 'expired_parameter': 'constexpr int const*f(int x){return &x;}static_assert(f(1)==f(1), "");',
 'mutable_reference_read': 'struct X{mutable int n;};constexpr X x={7};constexpr int const&r=x.n;static_assert(r==7, "");',
}
rows=[]
for name,source in {**GOOD,**BAD}.items():
 src=WORK/(name+'.cpp');src.write_text(source);ir=src.with_suffix('.lowir');exe=src.with_suffix('.exe')
 row=dict(name=name,source=source,expected_success=name in GOOD)
 try:
  p=subprocess.run([CC,'--emit-lowir','-O0','--validate-lowir','--stats','-o',ir,src],capture_output=True,text=True,timeout=30)
  row.update(compile_exit=p.returncode,stderr=p.stderr);ok=(p.returncode==0)==(name in GOOD)
  if not p.returncode and name in GOOD:
   row['lowir_sha256']=hashlib.sha256(ir.read_bytes()).hexdigest()
   p=subprocess.run([ROOT/'dev/lowir2native-ref','-O0','-o',exe,ir],capture_output=True,text=True,timeout=30)
   row.update(backend_exit=p.returncode,backend_stderr=p.stderr);ok &= p.returncode==0
   if not p.returncode:row['native_exit']=subprocess.run([exe],timeout=15).returncode;ok &= row['native_exit']==0
 except subprocess.TimeoutExpired:row['timeout']=True;ok=False
 row['passed']=ok;rows.append(row);print(name,'PASS' if ok else 'FAIL',flush=True)
(WORK/'results.json').write_text(json.dumps(dict(compiler_sha256=hashlib.sha256(CC.read_bytes()).hexdigest(),rows=rows),indent=2)+'\n')
assert all(r['passed'] for r in rows),[r['name'] for r in rows if not r['passed']]
