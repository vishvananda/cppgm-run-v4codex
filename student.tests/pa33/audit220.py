#!/usr/bin/env python3
"""Independent PA33 source/ABI trace, output identity and command failures."""
import hashlib,json,os,pathlib,subprocess,sys
ROOT=pathlib.Path(__file__).resolve().parents[2]
OUT=pathlib.Path(sys.argv[1]).resolve();OUT.mkdir(parents=True,exist_ok=True)
A=pathlib.Path(os.environ['RALPH_ARTIFACT_DIR'])/'pa33-219/final'
rows=[]
def run(args,success=True,env=None):
    args=list(map(str,args))
    p=subprocess.run(args,cwd=ROOT,capture_output=True,text=True,timeout=90,env=env)
    rows.append(dict(command=args,status=p.returncode,stdout=p.stdout,stderr=p.stderr))
    (OUT/'checks.json').write_text(json.dumps(rows,indent=2)+'\n')
    assert (p.returncode==0)==success,rows[-1]
    return p
src=ROOT/'student.tests/pa33/audit-trace.cpp'
for level in range(4):
    ir=OUT/f'trace{level}.lowir';obj=OUT/f'trace{level}.o'
    run(['dev/cppgm++','--emit-lowir','--validate-lowir','-gline-tables-only',f'-O{level}','--stats','-o',ir,src])
    run(['dev/cppgm++','-c','-gline-tables-only',f'-O{level}','--stats','-o',obj,src])
    replay=OUT/f'replay{level}.o'
    run(['dev/cppgm++','-c',f'-O{level}','-o',replay,OUT/'trace0.lowir'])
    assert obj.read_bytes()==replay.read_bytes(),('replay',level)
    before=OUT/f'entry{level}.o'
    run([A/'cppgm++','-c','-gline-tables-only',f'-O{level}','-o',before,src])
    assert obj.read_bytes()==before.read_bytes(),('refactor',level)
    exe=OUT/f'trace{level}'
    run(['g++',obj,'-o',exe])
    for args in [[],['a'],['a','b','c']]:run([exe,*args])
    mir=OUT/f'trace{level}.mir'
    run(['dev/lowir2native',f'-O{level}','--stats','--dump-machine-ir',mir,ir])
    assert 'audit-trace.cpp' in mir.read_text()
    assert ('strlen_prefix=16' in mir.read_text())==bool(level)
    assert 'copy_bytes_dynamic' in mir.read_text()
    for args in [['readelf','-SW',obj],['readelf','-Ws',obj],['readelf','-wf',obj],['objdump','-dr',obj]]:run(args)
    # One selector invocation must yield the same MIR/ELF as single outputs.
    simple=ROOT/'pa33/tests/o1/100-return-copy-coalesce.t'
    dump=OUT/f'combined{level}.mir';image=OUT/f'combined{level}'
    run(['dev/lowir2native',f'-O{level}','--dump-machine-ir',dump,'-o',image,simple])
    run([image])
    single=OUT/f'single{level}.mir';only=OUT/f'only{level}'
    run(['dev/lowir2native',f'-O{level}','--dump-machine-ir',single,simple])
    run(['dev/lowir2native',f'-O{level}','-o',only,simple])
    assert dump.read_bytes()==single.read_bytes() and image.read_bytes()==only.read_bytes()
for args in [[],['-O2'],[simple],['-o',OUT/'bad'],['-o'],['--dump-machine-ir'],['--target'],['--target','bad'],
             ['-o',OUT/'bad','/does/not/exist'],['--dump-machine-ir',OUT/'absent/out',simple],
             ['--dump-machine-ir','/dev/full',simple]]:
    run(['dev/lowir2native',*args],False)
for flag in ['--help','-h']:run(['dev/lowir2native',flag])
for name,text in [('invalid','invalid lowir'),('conflict','declare function @f(%s : ptr) -> i64 [object=cppgm_builtin_strlen, builtin=memcpy]'),
                  ('duplicate','declare function @f(%s : ptr) -> i64 [builtin=strlen, builtin=strlen]'),
                  ('unknown','declare function @f(%s : ptr) -> i64 [builtin=unknown]'),
                  ('undefined','declare function @missing() -> i32\nfunction @f() -> i32 [role=entry] { block ^b: %v = call i32 @missing() return i32 %v }')]:
    p=OUT/(name+'.lowir');p.write_text(text+'\n')
    run(['dev/lowir2native','-O2','-o',OUT/'bad',p],False)
env=dict(os.environ,RALPH_ARTIFACT_DIR=str(OUT/'personal'))
for script in ['check_native.py','check_budgets.py']:run(['python3',ROOT/'student.tests/pa33'/script],env=env)
print('PA33 independent source/template/builtin trace, ABI/output identity, CLI and personal controls: PASS')
