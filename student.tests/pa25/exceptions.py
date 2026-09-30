#!/usr/bin/env python3
"""Explicit source EH ownership, matching and control checks; no host oracle."""
import json, os, pathlib, subprocess, sys
ROOT=pathlib.Path(__file__).resolve().parents[2]
CXX=pathlib.Path(os.environ.get('CPPGM_TEST_CXX',ROOT/'dev/cppgm++')).resolve()
OUT=pathlib.Path(sys.argv[1] if len(sys.argv)>1 else ROOT/'artifacts/pa25-140/exceptions'); OUT.mkdir(parents=True,exist_ok=True)
cases={
'scalar-order':'''int main(){try{throw 9;}catch(double){return 1;}catch(int x){return x!=9;}return 2;}''',
'nested-rethrow':'''int dead;struct E{int n;E(int x):n(x){}~E(){++dead;}};
int main(){try{try{throw E(7);}catch(E& a){try{throw;}catch(E& b){if(&a!=&b)return 1;}if(dead)return 2;throw;}}
catch(E& c){if(c.n!=7 || dead)return 3;}return dead!=1;}''',
'nested-new':'''int dead,copy;struct E{int n;E(int x):n(x){}E(const E& e):n(e.n){++copy;}~E(){dead=dead*10+n;}};
int main(){try{try{throw E(1);}catch(E a){try{throw E(2);}catch(E& b){if(b.n!=2)return 1;}throw E(3);}}
catch(E c){if(c.n!=3 || dead!=211)return 2;}return copy!=2 || dead!=21133;}''',
'cleanup-nested-catch':'''int cleaned;struct G{~G(){try{throw 3;}catch(int x){cleaned=x;}}};
int main(){try{G g;throw 7;}catch(int x){return x!=7 || cleaned!=3;}return 2;}''',
'base-offset':'''struct A{int a;A():a(1){}};struct B{int b;B():b(7){}};struct D:A,B{};
int main(){try{throw D();}catch(B& b){return b.b!=7;}catch(...){return 2;}return 3;}''',
'base-access':'''struct A{};struct L:A{};struct R:A{};struct D:L,R{};struct P:private A{};
int main(){int n=0;try{throw D();}catch(A&){return 1;}catch(...){++n;}
try{throw P();}catch(A&){return 2;}catch(...){++n;}return n!=2;}''',
'virtual-base':'''struct A{int n;A():n(7){}};struct L:virtual A{};struct R:virtual A{};struct D:L,R{};
int main(){try{throw D();}catch(A& a){return a.n!=7;}catch(...){return 1;}return 2;}''',
'pointer-exact':'''int main(){int n=7;try{throw &n;}catch(int*& p){if(p!=&n)return 1;return *p!=7;}return 2;}''',
'pointer-qualification':'''int main(){int n=7;try{throw &n;}catch(const int* p){return p!=&n;}catch(...){return 1;}return 2;}''',
'pointer-base':'''struct A{int a;};struct B{int b;};struct D:A,B{};
int main(){D d;try{throw &d;}catch(B* p){return p!=(B*)&d;}catch(...){return 1;}return 2;}''',
'null-pointer-base':'''struct A{};struct D:A{};int main(){D* d=0;try{throw d;}catch(A* a){return a!=0;}catch(...){return 1;}return 2;}''',
'pointer-void':'''int main(){int n;try{throw &n;}catch(void* p){return p!=&n;}catch(...){return 1;}return 2;}''',
'function-try-members':'''int dead;struct G{~G(){++dead;}};struct D{G g;~D()noexcept(false)try{throw 7;}catch(int n){if(dead!=1)throw 9;}};
int main(){try{D d;}catch(int n){return n!=7 || dead!=1;}return 2;}''',
'constructor-body-try':'''int dead,seen;struct G{~G(){++dead;}};struct D{G g;D()try{G local;throw 7;}catch(int){seen=dead;}};
int main(){try{D d;}catch(int n){return n!=7 || seen!=2 || dead!=2;}return 2;}''',
'function-try-template':'''template<class T> int f(T x)try{throw x;}catch(T y){return y;}int main(){return f(7)!=7;}''',
'bad-cast':'''struct A{virtual ~A(){}};struct B:A{};int main(){A a;try{B& b=dynamic_cast<B&>(a);return 1;}catch(...){return 0;}}''',
'bad-typeid':'''namespace std{class type_info;}struct A{virtual ~A(){}};int main(){A* a=0;try{typeid(*a);return 1;}catch(...){return 0;}}''',
'allocation-failure':'''int main(int argc,char**){try{unsigned long n=~0UL-argc;char* p=new char[n];delete[] p;return 1;}catch(...){return 0;}}''',
'malloc-failure':'''extern "C" void* malloc(unsigned long);extern "C" void free(void*);int main(){void* p=malloc(~0UL);free(0);return p!=0;}''',
'cleanup-double-throw':('''struct G{~G()noexcept(false){throw 2;}};int main(){try{G g;throw 1;}catch(...){return 0;}}''',1),
'handler-copy-throw':('''int copies;struct E{E(){}E(const E&){throw 3;}};int main(){try{throw E();}catch(E e){return 0;}catch(...){return 0;}}''',1),
'noexcept':('''void fail()noexcept{throw 7;}int main(){try{fail();}catch(...){return 0;}}''',1),
'rethrow-empty':('''int main(){throw;}''',1)
}
cases.update({
'nullptr-data-member':'''struct A{int n;};int main(){try{throw nullptr;}catch(int A::* p){return p!=nullptr;}catch(...){return 1;}return 2;}''',
'nullptr-function-member':'''struct A{int f();};int main(){try{throw nullptr;}catch(int(A::*p)()){return p!=nullptr;}catch(...){return 1;}return 2;}''',
'nullptr-pointer':'''int main(){try{throw nullptr;}catch(int* p){return p!=nullptr;}catch(...){return 1;}return 2;}''',
'pointer-cv-deep':'''int main(){int n=7;int* p=&n;try{throw &p;}catch(const int**){return 1;}catch(const int*const* q){return **q!=7;}catch(...){return 2;}return 3;}''',
'pointer-cv-remove':'''int main(){const int n=7;try{throw &n;}catch(int*){return 1;}catch(const int* p){return *p!=7;}return 2;}''',
'function-pointer-void':'''int f(){return 7;}int main(){try{throw &f;}catch(void*){return 1;}catch(int(*p)()){return p()!=7;}return 2;}''',
'null-virtual':'''struct A{};struct L:virtual A{};struct R:virtual A{};struct D:L,R{};int main(){D* d=0;try{throw d;}catch(A* p){return p!=0;}catch(...){return 1;}return 2;}''',
'null-ambiguous':'''struct A{};struct L:A{};struct R:A{};struct D:L,R{};int main(){D* d=0;try{throw d;}catch(A*){return 1;}catch(...){return 0;}}''',
'exception-destructor-throw':('''struct E{~E()noexcept(false){throw 2;}};int main(){try{try{throw E();}catch(E&){}}catch(int){return 0;}return 2;}''',1),
'copy-throw-retains-outer':'''int dead;struct G{~G(){++dead;}};struct E{E(){}E(const E&){throw 7;}};int main(){G g;E e;try{throw e;}catch(int n){return n!=7 || dead!=0;}return 2;}''',
'destructor-handler-return':(ROOT/'student.tests/pa25/destructor-handler-return.cc').read_text(),
'function-try-parameters':'''int dead;struct E{~E(){++dead;}};int f(E e)try{throw 7;}catch(int n){return dead==0?n:0;}int main(){int x=f(E());return x!=7 || dead!=1;}''',
})
observations=[]
for name,case in cases.items():
 source,expected=case if isinstance(case,tuple) else (case,0)
 path=OUT/(name+'.cc');path.write_text(source+'\n')
 for mode in ('direct','object'):
  exe=OUT/(name+'-'+mode);args=[str(CXX),'-o',str(exe),str(path)]
  if mode=='object':
   obj=OUT/(name+'.obj');p=subprocess.run([str(CXX),'-c','-o',str(obj),str(path)],capture_output=True,timeout=30)
   if p.returncode:
    observations.append(dict(case=name,mode=mode,phase='compile',status=p.returncode,error=p.stderr.decode()));continue
   args=[str(CXX),'-o',str(exe),str(obj)]
  p=subprocess.run(args,capture_output=True,timeout=30)
  if p.returncode:
   observations.append(dict(case=name,mode=mode,phase='link',status=p.returncode,error=p.stderr.decode()));continue
  p=subprocess.run([str(exe)],capture_output=True,timeout=5)
  observations.append(dict(case=name,mode=mode,phase='runtime',status=p.returncode,expected=expected,pass_test=p.returncode==expected))
reject=OUT/'constructor-handler-return.cc'
reject.write_text('struct E{E()try{throw 1;}catch(...){return;}}; int main(){E e;}\n')
p=subprocess.run([str(CXX),'-c','-o',str(OUT/'reject.obj'),str(reject)],capture_output=True,timeout=30)
observations.append(dict(case='constructor-handler-return',phase='compile',status=p.returncode,expected=1,pass_test=p.returncode!=0))
header=OUT/'shared.h';header.write_text('struct A{int n;A():n(7){} virtual ~A(){}}; struct B{int pad;B():pad(9){}}; struct D:B,A{}; void raise(); int handle();\n')
texts=['#include "shared.h"\nvoid raise(){throw D();}\n', '#include "shared.h"\nint handle(){try{raise();}catch(A& a){if(a.n!=7)throw 3;throw;}return 4;}\n', '#include "shared.h"\nint main(){try{handle();}catch(D& d){return d.n!=7 || d.pad!=9;}catch(...){return 2;}return 3;}\n']
paths=[];objects=[]
for i,text in enumerate(texts):
 src=OUT/f'cross{i}.cc';src.write_text(text);paths.append(src);obj=OUT/f'cross{i}.obj';objects.append(obj)
 p=subprocess.run([str(CXX),'-c','-o',str(obj),str(src)],capture_output=True,timeout=30)
 assert p.returncode==0,p.stderr
for mode,inputs in [('cross-direct',paths),('cross-objects',objects),('cross-mixed',[objects[0],*paths[1:]])]:
 exe=OUT/mode;p=subprocess.run([str(CXX),'-o',str(exe),*map(str,inputs)],capture_output=True,timeout=30)
 assert p.returncode==0,p.stderr
 p=subprocess.run([str(exe)],capture_output=True,timeout=5)
 observations.append(dict(case=mode,phase='runtime',status=p.returncode,expected=0,pass_test=p.returncode==0))
(OUT/'results.json').write_text(json.dumps(observations,indent=2)+'\n')
failed=[o for o in observations if not o.get('pass_test')]
print(json.dumps(failed,indent=2));print(f'{len(observations)-len(failed)}/{len(observations)} passed')
sys.exit(bool(failed))
