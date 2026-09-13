#!/usr/bin/env python3
"""Deferred regions still validate every demanded body/default and fixed name."""
from pathlib import Path
import subprocess,sys,tempfile
ROOT=Path(__file__).resolve().parents[2]
BINARY=Path(sys.argv[1]).resolve() if len(sys.argv)>1 else ROOT/'dev/cppgm++'
CASES=[
 'template<class T>struct C{int f(int n=T::missing){return n;}};int main(){C<int> c;return c.f();}',
 'template<class T>struct C{int f(){return T::missing;}};int main(){C<int> c;return c.f();}',
 'template<class T>struct C{C(int n=T::missing){}};int main(){C<int> c;}',
 'template<class T>struct C{int f(int n=missing){return n;}};',
 'template<class T>struct C{int f(int n=missing);};',
 'template<class T>struct C{int f(int n=T::missing,int m){return m;}};C<int> c;',
 'template<class T>struct C{int f(int n=T::missing);};template<class U>int C<U>::f(int n=2){return n;}int main(){C<int> c;return c.f(1);}',
 'template<class T>struct C{int value;C(int n):absent(n){}};int main(){C<int> c(1);}',
]
if __name__=='__main__':
 with tempfile.TemporaryDirectory(prefix='pa14-demand-regions-') as tmp:
  w=Path(tmp)
  for i,source in enumerate(CASES):
   src=w/f'reject-{i}.cpp';src.write_text(source)
   r=subprocess.run([BINARY,'--emit-lowir','-O0','-o',w/'out',src],capture_output=True,text=True,timeout=60)
   assert r.returncode==1,(i,r.returncode,r.stderr)
   assert not any(s in r.stderr for s in ('AddressSanitizer','UndefinedBehaviorSanitizer','runtime error:')),(i,r.stderr)
  print(len(CASES),'demand-region rejections PASS')
