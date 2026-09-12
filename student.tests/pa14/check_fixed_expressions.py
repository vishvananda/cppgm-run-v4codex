#!/usr/bin/env python3
"""Unused fixed scalar expression legality at template definition."""
from pathlib import Path
import subprocess, sys, tempfile
root=Path(__file__).resolve().parents[2]
binary=Path(sys.argv[1]).resolve() if len(sys.argv)>1 else root/'dev/cppgm++'
cases=[
 'template<class T>int f(){return 1.0 % 2;}',
 'template<class T>int f(){return ~1.5;}',
 'template<class T>int f(){return *3;}',
 'template<class T>int f(){return ++2;}',
 'template<class T>int f(){const int x=1;return ++x;}',
 'template<class T>int f(){return 1 << 1.5;}',
 'template<class T>int f(){return sizeof(1.0%2);}',
 'template<class T>int f(){return 1[0];}',
 'template<class T>int f(){int x=1;return x & 1.5;}',
 'template<class T>int f(){int x=1;return (x+1)=3;}',
 'template<class T>struct S{int f(){return 1.0%2;}};',
 'template<class T>struct S{static int f();}; template<class T>int S<T>::f(){return *2;}',
]
with tempfile.TemporaryDirectory(prefix='pa14-fixed-') as directory:
 work=Path(directory)
 for i,source in enumerate(cases):
  path=work/f'reject-{i}.cpp';path.write_text(source)
  r=subprocess.run([binary,'--emit-lowir','-O0','-o',work/'output',path],capture_output=True,text=True)
  assert r.returncode==1,(i,r.returncode,r.stderr)
  assert not any(s in r.stderr for s in ('AddressSanitizer','runtime error:','UndefinedBehaviorSanitizer')),(i,r.stderr)
 print(f'{len(cases)} unused fixed scalar expression rejections PASS')
