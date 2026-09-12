#!/usr/bin/env python3
"""Definition-owned object/member facts validate unused bodies without emission."""
from pathlib import Path
import subprocess,tempfile,sys,json
root=Path(__file__).resolve().parents[2]
binary=Path(sys.argv[1]).resolve() if len(sys.argv)>1 else root/'dev/cppgm++'
rejections=[
 'struct V{};template<class T>int f(V& v){return v.missing;}',
 'template<class T>int f(int n){return n.missing;}',
 'struct V{private:int n;};template<class T>int f(V& v){return v.n;}',
 'struct V{private:int get();};template<class T>int f(V& v){return v.get();}',
 'struct V{int get()=delete;};template<class T>int f(V& v){return v.get();}',
 'struct V{int get()&;};template<class T>int f(V&& v){return static_cast<V&&>(v).get();}',
 'struct V{int get();};template<class T>int f(const V& v){return v.get();}',
 'struct V{int n;};template<class T>int f(const V& v){return ++v.n;}',
 'struct V{int get(int*);};template<class T>int f(V& v){return v.get(1);}',
 'struct B{int get();};struct V:private B{};template<class T>int f(V& v){return v.get();}',
 'struct V{using Type=int;};template<class T>void f(V& v){v.Type;}',
 'struct V{using Type=int;};template<class T>auto f(V& v)->decltype(v.Type);',
 'struct V{private:int n;};template<class T>auto f(V& v)->decltype(v.n);',
 'template<class T>struct V{int get();};template<class T>int f(V<int>& v){return v.missing();}',

]
with tempfile.TemporaryDirectory(prefix='pa14-fixed-objects-') as directory:
 work=Path(directory)
 for i,source in enumerate(rejections):
  src=work/f'reject-{i}.cpp';src.write_text(source)
  r=subprocess.run([binary,'--emit-lowir','-O0','-o',work/'output',src],capture_output=True,text=True)
  assert r.returncode==1,(i,r.returncode,r.stderr)
  assert not any(x in r.stderr for x in ('AddressSanitizer','UndefinedBehaviorSanitizer','runtime error:')),(i,r.stderr)
 src=work/'unused.cpp';src.write_text('template<class T>struct Lazy{int bad(){return T::missing;}};template<class U>int unused(Lazy<int>& v){return v.bad();}int main(){return 0;}')
 r=subprocess.run([binary,'--emit-lowir','-O0','--stats','-o',work/'output',src],capture_output=True,text=True)
 assert r.returncode==0,r.stderr
 t=json.loads(r.stderr.splitlines()[0]);assert t['template_body_transitions']==0 and t['semantic_member_demands']==0,t
 assert t['semantic_template_fixed_calls']==1,t
 print(len(rejections),'fixed object/member rejections and unused-member non-demand PASS')
