#!/usr/bin/env python3
"""C++11 [except.spec]/5,8 and [except.handle]/3 override controls.
Usage: exception-override154.py OUT [COMPILER] [--observe]
--observe preserves the unfixed compiler's outcomes without accepting them.
"""
import hashlib, json, pathlib, subprocess, sys
root = pathlib.Path(__file__).resolve().parents[2]
out = pathlib.Path(sys.argv[1]).resolve(); out.mkdir(parents=True, exist_ok=True)
compiler = pathlib.Path(sys.argv[2] if len(sys.argv) > 2 else root/'dev/cppgm++').resolve()
observe = '--observe' in sys.argv
cases = {}
def case(name, base, derived, valid, prefix=''):
    cases[name] = (prefix+'\nstruct B { virtual void f() '+base+'; };\nstruct D:B { void f() '+derived+'; };\n', valid)
case('disjoint', 'throw(int)', 'throw(double)', False)
case('unrestricted', 'throw(int)', '', False)
case('false-noexcept', 'throw(int)', 'noexcept(false)', False)
case('narrower', 'throw(int,double)', 'throw(int)', True)
case('duplicate-types', 'throw(int,double)', 'throw(double,int,int)', True)
case('empty', 'throw(int)', 'throw()', True)
case('nonthrowing', 'throw(int)', 'noexcept', True)
case('empty-base', 'throw()', 'throw(int)', False)
case('unrestricted-base', '', 'throw(int)', True)
case('scalar-promotion', 'throw(long)', 'throw(int)', False)
case('public-class', 'throw(X)', 'throw(Y)', True, 'struct X {}; struct Y:X {};')
case('private-class', 'throw(X)', 'throw(Y)', False, 'struct X {}; class Y:X {};')
case('protected-class', 'throw(X)', 'throw(Y)', False, 'struct X {}; struct Y:protected X {};')
case('ambiguous-class', 'throw(X)', 'throw(Y)', False, 'struct X {}; struct L:X {}; struct R:X {}; struct Y:L,R {};')
case('shared-class', 'throw(X)', 'throw(Y)', True, 'struct X {}; struct L:virtual X {}; struct R:virtual X {}; struct Y:L,R {};')
case('shared-public-path', 'throw(X)', 'throw(Y)', True, 'struct X {}; class L:virtual X {}; struct R:virtual X {}; struct Y:L,R {};')
case('public-pointer', 'throw(const X*)', 'throw(Y*)', True, 'struct X {}; struct Y:X {};')
case('private-pointer', 'throw(X*)', 'throw(Y*)', False, 'struct X {}; class Y:X {};')
case('qualification', 'throw(const int*)', 'throw(int*)', True)
case('remove-qualification', 'throw(int*)', 'throw(const int*)', False)
case('void-pointer', 'throw(const void*)', 'throw(int*)', True)
case('function-pointer', 'throw(void*)', 'throw(void(*)())', False)
case('deep-qualification', 'throw(const int* const*)', 'throw(int**)', True)
case('unsafe-qualification', 'throw(const int**)', 'throw(int**)', False)
case('null-pointer', 'throw(int*)', 'throw(decltype(nullptr))', True)
cases['template-invalid'] = ('template<class T> struct B {virtual void f() throw(T);}; struct D:B<int> {void f() throw(double);};', False)
cases['template-valid'] = ('template<class T> struct B {virtual void f() throw(T);}; template<class T> struct D:B<T> {void f() throw(T);}; int n=sizeof(D<int>);', True)
cases['multiple-base'] = ('struct B {virtual void f() throw(int,double);}; struct C {virtual void f() throw(char,int);}; struct D:B,C {void f() throw(double);};', False)
cases['implicit-destructor'] = ('struct B {virtual ~B() throw(int);}; struct D:B {};', True)
cases['implicit-destructor-extra'] = ('struct B {virtual ~B() throw(int);}; struct M {~M() throw(double);}; struct D:B {M m;};', False)
cases['implicit-destructor-unrestricted'] = ('struct B {virtual ~B() throw(int);}; struct M {~M() noexcept(false);}; struct D:B {M m;};', False)
cases['implicit-destructor-transitive'] = ('struct B {virtual ~B() throw(int);}; struct C:B {}; struct D:C {~D() throw(double);};', False)
cases['unused-member-body'] = ('template<class T> struct B {virtual void f() throw(int); void unused(){T::missing();}}; struct D:B<int> {void f() throw(int);};', True)
rows=[]
for name, (source, valid) in cases.items():
    path=out/(name+'.cpp'); path.write_text(source)
    args=[str(compiler),'-std=c++11','-c',str(path),'-o',str(out/'check.o')]
    p=subprocess.run(args,cwd=root,capture_output=True,text=True,timeout=30)
    rows.append(dict(name=name,source=source,expected_success=valid,command=args,status=p.returncode,stderr=p.stderr))
    (out/'results.json').write_text(json.dumps(dict(binary_sha256=hashlib.sha256(compiler.read_bytes()).hexdigest(),checks=rows),indent=2)+'\n')
    if not observe: assert (p.returncode==0)==valid,rows[-1]
print('exception override', 'observed' if observe else 'PASS',len(rows),'declarations')
