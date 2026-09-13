#!/usr/bin/env python3
"""N3485 [temp.decls]/2, [temp.inst]/10,12-13: separate default definitions.
Ordinary unused defaults must not instantiate function/member specializations.
A used default is checked as an independent initializer, including when the
outer call is inside decltype. Genuine sizeof operands inside it remain unevaluated.
"""
from pathlib import Path
import hashlib,json,subprocess,sys
ROOT=Path(__file__).resolve().parents[2]
B,W=map(lambda p:Path(p).resolve(),sys.argv[1:3]);W.mkdir(parents=True,exist_ok=True)
observe=len(sys.argv)>3 and sys.argv[3]=='observe'
CASES={
 'function':('template<class T>int make(){return 4;}template<class T>int f(int n=make<T>()){return n;}using Probe=decltype(f<int>());int main(){return f<int>()!=4;}',True),
 'member':('template<class T>struct C{static int make(){return 4;}static int f(int n=make()){return n;}};using Probe=decltype(C<int>::f());int main(){return C<int>::f()!=4;}',True),
 'storage':('template<class T>struct C{static int n;};template<class T>int C<T>::n=4;template<class T>int f(int n=C<T>::n){return n;}using Probe=decltype(f<int>());int main(){if(f<int>()!=4)return 1;++C<int>::n;return f<int>()!=5;}',True),
 'nested-query':('template<class T>int make(){return T::missing;}template<class T>int f(int n=sizeof(make<T>())){return n;}int main(){return f<int>()!=4;}',True),
 'used-default-bad':('template<class T>int make(){return T::missing;}template<class T>int f(int n=make<T>()){return n;}using Probe=decltype(f<int>());int main(){return 0;}',False),
 'unused-ordinary':('template<class T>int bad(){return T::missing;}void f(int n=bad<int>());int main(){return 0;}',True),
 'unused-fixed-function':('template<class T>int bad(){return T::missing;}int f(int n=bad<int>());template<class T>int unused(){return f();}int main(){return 0;}',True),
 'unused-fixed-constructor':('template<class T>int bad(){return T::missing;}struct A{A(int,int=bad<int>());};int f(A);template<class T>int unused(){return f(1);}int main(){return 0;}',True),
 'unselected-constructor':('template<class T>int bad(){return T::missing;}struct A{A(int,int=bad<int>());};int f(A);int f(int n){return n;}int main(){return f(1)!=1;}',True),
 'nested-defaults':('int count;template<class T>int make(){return ++count;}template<class T>int g(int n=make<T>()){return n;}template<class T>int f(int n=g<T>()){return n;}using Probe=decltype(f<int>());int main(){if(f<int>()!=1)return 1;return f<int>()!=2;}',True),
 'constructor-recipe':('int count;template<class T>int make(){return ++count;}struct A{int n;A(int a,int b=make<int>()):n(a+b){}};int f(A a){return a.n;}template<class T>int use(){return f(1)+f(2);}int main(){return use<int>()!=6||count!=2;}',True),
 'constructor-query':('template<class T>int make(){return 4;}struct A{int n;A(int a=make<int>()):n(a){}};using Probe=decltype(A());int main(){A a;return a.n!=4;}',True),
 'private-conversion-context':('struct Owner;class Secret{friend struct Owner;operator int(){return 9;}};struct Owner{static int f(int n=Secret()){return n;}};int main(){return Owner::f()!=9;}',True),
 'list-conversion-identity':('int count;struct Convert{operator int(){return ++count;}};struct A{int n;};template<class T>int f(T a={Convert()}){return a.n;}int main(){if(f<A>()!=1)return 1;return f<A>()!=2||count!=2;}',True),
}
CASES.update({
 'selected-list-bad':('template<class T>struct A{A(T& n=1){}};int f(A<int>);int main(){return f({});}',False),
 'unselected-list-bad':('template<class T>struct A{A(T& n=1){}};int f(A<int>);int f(int n){return n;}int main(){return f({});}',True),
 'list-class-completion':('template<class T>struct A{int n;A(int v=4):n(v){}};int f(A<int> a){return a.n;}int main(){return f({})!=4;}',True),
 'self-default':('struct C{int n;C(int v):n(v){}static int f(C c={4}){return c.n;}};int main(){return C::f()!=4;}',True),
 'later-member':('struct C{static int f(int n=g()){return n;}static int g(){return 4;}};int main(){return C::f()!=4;}',True),
 'nested-later-member':('struct C{struct Inner{static int f(int n=g()){return n;}};static int g(){return 4;}};int main(){return C::Inner::f()!=4;}',True),
 'elided-template-copy-bad':('template<class T>struct C{C(int){}C(const C&){int n=T::missing;}};template<class T>C<T>make(){return C<T>(4);}template<class T>void f(C<T> c=make<T>());int main(){f<int>();}',False),
 'unused-template-copy-bad':('template<class T>struct C{C(int){}C(const C&){int n=T::missing;}};template<class T>C<T>make(){return C<T>(4);}template<class T>void f(C<T> c=make<T>());int main(){C<int> value(4);return 0;}',True),
 'incomplete-reference':('struct C;C&get();void use(C&);void f(){use({get()});}',True),
})
COMPILE_ONLY={'incomplete-reference'}
def sha(p):return hashlib.sha256(Path(p).read_bytes()).hexdigest()
rows=[]
def run(cmd,name,expected):
 p=subprocess.run(list(map(str,cmd)),capture_output=True,text=True,cwd=ROOT,timeout=180)
 log=W/(name+'.log');log.write_text(p.stdout+p.stderr)
 rows.append(dict(name=name,command=list(map(str,cmd)),expected=expected,exit_code=p.returncode,log=str(log),log_sha256=sha(log)))
 (W/'checks.json').write_text(json.dumps(dict(binary=str(B),binary_sha256=sha(B),checks=rows),indent=2)+'\n')
 if not observe:assert p.returncode==expected,(name,p.returncode,p.stderr)
 assert 'AddressSanitizer' not in p.stderr and 'runtime error:' not in p.stderr,(name,p.stderr)
 return p.returncode
for name,(source,valid) in CASES.items():
 path=W/(name+'.cpp');path.write_text(source);ir=W/(name+'.lowir');exe=W/(name+'.exe')
 status=run([B,'--emit-lowir','-O0','--validate-lowir','-o',ir,path],name+'-compile',0 if valid else 1)
 if not status and valid and name not in COMPILE_ONLY:
  status=run([ROOT/'dev/lowir2native-ref','-O0','-o',exe,ir],name+'-backend',0)
  if not status:status=run([exe],name+'-run',0)
 print(name,'PASS' if status==(0 if valid else 1) else 'observed failure',status,flush=True)
