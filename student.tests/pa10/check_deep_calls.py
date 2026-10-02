#!/usr/bin/env python3
"""Deep procedural call lowering under the ordinary 8 MiB process stack."""
import pathlib
import resource
import subprocess
import sys
import tempfile

compilers = [pathlib.Path(p).resolve() for p in sys.argv[1:]]
assert compilers, 'pass one or more compiler paths'
def stack():
    resource.setrlimit(resource.RLIMIT_STACK, (8*1024*1024, resource.getrlimit(resource.RLIMIT_STACK)[1]))
with tempfile.TemporaryDirectory(prefix='pa10-deep-calls-') as directory:
    out = pathlib.Path(directory)
    depth = 400
    source = out/'deep.cpp'
    expression = 'value'
    for i in range(1, depth+1):
        expression = f'add({expression},{i})'
    source.write_text('int calls; long& add(long& v,int n){++calls;v+=n;return v;}\n'
                      'int main(int argc,char**){long value=argc;'+expression+';'
                      f'return value!=argc+{depth*(depth+1)//2} || calls!={depth};'+'}\n')
    for level in ('-O0','-O3'):
        for i,compiler in enumerate(compilers):
            for mode,ext in (('--emit-lowir','lowir'),('-c','o')):
                subprocess.run([compiler,level,mode,source,'-o',out/f'{i}.{ext}'],check=True,preexec_fn=stack)
            subprocess.run(['g++',out/f'{i}.o','-o',out/f'{i}.exe'],check=True)
            for args in ([],['a'],['a','b']):
                subprocess.run([out/f'{i}.exe',*args],check=True)
            for ext in ('lowir','o'):
                assert (out/f'0.{ext}').read_bytes()==(out/f'{i}.{ext}').read_bytes(), (level,i,ext)
print('400 nested calls: 8 MiB stack, O0/O3, exact output and runtime effects PASS')
