#!/usr/bin/env python3
"""Bind the new loop proof to source/template lowering, debug MIR and ELF replay."""
import hashlib,json,pathlib,subprocess,sys
root=pathlib.Path(__file__).resolve().parents[2]
out=pathlib.Path(sys.argv[1]).resolve();out.mkdir(parents=True,exist_ok=True)
source=root/'student.tests/pa32/loop-trace.cpp';records=[]
def run(args):
 command=list(map(str,args));r=subprocess.run(command,capture_output=True,text=True,timeout=60,cwd=root)
 records.append(dict(command=command,exit_code=r.returncode,stdout=r.stdout,stderr=r.stderr))
 (out/'trace.json').write_text(json.dumps(records,indent=2)+'\n')
 assert r.returncode==0,(command,r.stderr[-2000:]);return r
for level in range(4):
 ir=out/f'trace{level}.lowir';direct=out/f'direct{level}.o';replay=out/f'replay{level}.o';exe=out/f'exe{level}'
 r=run(['dev/cppgm++','--emit-lowir','--validate-lowir','-gline-tables-only',f'-O{level}','--stats','-o',ir,source])
 if level==3:
  stats=next(json.loads(s) for s in r.stderr.splitlines() if s.startswith('{') and 'loops_unrolled' in s)
  assert stats['loops_unrolled']>=1,stats
 run(['dev/cppgm++','-c','-gline-tables-only',f'-O{level}','-o',direct,source])
 run(['dev/cppgm++','-c',f'-O{level}','-o',replay,out/'trace0.lowir'])
 assert direct.read_bytes()==replay.read_bytes(),(level,'ELF replay mismatch')
 run(['g++',direct,'-o',exe]);run([exe]);run([exe,'runtime-input'])
 mir=out/f'trace{level}.mir';run(['dev/lowir2native','--dump-machine-ir',mir,'--stats',ir])
 assert 'loop-trace.cpp' in mir.read_text()
 run(['objdump','-d',direct]);run(['readelf','-SW',direct])
(out/'source.sha256').write_text(hashlib.sha256(source.read_bytes()).hexdigest()+'\n')
print('loop source/template fact trace PASS: O0-O3 validation, two runtime inputs, debug MIR and identical direct/replayed ELF')
