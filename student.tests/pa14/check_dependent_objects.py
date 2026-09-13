#!/usr/bin/env python3
"""Fixed member value facts are checked before class body instantiation."""
from pathlib import Path
import subprocess,tempfile,sys
root=Path(__file__).resolve().parents[2]
binary=Path(sys.argv[1]).resolve() if len(sys.argv)>1 else root/'dev/cppgm++'
rejections=[
 'template<class T>struct C{int n;int f(){return *n;}};',
 'template<class T>struct C{int n;int f()const{return ++n;}};',
 'template<class T>struct C{int n;int f()const{return ++this->n;}};',
 'template<class T>struct C{int n;static int f(){return n;}};',
 'template<class T>struct C{int n;int f(){return sizeof(*n);}};',
 'template<class T>struct C{int n;int f(){return this->n.missing;}};',
 'template<class T>struct C{int n;struct Inner{int f(){return n;}};};',
 'struct Base{private:int n;};template<class T>struct C:Base{int f(){return n;}};',
 'template<class T>struct C{int n;int f()const;};template<class U>int C<U>::f()const{return ++n;}',
 'template<class T>struct C{int n;static int f();};template<class U>int C<U>::f(){return n;}',
 'template<class T>struct C{int n;int f();};template<class U>int C<U>::f(){return *this->n;}',
 'template<class T>struct C{int n;static int f(int);int f(long);};template<class U>int C<U>::f(int){return n;}',
 'template<class T>struct C{int n;static int f();};template<class U>int C<U>::f(){return sizeof(this);}',
]
with tempfile.TemporaryDirectory(prefix='pa14-dependent-objects-') as tmp:
 work=Path(tmp)
 for i,source in enumerate(rejections):
  src=work/f'reject-{i}.cpp';src.write_text(source)
  r=subprocess.run([binary,'--emit-lowir','-O0','-o',work/'out',src],capture_output=True,text=True)
  assert r.returncode==1,(i,r.returncode,r.stderr)
  assert not any(s in r.stderr for s in ('AddressSanitizer','runtime error:','UndefinedBehaviorSanitizer')),(i,r.stderr)
 print(len(rejections),'template-owned member rejections PASS')
