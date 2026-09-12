#!/usr/bin/env python3
"""Definition-time binding and control checks, including unused bodies."""
from pathlib import Path
import subprocess, tempfile
root=Path(__file__).resolve().parents[2]
cases=[
 'template<class T> int f(){return missing;}',
 'template<class T> struct S { struct I { int f(){return missing;} }; };',
 'template<class T> int f(){typedef int value; return value;}',
 'template<class T> int f(){if(int n=1){} return n;}',
 'template<class T> int f(){if(true){return later; if(int later=0){}} return 0;}',
 'template<class T> int f(){goto label; int value=1; label:return value;}',
 'template<class T> struct S { int f(){switch(int n=0){case 0:int value=n;return value;default:return 1;}} };',
 'template<class T> int f(){struct Local { int value; } object; return value;}',
 'namespace A{int n;} namespace B{int n;} template<class T> int f(){using namespace A;using namespace B;return n;}',
 'template<class T> int f(){return unknown_call(1);}',
]
with tempfile.TemporaryDirectory(prefix='pa14-bindings-') as directory:
 work=Path(directory)
 for i,source in enumerate(cases):
  path=work/f'reject-{i}.cpp';path.write_text(source)
  r=subprocess.run([root/'dev/cppgm++','--emit-lowir','-O0','-o',work/'output',path],capture_output=True,text=True)
  assert r.returncode==1,(i,r.returncode,r.stderr)
 print(f'{len(cases)} unused-body binding/control rejections PASS')
