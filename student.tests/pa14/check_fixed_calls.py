#!/usr/bin/env python3
"""Fixed call selection validates unused definitions without demanding bodies."""
from pathlib import Path
import subprocess,sys,tempfile,json
root=Path(__file__).resolve().parents[2]
binary=Path(sys.argv[1]).resolve() if len(sys.argv)>1 else root/'dev/cppgm++'
rejections=[
 'int call(int*);template<class T>int f(){return call(1);}',
 "int call(int*);template<class T>int f(){return call('\\0');}",
 'int call(long);int call(unsigned);template<class T>int f(int n){return call(n);}',
 'int call(int,int);template<class T>int f(){return call(1);}',
 'int call();template<class T>int f(){return call(1);}',
 'struct V{private:V(int);};int call(V);template<class T>int f(int n){return call(n);}',
 'struct V{V(int)=delete;};int call(V);template<class T>int f(int n){return call(n);}',
 'struct V{private:~V();};V make();template<class T>void f(){make();}',
 'template<class T>int f(int(*fn)(int*)){return fn(1);}',
 'struct X{int n;X(const X&)=delete;};struct V{operator X();};V make();int call(X);template<class T>int f(){return call(make());}',
 'struct B{private:static int call(int);};template<class T>struct D:B{int f(int n){return call(n);}};',
 'struct B{protected:static int call(int);};template<class T>int f(int n){return B::call(n);}',
 'struct B{public:static int call(int);};class D:B{};template<class T>struct E:D{int f(int n){return call(n);}};',
 'struct B{};struct D:private B{};D make();int call(const B&);template<class T>int f(){return call(make());}',
 'struct B{};struct D:private B{};D make();int call(B);template<class T>int f(){return call(make());}',
 'struct B{};struct D:private B{};D* make();int call(B*);template<class T>int f(){return call(make());}',
 'struct B{};struct D:private B{};struct V{operator D*();};V make();int call(B*);template<class T>int f(){return call(make());}',

]
with tempfile.TemporaryDirectory(prefix='pa14-fixed-calls-') as directory:
 work=Path(directory)
 for i,source in enumerate(rejections):
  path=work/f'reject-{i}.cpp';path.write_text(source)
  r=subprocess.run([binary,'--emit-lowir','-O0','-o',work/'output',path],capture_output=True,text=True)
  assert r.returncode==1,(i,r.returncode,r.stderr)
  assert not any(s in r.stderr for s in ('AddressSanitizer','runtime error:','UndefinedBehaviorSanitizer')),(i,r.stderr)
 source='template<class U>int bad(){return U::missing;}template<class T>int unused(){return bad<int>();}int main(){return 0;}'
 path=work/'unused.cpp';path.write_text(source)
 r=subprocess.run([binary,'--emit-lowir','-O0','--stats','-o',work/'output',path],capture_output=True,text=True)
 assert r.returncode==0,r.stderr
 t=json.loads(r.stderr.splitlines()[0]);assert t['template_body_transitions']==0,t
 assert t['semantic_template_fixed_calls']==1,t
 print(f'{len(rejections)} fixed call rejections and unused-body non-demand PASS')
