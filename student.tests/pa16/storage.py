#!/usr/bin/env python3
"""Explicit PA16 persistent storage and constant-data identity controls."""
from pathlib import Path
import subprocess,sys,tempfile
ROOT=Path(__file__).resolve().parents[2]
CC=Path(sys.argv[1]).resolve() if len(sys.argv)>1 else ROOT/'dev/cppgm++'
GOOD={
 'scalar_first_use':'int n;int next(){return ++n;}int& value(){static int x=next();return x;}int main(){if(n)return 1;if(value()!=1)return 2;value()=9;return value()!=9 || n!=1;}',
 'static_parameter':'int f(int n){static int x=n;return x++;}int main(){return f(4)!=4 || f(8)!=5;}',
 'static_array':'int n;int get(){return ++n;}int f(){static int x[3]={get(),get(),get()};return ++x[1];}int main(){return f()!=3 || f()!=4 || n!=3;}',
 'static_class_factory':'int calls;struct A{long x,y,z;};A make(int v){++calls;return A{v,2,v+3};}long f(int v){static A a=make(v);return a.x+a.z;}int main(){return f(4)!=11 || f(9)!=11 || calls!=1;}',
 'static_reference':'int x=4;int& f(){static int& r=x;return r;}int main(){f()=7;return x!=7 || &f()!=&x;}',
 'static_dynamic_reference':'int n;int x=3;int& get(){++n;return x;}int& f(){static int& r=get();return r;}int main(){f()=6;return f()!=6 || n!=1 || x!=6;}',
 'static_reference_parameter':'int& f(int& x){static int& r=x;return r;}int main(){int a=3,b=4;f(a)=9;return &f(b)!=&a || a!=9 || b!=4;}',
 'static_pointer_parameter':'int* f(int x){static int*p=&x;return p;}int main(){return 0;}',
 'static_scalar_temporary':'int calls;int get(){++calls;return 7;}const int& f(){static const int& r=get();return r;}int main(){const int*p=&f();return f()!=7 || &f()!=p || calls!=1;}',
 'static_class_reference':'struct A{int x;};A& get(){static A a={7};return a;}int f(){static A&r=get();return ++r.x;}int main(){return f()!=8 || f()!=9;}',
 'nested_scopes':'int f(bool b){if(b){static int x=4;return ++x;}static int x=8;return ++x;}int main(){return f(true)!=5 || f(false)!=9 || f(true)!=6 || f(false)!=10;}',
 'template_identity':'template<class T>int f(){static int a[2]={1,2};return ++a[0];}int main(){return f<int>()!=2 || f<char>()!=2 || f<int>()!=3;}',
 'overload_identity':'int f(int){static int x=1;return ++x;}int f(char){static int x=5;return ++x;}int main(){return f(1)!=2 || f(char(1))!=6 || f(2)!=3;}',
 'array_large_copy':'int f(int i){constexpr int a[40]={1,2,3};constexpr int b[40]={1,2,3};return a!=b && a[i]==b[i] ? a[i] : -1;}int main(){return f(1)!=2 || f(39)!=0;}',
 'array_plain_copy':'int f(int i){int a[2]={1,2};int b[2]={1,2};a[0]=7;return a!=b ? a[i]+b[i] : -1;}int main(){return f(0)!=8 || f(1)!=4;}',
 'array_float_signed_zero':'int f(int i){constexpr double a[]={0.0,-0.0};constexpr double b[]={0.0,0.0};return i ? 1.0/a[1]<0.0 : 1.0/b[1]>0.0;}int main(){return !f(0) || !f(1);}',
 'array_pointer_addends':'int a[3]={2,4,6};int f(int i){constexpr int* p[]={a,a+1};constexpr int* q[]={a,a+2};return *p[i]+*q[i];}int main(){return f(1)!=10;}',
 # Source try/throw lowering starts in PA19; retain the intended future control.
 # 'static_throw_retry':'int calls;int get(){if(++calls==1)throw 1;return 8;}int f(){static int x=get();return x;}int main(){try{f();return 1;}catch(int){}return f()!=8 || f()!=8 || calls!=2;}',
}
# The supplied freestanding backend has no libc atexit. This test runtime
# records the real emitted registrations and drains callbacks before main exits.
# The observer is registered first, so it verifies reverse destruction order.
GOOD.update({
 'static_destructor':'extern "C" int atexit(void(*)());int n;void check(){if(n!=1)__builtin_abort();}struct A{~A(){++n;}};void f(){static A a;}int main(){atexit(check);f();f();return n;}',
 'static_destructor_order':'extern "C" int atexit(void(*)());int n;void check(){if(n!=21)__builtin_abort();}struct A{int x;A(int i):x(i){}~A(){n=n*10+x;}};void f(){static A a(1);}void g(){static A a(2);}int main(){atexit(check);f();g();f();return n;}',
 'static_class_temporary':'extern "C" int atexit(void(*)());int n;void check(){if(n!=1)__builtin_abort();}struct A{int x;A(int i):x(i){}~A(){++n;}};int f(){static const A& a=A(9);return a.x;}int main(){atexit(check);return f()!=9 || f()!=9 || n;}',
 'static_destructor_array':'extern "C" int atexit(void(*)());int n;void check(){if(n!=321)__builtin_abort();}struct A{int x;A(int i):x(i){}~A(){n=n*10+x;}};void f(){static A a[3]={1,2,3};}int main(){atexit(check);f();f();return n;}',
 'static_conditional_temporary':'extern "C" int atexit(void(*)());int n;void check(){if(n!=2)__builtin_abort();}struct A{int x;A(int i):x(i){}~A(){n+=x;}};int f(bool b){static const A& a=b?A(2):A(7);return a.x;}int main(){atexit(check);return f(true)!=2 || f(false)!=2 || n;}',
 'static_zero_aggregate':'struct A{int x,y;};A& f(){static A a;return a;}int main(){if(f().x || f().y)return 1;f().x=5;return f().x!=5;}',
 'static_never_reached':'extern "C" int atexit(void(*)());int n;void check(){if(n)__builtin_abort();}struct A{~A(){++n;}};void f(bool b){if(b){static A a;}}int main(){atexit(check);f(false);return n;}',
})
RUNTIME='void(*callbacks[32])();int count;extern "C" int atexit(void(*f)()){callbacks[count++]=f;return 0;}int finish(int result){while(count)callbacks[--count]();return result;}'
for name in list(GOOD):
 if 'atexit' in GOOD[name]:
  source=GOOD[name];at=source.index('int main()');prefix,main=source[:at],source[at:]
  at=main.rindex('return ');end=main.index(';',at)
  main=main[:at]+'return finish('+main[at+7:end]+')'+main[end:]
  GOOD[name]=RUNTIME+prefix+main
failed=[]
with tempfile.TemporaryDirectory(prefix='pa16-storage-') as td:
 for name,source in GOOD.items():
  src=Path(td)/(name+'.cpp');ir=src.with_suffix('.lowir');exe=src.with_suffix('.exe');src.write_text(source)
  r=subprocess.run([CC,'--emit-lowir','-O0','--validate-lowir','-o',ir,src],capture_output=True,text=True,timeout=30)
  okay=r.returncode==0
  if okay and name=='array_large_copy':
   text=ir.read_text();okay=text.count('copyobj')==2 and text.count('storage=readonly')==1
  if okay:
   r=subprocess.run([ROOT/'dev/lowir2native-ref','-O0','-o',exe,ir],capture_output=True,text=True);okay=r.returncode==0
   if okay:r=subprocess.run([exe],capture_output=True,text=True,timeout=10);okay=r.returncode==0
  print(name,'PASS' if okay else 'FAIL',r.returncode,r.stderr.strip(),flush=True)
  if not okay:failed.append(name)
assert not failed,failed
print(f'{len(GOOD)} native storage controls passed')
