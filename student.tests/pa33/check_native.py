#!/usr/bin/env python3
"""Explicit host/native checks; the host compiler only builds the test driver."""
import os,pathlib,subprocess,json,hashlib
root=pathlib.Path(__file__).resolve().parents[2]
source=root/'student.tests/pa33'
out=pathlib.Path(os.environ['RALPH_ARTIFACT_DIR'])/'pa33-219/personal'
out.mkdir(parents=True,exist_ok=True)
rows=[]
def run(args):
    p=subprocess.run(list(map(str,args)),capture_output=True,text=True,timeout=60)
    rows.append(dict(command=list(map(str,args)),status=p.returncode,stdout=p.stdout,stderr=p.stderr))
    (out/'checks.json').write_text(json.dumps(rows,indent=2)+'\n')
    assert p.returncode==0,rows[-1]
for level in range(4):
    objects=[]
    for name in ['registers','builtins']:
        obj=out/f'{name}{level}.o';objects.append(obj)
        run([root/'dev/cppgm++',f'-O{level}','-c',source/(name+'.lowir'),'-o',obj])
    exe=out/f'check{level}'
    run(['g++','-std=c++11','-O2',source/'check_native.cpp',*objects,'-o',exe])
    run([exe])
print('PA33 personal cycles, ABI copy setup, builtin gates and protected-page strings: PASS (4 levels)')
