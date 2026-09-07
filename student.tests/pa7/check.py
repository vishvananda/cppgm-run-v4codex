#!/usr/bin/env python3
"""Explicit PA7 behavioral checks, independent of course discovery/oracles."""
import pathlib
import subprocess
import sys
import tempfile

compiler = str(pathlib.Path(sys.argv[1] if len(sys.argv) > 1 else 'dev/cppgm++').resolve())
cases = [
    ('unresolved arithmetic', 'int f(int); int f(long); int g(){return f+1;}', False, ''),
    ('unresolved sizeof', 'int f(int); int f(long); int g(){return sizeof(f);}', False, ''),
    ('unresolved discarded function', 'int f(int); int f(long); void g(){f;}', False, ''),
    ('member cv pointer identity', 'struct C{void f();void f()const;};using P=void(C::*)()const;P p=&C::f;', True, 'id-expression lvalue function of (pointer to const struct C) returning void C::f'),
    ('deferred unused body', 'struct C{void unused(){missing();} void used(){}};using P=void(C::*)();P p=&C::used;', True, 'function-definition C::used'),
    ('template type substitution', 'template<class T>void f(T*);void(*p)(int*)=&f<int>;', True, 'function-declaration f function of (pointer to int) returning void'),

    ('recursive call', 'int count(int n) { if(n) return count(n-1); return 0; }', True, 'callee count'),
    ('source-order lookup', 'int f(){ return later(); } int later(){return 0;}', False, ''),
    ('overload arity', 'int f(int); long f(int,int); long g(){return f(1,2);}', True, 'callee f function of (int, int) returning long int'),
    ('top-level cv identity', 'int f(int); int f(const int x){return x;} int g(){return f(1);}', True, 'id-expression lvalue const int x'),
    ('definition parameter binding', 'int f(int old); int f(int fresh){return fresh;}', True, 'id-expression lvalue int fresh'),
    ('function target context', 'int f(int); long f(long); int (*p)(int)=f;', True, 'id-expression lvalue function of (int) returning int f'),
    ('indirect conversion', 'int f(int(*p)(int*)){return p(2);}', False, ''),
    ('qualified nonescape', 'int f(); namespace N {} int g(){return N::f();}', False, ''),
    ('using snapshot', 'namespace N { int f(int); } using N::f; namespace N { long f(long); } int g(){return f(1L);}', True, 'callee N::f function of (int) returning int'),
    ('const intermediate pointer', 'void f(const int*const*); void g(int**p){f(p);}', True, 'callee f'),
    ('missing intermediate const', 'void f(const int**); void g(int**p){f(p);}', False, ''),
    ('const source reference', 'void f(int*&); void g(int*const&p){f(p);}', False, ''),
    ('reference return increment', 'int& f(); int g(){return ++f();}', True, 'unary-expression lvalue int OP_INC:++'),
    ('array sizeof expression', 'int g(){int a[3]; return sizeof(a[1]);}', True, 'sizeof-expression prvalue unsigned long int'),
    ('scope siblings', 'int f(int c){if(c) int a=1; else long a=2; return 0;}', True, 'variable a long int'),
    ('scope leak', 'int f(int c){if(c) int a=1; return a;}', False, ''),
    ('continue in switch', 'void f(int x){switch(x){case 1: continue;}}', False, ''),
    ('switch inside loop', 'void f(int x){while(x){switch(x){case 1:continue;default:break;} break;}}', True, 'continue-statement'),
    ('scoped enum condition', 'enum class E{a}; int f(E e){if(e)return 1;return 0;}', False, ''),
    ('constant propagation', 'int f(int x){return __builtin_constant_p(false ? x : 3);}', True, 'literal prvalue int 1'),
    ('ordinary builtin prefix', 'int __builtin_mine(int x){return x;} int f(){return __builtin_mine(4);}', True, 'callee __builtin_mine'),
    ('array bound inference', 'int a[]={1,2,3,4};', True, 'variable a array of 4 int'),
    ('mixed bool conditional', 'bool f(bool c,bool b){return c?b:true;}', True, 'conditional-expression prvalue bool'),
]
with tempfile.TemporaryDirectory(prefix='pa7-personal-') as directory:
    root = pathlib.Path(directory)
    for name, source, success, fragment in cases:
        src, out = root / 'input.cc', root / 'output.txt'
        src.write_text(source)
        run = subprocess.run([compiler, '--emit-semantics', '-o', str(out), str(src)], capture_output=True, text=True)
        assert (run.returncode == 0) == success, (name, run.returncode, run.stderr)
        if success:
            assert fragment in out.read_text(), (name, out.read_text())
    one, two, out = root / 'one.cc', root / 'two.cc', root / 'multi.txt'
    one.write_text('int x;'); two.write_text('long x;')
    subprocess.run([compiler, '--emit-semantics', '-o', str(out), str(one), str(two)], check=True)
    assert out.read_text().startswith('2 translation units\n')
    assert 'variable x int\n' in out.read_text() and 'variable x long int\n' in out.read_text()
print(f'{len(cases)} PA7 personal cases and independent translation units passed')
