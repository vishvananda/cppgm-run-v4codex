#!/usr/bin/env python3
"""Definition matching: N3485 [class.mem]/1, [class.mfct]/2, [except.spec]/3–4, [basic.def.odr]/1."""
from pathlib import Path
import subprocess,sys,tempfile
ROOT=Path(__file__).resolve().parents[2]
BINARY=Path(sys.argv[1]).resolve() if len(sys.argv)>1 else ROOT/'dev/cppgm++'
CASES={
 'parameter':('int f(int);','int A<T>::f(double)'),
 'return':('int f(int);','double A<T>::f(int)'),
 'cv':('int f(int) const;','int A<T>::f(int)'),
 'ref':('int f(int) &;','int A<T>::f(int) &&'),
 'variadic':('int f(int,...);','int A<T>::f(int)'),
 'overload':('int f(int); int f(long);','int A<T>::f(double)'),
 'dependent_parameter':('int f(T);','int A<T>::f(T*)'),
 'dependent_return':('T f(int);','T* A<T>::f(int)'),
 'noexcept':('int f(int) noexcept;','int A<T>::f(int)'),
}
def source(declaration,definition):
 return 'template<class T> struct A {'+declaration+'};\ntemplate<class T> '+definition+' { return 0; }\nint main(){return 0;}\n'
EXTRA_CASES={
 'friend_not_member': 'template<class T> struct A { friend int f(int); };\ntemplate<class T> int A<T>::f(int){return 0;}\nint main(){return 0;}\n',
 'redefinition': 'template<class T> struct A { int f(int); };\ntemplate<class T> int A<T>::f(int){return 0;}\ntemplate<class T> int A<T>::f(int){return 1;}\nint main(){return 0;}\n',
 'inline_redefinition': 'template<class T> struct A { int f(int){return 0;} };\ntemplate<class T> int A<T>::f(int){return 1;}\nint main(){return 0;}\n',
}
if __name__=='__main__':
 with tempfile.TemporaryDirectory(prefix='pa14-definition-checks-') as tmp:
  inputs={name:source(*args) for name,args in CASES.items()};inputs.update(EXTRA_CASES)
  for name,text in inputs.items():
   src=Path(tmp)/(name+'.cpp');src.write_text(text)
   command=[str(BINARY),'--emit-lowir','-O0','-o',str(Path(tmp)/(name+'.lowir')),str(src)]
   result=subprocess.run(command,capture_output=True,text=True,timeout=30)
   assert result.returncode==1,(name,result.returncode,result.stderr)
   assert 'AddressSanitizer' not in result.stderr and 'runtime error:' not in result.stderr,(name,result.stderr)
   print(name,'rejected')
