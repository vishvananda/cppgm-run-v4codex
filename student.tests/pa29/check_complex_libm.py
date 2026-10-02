#!/usr/bin/env python3
"""Explicit complex libm registry/signature/runtime controls; optional compiler."""
import pathlib
import subprocess
import sys
import tempfile

compiler = pathlib.Path(sys.argv[1] if len(sys.argv)>1 else 'dev/cppgm++').resolve()
unary = 'conj cproj csqrt cexp clog csin ccos ctan casin cacos catan csinh ccosh ctanh casinh cacosh catanh'.split()
real = 'cabs carg creal cimag'.split()
with tempfile.TemporaryDirectory(prefix='pa29-complex-') as tmp:
    out = pathlib.Path(tmp)
    declarations = []
    for suffix, scalar in (('f','float'), ('','double'), ('l','long double')):
        for name in unary + real + ['cpow']:
            builtin = '__builtin_'+name+suffix
            result = scalar if name in real else '__complex__ '+scalar
            params = ','.join(['__complex__ '+scalar]*(2 if name == 'cpow' else 1))
            declarations += [f'#if !__has_builtin({builtin})\n#error missing registry entry\n#endif',
                             f'{result} (*p_{name}{suffix})({params}) = {builtin};']
    src = out/'signatures.cpp'
    src.write_text('\n'.join(declarations)+'\n')
    subprocess.run([compiler,'-c',src,'-o',out/'signatures.o'],check=True)
    for level in ('-O0','-O3'):
        subprocess.run([compiler,level,'-c','student.tests/pa29/complex-libm.cpp','-o',out/'source.o'],check=True)
        subprocess.run(['g++',out/'source.o','-lm','-o',out/'source'],check=True)
        for args in ([],['input'],['more','input']):
            subprocess.run([out/'source',*args],check=True)
    for call in ('__builtin_cabsf()', '__builtin_cpow(1.0)', '__builtin_csinl(1.0,2.0)'):
        src.write_text('void f(){'+call+';}\n')
        r = subprocess.run([compiler,'-c',src,'-o',out/'bad.o'],capture_output=True)
        assert r.returncode == 1, r
print('complex libm: 66 signatures/probes, O0/O3 three-precision runtime, arity rejection PASS')
