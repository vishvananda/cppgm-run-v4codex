#!/usr/bin/env python3
"""Explicit member-pointer constexpr/value/storage controls."""
from pathlib import Path
import hashlib,json,subprocess,sys
ROOT=Path(__file__).resolve().parents[2];CC=Path(sys.argv[1]).resolve();WORK=Path(sys.argv[2]);WORK.mkdir(parents=True,exist_ok=True)
GOOD={
'pending_array': (ROOT/'student.tests/pa16/initialization/member_pointer_pending.cpp').read_text(),
'data_projection': 'struct X{int a,b;};constexpr X x={2,7};constexpr int X::*p=&X::b;static_assert(x.*p==7,"dot");static_assert((&x)->*p==7,"arrow");constexpr int const&q=x.*p;static_assert(&q==&x.b,"identity");int main(){return q-7;}',
'data_call': 'struct X{int n;};constexpr int read(X const&x,int X::*p){return x.*p;}constexpr X x={9};static_assert(read(x,&X::n)==9,"");int main(){return read(x,&X::n)-9;}',
'data_bool_null': 'struct X{int a,b;};constexpr int X::*a=&X::a;constexpr int X::*b=&X::b;constexpr int X::*z=nullptr;static_assert(a&&b&&!z&&a!=b&&a==&X::a&&z==nullptr,"");int main(){return a&&b&&!z&&a!=b&&z==nullptr?0:1;}',
'data_omitted': 'struct X{int a,b;};constexpr int X::*p[40]={&X::a,&X::b};static_assert(p[39]==nullptr&&p[0]!=nullptr,"");int main(){int X::*q[40]={&X::a,&X::b};X x={3,7};return x.*q[1]==7&&q[39]==nullptr?0:1;}',
'data_distinct': 'struct X{int a,b;};int main(){int X::*p[]={&X::a,&X::b};int X::*q[]={&X::a,&X::b};p[0]=&X::b;X x={3,7};return x.*q[0]==3&&x.*p[0]==7?0:1;}',
'data_nested': 'struct X{int a,b;};struct P{int X::*p[2];};constexpr P p={{&X::b}};constexpr X x={2,7};static_assert(x.*p.p[0]==7&&p.p[1]==nullptr,"");int main(){return x.*p.p[0]-7;}',
'data_base_receiver': 'struct B{int n;constexpr B(int n):n(n){}};struct P{int pad;constexpr P():pad(4){}};struct D:P,B{constexpr D():P(),B(7){}};constexpr D d;constexpr int B::*p=&B::n;static_assert(d.*p==7,"");int main(){return d.*p-7;}',
'data_union_equal': 'union U{int a,b;};constexpr int U::*a=&U::a;constexpr int U::*b=&U::b;static_assert(a==b,"");int main(){return a==b?0:1;}',
'data_qualification': 'struct X{int n;};constexpr const int X::*p=&X::n;constexpr X x={8};static_assert(x.*p==8,"");int main(){return x.*p-8;}',
'function_call': 'struct X{int n;constexpr int f(int k)const{return n+k;}constexpr int g(int k)const{return n-k;}};constexpr X x={7};constexpr int(X::*p)(int)const=&X::f;static_assert((x.*p)(3)==10,"");static_assert(((&x)->*p)(2)==9,"");int main(){return (x.*p)(3)-10;}',
'function_parameter': 'struct X{int n;constexpr int f(int k)const{return n+k;}};constexpr int call(X const&x,int(X::*p)(int)const,int n){return (x.*p)(n);}constexpr X x={7};static_assert(call(x,&X::f,3)==10,"");int main(){return call(x,&X::f,3)-10;}',
'function_array': 'struct X{int n;constexpr int f(int k)const{return n+k;}constexpr int g(int k)const{return n-k;}};constexpr int(X::*p[12])(int)const={&X::f,&X::g};constexpr X x={7};static_assert((x.*p[0])(3)==10&&(x.*p[1])(2)==5&&!p[11],"");int main(){int(X::*q[12])(int)const={&X::f,&X::g};return (x.*q[1])(2)==5&&!q[11]?0:1;}',
'function_equality': 'struct X{int f()const{return 1;}int g()const{return 2;}};constexpr int(X::*f)()const=&X::f;constexpr int(X::*g)()const=&X::g;constexpr int(X::*z)()const=nullptr;static_assert(f!=g&&f==&X::f&&f&&g&&!z,"");int main(){return f!=g&&f==&X::f&&f&&g&&!z&&z==nullptr?0:1;}',
'function_overload': 'struct X{constexpr int f(int k)const{return k+1;}constexpr int f()const{return 2;}};constexpr int(X::*p)(int)const=&X::f;constexpr X x={};static_assert((x.*p)(3)==4,"");int main(){return (x.*p)(3)-4;}',
'function_nested': 'struct X{constexpr int f()const{return 9;}};struct P{int(X::*p)()const;};constexpr P p={&X::f};constexpr X x={};static_assert((x.*p.p)()==9,"");int main(){return (x.*p.p)()-9;}',
'function_local_static': 'struct X{int f()const{return 9;}};int call(){static int(X::*p)()const=&X::f;X x;return (x.*p)();}int main(){return call()-9;}',
'zero_namespace': 'struct X{int n;int f()const{return n;}};int X::*p;int(X::*q)()const;int main(){return p==nullptr&&q==nullptr?0:1;}',
}
GOOD.update({
'value_init': 'struct X{int n;int f()const{return n;}};typedef int X::*P;typedef int(X::*F)()const;constexpr P p=P();constexpr F f=F();static_assert(!p&&!f, "");int main(){P q{};F g{};return !q&&!g&&!P()&&!F()?0:1;}',
'conditions': 'struct X{int n;int f()const{return n;}};int main(){int X::*p=&X::n;int(X::*f)()const=&X::f;int n=0;if(p)++n;if(f)++n;if(int X::*q=&X::n)++n;return n==3&&(p?true:false)&&(f?true:false)?0:1;}',
'conditional_null': 'struct X{int n;};constexpr int X::*pick(bool b){return b?&X::n:nullptr;}static_assert(pick(true)&&!pick(false), "");int main(){X x={7};return x.*pick(true)-7;}',
'reference_parameter': 'struct X{int a,b;};constexpr int read(X const&x,int X::*const&p){return x.*p;}constexpr X x={2,7};constexpr int X::*p=&X::b;static_assert(read(x,p)==7, "");int main(){return read(x,p)-7;}',
'memoization': 'struct X{int a,b;};constexpr int read(X const&x,int X::*p){return x.*p;}constexpr X x={2,7};static_assert(read(x,&X::a)+read(x,&X::b)==9, "");int main(){return 0;}',
'local_mutation': 'struct X{int a,b;};constexpr int read(){X x={2,7};int X::*p=&X::a;int a=x.*p;p=&X::b;return a+x.*p;}static_assert(read()==9, "");int main(){return read()-9;}',
'array_local_constexpr': 'struct X{int a,b;};constexpr int read(int i){constexpr int X::*p[]={&X::a,&X::b};X x={2,7};return x.*p[i];}static_assert(read(0)+read(1)==9, "");int main(){return read(0)+read(1)-9;}',
'computed_template_arg': 'struct X{int n;};constexpr int read(X const&x,int X::*p){return x.*p;}constexpr X x={7};constexpr int X::*p=&X::n;template<int N>struct A{char data[N];};static_assert(sizeof(A<read(x,p)>)==7, "");int main(){return 0;}',
'static_template_members': 'struct X{int a,b;};template<class T>struct P{static constexpr int X::*p[2]={&X::a,&X::b};};template<class T>constexpr int X::*P<T>::p[2];constexpr X x={2,7};static_assert(x.*P<int>::p[1]==7, "");int main(){return x.*P<int>::p[1]-7;}',
})
GOOD.update({
'function_reference': 'struct X{constexpr int f()const{return 9;}};typedef int(X::*F)()const;constexpr F const&r=&X::f;constexpr X x={};static_assert((x.*r)()==9, "");int main(){return (x.*r)()-9;}',
'function_return': 'struct X{constexpr int f()const{return 9;}};typedef int(X::*F)()const;constexpr F get(){return &X::f;}constexpr X x={};static_assert((x.*get())()==9, "");int main(){return (x.*get())()-9;}',
'user_conversion': 'struct X{int n;};typedef int X::*P;struct Y{constexpr operator P()const{return &X::n;}};constexpr P p=Y();constexpr X x={7};static_assert(x.*p==7, "");int main(){return x.*p-7;}',
'function_template_members': 'struct X{constexpr int f()const{return 9;}};template<class T>struct P{static constexpr int(X::*p[2])()const={&X::f};};template<class T>constexpr int(X::*P<T>::p[2])()const;constexpr X x={};static_assert((x.*P<int>::p[0])()==9&&!P<int>::p[1], "");int main(){return (x.*P<int>::p[0])()-9;}',
'computed_template_address': 'struct X{int n;};constexpr int read(X const&x,int X::*p){return x.*p;}constexpr X x={7};template<int N>struct A{char data[N];};static_assert(sizeof(A<read(x,&X::n)>)==7, "");int main(){return 0;}',
})
GOOD.update({
'scalar_reference_storage': 'constexpr int const&r=7;constexpr int const&s=7;constexpr int const&alias=r;static_assert(&r!=&s&&&alias==&r&&r+s==14, "");int main(){return &r!=&s&&&alias==&r&&r+s==14?0:1;}',
'unused_template_storage': 'struct X{constexpr int f()const;};template<class T>struct P{static constexpr int(X::*p[2])()const={&X::f};};int main(){return 0;}',
})
BAD={
'null_projection': 'struct X{int n;};constexpr X x={3};constexpr int X::*p=nullptr;static_assert(x.*p==0,"");',
'null_call': 'struct X{constexpr int f()const{return 1;}};constexpr X x={};constexpr int(X::*p)()const=nullptr;static_assert((x.*p)()==0,"");',
'nonconstexpr_call': 'struct X{int f()const{return 1;}};constexpr X x={};constexpr int(X::*p)()const=&X::f;static_assert((x.*p)()==1,"");',
'mutable_read': 'struct X{mutable int n;};constexpr X x={3};constexpr int X::*p=&X::n;static_assert(x.*p==3,"");',
'volatile_read': 'struct X{volatile int n;};X x={3};constexpr int volatile X::*p=&X::n;static_assert(x.*p==3,"");',
'inactive_union': 'union U{int a;double b;};constexpr U u={3};constexpr double U::*p=&U::b;static_assert(u.*p==0,"");',
}
rows=[]
for name,source in {**GOOD,**BAD}.items():
 src=WORK/(name+'.cpp');src.write_text(source);ir=src.with_suffix('.lowir');exe=src.with_suffix('.exe')
 p=subprocess.run([CC,'--emit-lowir','-O0','--validate-lowir','-o',ir,src],capture_output=True,text=True,timeout=30)
 row=dict(name=name,source=source,compile_exit=p.returncode,stderr=p.stderr,expected=name in GOOD);ok=(p.returncode==0)==(name in GOOD)
 if not p.returncode and name in GOOD:
  row['lowir_sha256']=hashlib.sha256(ir.read_bytes()).hexdigest()
  p=subprocess.run([ROOT/'dev/lowir2native-ref','-O0','-o',exe,ir],capture_output=True,text=True,timeout=30)
  row['backend_exit']=p.returncode;row['backend_stderr']=p.stderr;ok &= p.returncode==0
  if not p.returncode:row['native_exit']=subprocess.run([exe],timeout=10).returncode;ok &= row['native_exit']==0
 row['passed']=ok;rows.append(row);print(name,'PASS' if ok else 'FAIL',row.get('stderr') if not ok else '',flush=True)
(WORK/'results.json').write_text(json.dumps(dict(compiler_sha256=hashlib.sha256(CC.read_bytes()).hexdigest(),rows=rows),indent=2)+'\n')
assert all(r['passed'] for r in rows),[r['name'] for r in rows if not r['passed']]
