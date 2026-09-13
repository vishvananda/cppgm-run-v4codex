#!/usr/bin/env python3
"""Retain source/constant/ABI ownership proofs, including prior incorrect acceptance."""
from pathlib import Path
import json,shutil,subprocess,sys
from check_body_values import CASES as BODY
from check_value_queries import CASES as BOUNDS
ROOT=Path(__file__).resolve().parents[2]
sys.path.insert(0,str(ROOT/'student.tests/pa10'))
import benchmark as shared
A,B,WORK,OUT=map(Path,sys.argv[1:5]);WORK.mkdir(parents=True,exist_ok=True)
binaries=[A.resolve(),B.resolve()]
result=dict(harness_sha256=shared.sha(__file__),rejection_harnesses=[dict(path=str(ROOT/'student.tests/pa14'/name),sha256=shared.sha(ROOT/'student.tests/pa14'/name)) for name in ('check_body_values.py','check_value_queries.py')],
 rules=['N3485 [expr.sizeof]/1,6','N3485 [temp.dep.constexpr]/2','N3485 [dcl.array]/1','N3485 [dcl.fct]/5','N3485 [expr.cond]','N3485 [expr.const]/2','https://itanium-cxx-abi.github.io/cxx-abi/abi.html#mangling-expressions'],
 binaries=[dict(path=str(p),sha256=shared.sha(p)) for p in binaries],rejections=[],positives=[])
for i,source in enumerate(BODY+BOUNDS):
 src=WORK/f'reject-{i}.cpp';src.write_text(source);row=dict(source_path=str(src),source_sha256=shared.sha(src),outputs=[])
 for b,binary in enumerate(binaries):
  log=WORK/f'reject-{i}-{b}.log';command=[binary,'--emit-lowir','-O0','-o',WORK/f'reject-{i}-{b}.lowir',src]
  r=subprocess.run(list(map(str,command)),capture_output=True,text=True,timeout=60);log.write_text(r.stdout+r.stderr)
  assert r.returncode in (0,1) and (b==0 or r.returncode==1),(i,b,r.returncode,r.stderr)
  row['outputs'].append(dict(binary=b,exit=r.returncode,command=list(map(str,command)),log_path=str(log),log_sha256=shared.sha(log)))
 result['rejections'].append(row)
for name in ('body-values.cpp','dependent-bounds.cpp','value-conversion.t'):
 src=WORK/name;shutil.copy2(ROOT/'student.tests/pa14'/name,src);row=dict(source_path=str(src),source_sha256=shared.sha(src),outputs=[])
 for b,binary in enumerate(binaries):
  ir=WORK/f'{src.stem}-{b}.lowir';log=WORK/f'{src.stem}-{b}.log';exe=WORK/f'{src.stem}-{b}';command=[binary,'--emit-lowir','-O0','--validate-lowir','-o',ir,src]
  r=subprocess.run(list(map(str,command)),capture_output=True,text=True,timeout=60);log.write_text(r.stdout+r.stderr)
  expected=1 if b==0 and name=='dependent-bounds.cpp' else 0
  assert r.returncode==expected,(name,b,r.returncode,r.stderr)
  item=dict(binary=b,exit=r.returncode,command=list(map(str,command)),log_path=str(log),log_sha256=shared.sha(log))
  if not r.returncode:
   shared.run([ROOT/'dev/lowir2native-ref','-O0','-o',exe,ir]);shared.run([exe])
   item.update(path=str(ir),sha256=shared.sha(ir),native_path=str(exe),native_sha256=shared.sha(exe),native_exit=0)
  row['outputs'].append(item)
 if name!='dependent-bounds.cpp':
  assert len(set(out['sha256'] for out in row['outputs']))==1
  assert len(set(out['native_sha256'] for out in row['outputs']))==1
 result['positives'].append(row)
OUT.write_text(json.dumps(result,indent=2)+'\n');print(len(BODY+BOUNDS),'rejections,',sum(r['outputs'][0]['exit']==0 for r in result['rejections']),'new rejections; three native proofs retained')
