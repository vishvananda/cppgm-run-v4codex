#!/usr/bin/env python3
"""Fixed operand types remain definition-time obligations despite dependent values."""
from pathlib import Path
import subprocess,sys,tempfile
ROOT=Path(__file__).resolve().parents[2]
CASES=[
 'template<class T>int f(){return sizeof(T)+nullptr;}',
 'template<class T>int f(){return *sizeof(T);}',
 'int take(int*);template<class T>int f(){return take(sizeof(T));}',
 'template<class T>int f(){return (sizeof(T)).missing;}',
 'template<class T>int f(){sizeof(T)=2;return 0;}',
 'template<class T>int f(){return (sizeof(T))[0];}',
 'template<class T>int f(){return sizeof(T);}int main(){return f<void>();}',
 'template<class T>int f(){return sizeof(sizeof(T));}int main(){return f<void>();}',
]
if __name__=='__main__':
 binary=Path(sys.argv[1]).resolve() if len(sys.argv)>1 else ROOT/'dev/cppgm++'
 with tempfile.TemporaryDirectory(prefix='pa14-body-values-') as directory:
  work=Path(directory)
  for i,source in enumerate(CASES):
   src=work/f'{i}.cpp';src.write_text(source)
   r=subprocess.run([binary,'--emit-lowir','-O0','-o',work/'out',src],capture_output=True,text=True,timeout=60)
   assert r.returncode==1,(i,r.returncode,r.stderr)
   assert not any(m in r.stderr for m in ('AddressSanitizer','UndefinedBehaviorSanitizer','runtime error:')),(i,r.stderr)
  print(len(CASES),'body value rejections PASS')
