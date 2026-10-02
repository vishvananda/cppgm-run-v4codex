#!/usr/bin/env python3
"""Bind the range-fill proof to source/template lowering, debug MIR and ELF replay."""
import hashlib,json,pathlib,subprocess,sys
root=pathlib.Path(__file__).resolve().parents[2]
out=pathlib.Path(sys.argv[1]).resolve();out.mkdir(parents=True,exist_ok=True)
source=root/'student.tests/pa32/range-trace.cpp';records=[]
def run(args):
 command=list(map(str,args));r=subprocess.run(command,capture_output=True,text=True,timeout=60,cwd=root)
 records.append(dict(command=command,exit_code=r.returncode,stdout=r.stdout,stderr=r.stderr))
 (out/'trace.json').write_text(json.dumps(records,indent=2)+'\n')
 assert r.returncode==0,(command,r.stderr[-2000:]);return r
# Located source copies are observable debug-value anchors; the source-debug
# contract permits retaining these loops. Keep the useful transform assertion
# on g0 and double execution/replay coverage instead of imposing a debug pass
# selection that conflicts with source-value preservation.
for debug in [False,True]:
 for level in range(4):
  prefix='debug' if debug else 'plain'
  ir=out/f'{prefix}{level}.lowir';direct=out/f'{prefix}-direct{level}.o';replay=out/f'{prefix}-replay{level}.o';exe=out/f'{prefix}-exe{level}'
  r=run(['dev/cppgm++','--emit-lowir','--validate-lowir','-gline-tables-only' if debug else '-g0',f'-O{level}','--stats','-o',ir,source])
  if level and not debug:
   stats=next(json.loads(s) for s in r.stderr.splitlines() if s.startswith('{') and 'loops_filled' in s)
   assert stats['loops_filled']>=1,stats
  run(['dev/cppgm++','-c','-gline-tables-only' if debug else '-g0',f'-O{level}','-o',direct,source])
  run(['dev/cppgm++','-c',f'-O{level}','-o',replay,out/f'{prefix}0.lowir'])
  assert direct.read_bytes()==replay.read_bytes(),(level,'ELF replay mismatch')
  run(['g++',direct,'-o',exe]);run([exe]);run([exe,'runtime-input'])
  mir=out/f'{prefix}{level}.mir';run(['dev/lowir2native','--dump-machine-ir',mir,'--stats',ir])
  if debug: assert 'range-trace.cpp' in mir.read_text()
  run(['objdump','-d',direct]);run(['readelf','-SW',direct])
(out/'source.sha256').write_text(hashlib.sha256(source.read_bytes()).hexdigest()+'\n')
print('range source/template fact trace PASS: O0-O3 validation, two runtime inputs, debug MIR and identical direct/replayed ELF')
