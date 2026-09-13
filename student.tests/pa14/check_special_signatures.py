#!/usr/bin/env python3
"""N3485 [class.mfct]/2, [class.conv.fct]/1, [except.spec]/3–4, [basic.def.odr]/1, [dcl.fct.def.delete]/4."""
from pathlib import Path
import subprocess,sys,tempfile
ROOT=Path(__file__).resolve().parents[2]
BINARY=Path(sys.argv[1]).resolve() if len(sys.argv)>1 else ROOT/'dev/cppgm++'
CASES={
 'constructor_parameter':('A(int);','A<T>::A(double)'),
 'constructor_dependent':('A(T);','A<T>::A(T*)'),
 'constructor_reference':('A(const T&);','A<T>::A(T&)'),
 'constructor_noexcept':('A(int) noexcept;','A<T>::A(int)'),
 'copy_parameter':('A(const A&);','A<T>::A(A&)'),
 'move_parameter':('A(A&&);','A<T>::A(A&)'),
 'assignment_result':('A& operator=(const A&);','A<T> A<T>::operator=(const A&)'),
 'assignment_parameter':('A& operator=(const A&);','A<T>& A<T>::operator=(A*)'),
 'constructor_arity':('A(int);','A<T>::A(int,int)'),
 'conversion_result':('operator int() const;','A<T>::operator double() const'),
 'conversion_cv':('operator T() const;','A<T>::operator T()'),
 'conversion_ref':('operator int() &;','A<T>::operator int() &&'),
 'conversion_noexcept':('operator int() noexcept;','A<T>::operator int()'),
}
def source(declaration,definition):
 return 'template<class T> struct A {'+declaration+'};\ntemplate<class T> '+definition+' { for(;;){} }\nint main(){return 0;}\n'
EXTRA_CASES={
 'constructor_redefinition':'template<class T> struct A { A(int); };\ntemplate<class T> A<T>::A(int){}\ntemplate<class T> A<T>::A(int){}\nint main(){return 0;}\n',
 'destructor_redefinition':'template<class T> struct A { ~A(); };\ntemplate<class T> A<T>::~A(){}\ntemplate<class T> A<T>::~A(){}\nint main(){return 0;}\n',
 'inline_destructor':'template<class T> struct A { ~A(){} };\ntemplate<class T> A<T>::~A(){}\nint main(){return 0;}\n',
 'defaulted_constructor_redefinition':'template<class T> struct A { A(); };\ntemplate<class T> A<T>::A() = default;\ntemplate<class T> A<T>::A() = default;\nint main(){return 0;}\n',
 'defaulted_destructor_redefinition':'template<class T> struct A { ~A(); };\ntemplate<class T> A<T>::~A() = default;\ntemplate<class T> A<T>::~A() = default;\nint main(){return 0;}\n',
 'constructor_redeclaration':'template<class T> struct A { A(); };\ntemplate<class T> A<T>::A();\nint main(){return 0;}\n',
 'late_deleted_constructor':'template<class T> struct A { A(); };\ntemplate<class T> A<T>::A() = delete;\nint main(){return 0;}\n',
 'nested_alias_parameter':'template<class T> struct A { struct B; };\ntemplate<class U> struct A<U>::B { typedef U type; B(type); };\ntemplate<class V> A<V>::B::B(type*) {}\nint main(){return 0;}\n',

}
if __name__=='__main__':
 with tempfile.TemporaryDirectory(prefix='pa14-special-signatures-') as tmp:
  inputs={name:source(*args) for name,args in CASES.items()};inputs.update(EXTRA_CASES)
  for name,text in inputs.items():
   src=Path(tmp)/(name+'.cpp');src.write_text(text)
   result=subprocess.run([str(BINARY),'--emit-lowir','-O0','-o',str(Path(tmp)/(name+'.lowir')),str(src)],capture_output=True,text=True,timeout=30)
   assert result.returncode==1,(name,result.returncode,result.stderr)
   assert 'AddressSanitizer' not in result.stderr and 'runtime error:' not in result.stderr,(name,result.stderr)
   print(name,'rejected')
