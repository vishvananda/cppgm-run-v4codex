#!/usr/bin/env python3
"""Frozen standard-grounded default declaration and demand proofs."""
from pathlib import Path
import json,shutil,subprocess,sys
from check_demand_regions import CASES
ROOT=Path(__file__).resolve().parents[2]
sys.path.insert(0,str(ROOT/'student.tests/pa10'))
import benchmark as shared
A,B,WORK,OUT=map(Path,sys.argv[1:5]);WORK.mkdir(parents=True,exist_ok=True)
binaries=[A.resolve(),B.resolve()]
result=dict(harness_sha256=shared.sha(__file__),rejection_harness_sha256=shared.sha(ROOT/'student.tests/pa14/check_demand_regions.py'),
 rules=['N3485 [dcl.fct.default]/4-6','N3485 [temp.inst]/1-2','https://cplusplus.github.io/CWG/issues/15.html','https://cplusplus.github.io/CWG/issues/217.html'],
 binaries=[dict(path=str(p),sha256=shared.sha(p)) for p in binaries],rejections=[],positives=[])
for i,source in enumerate(CASES):
 src=WORK/f'reject-{i}.cpp';src.write_text(source);row=dict(source_path=str(src),source_sha256=shared.sha(src),outputs=[])
 for b,binary in enumerate(binaries):
  log=WORK/f'reject-{i}-{b}.log';ir=WORK/f'reject-{i}-{b}.lowir';command=[binary,'--emit-lowir','-O0','-o',ir,src]
  r=subprocess.run(list(map(str,command)),capture_output=True,text=True,timeout=60);log.write_text(r.stdout+r.stderr)
  assert r.returncode in (0,1) and (b==0 or r.returncode==1),(i,b,r.returncode,r.stderr)
  row['outputs'].append(dict(binary=b,exit=r.returncode,command=list(map(str,command)),log_path=str(log),log_sha256=shared.sha(log)))
 result['rejections'].append(row)
for name in ('demand-regions.cpp','default-heads.t'):
 src=WORK/name;shutil.copy2(ROOT/'student.tests/pa14'/name,src);row=dict(source_path=str(src),source_sha256=shared.sha(src),outputs=[])
 for b,binary in enumerate(binaries):
  ir=WORK/f'{src.stem}-{b}.lowir';log=WORK/f'{src.stem}-{b}.log';exe=WORK/f'{src.stem}-{b}';command=[binary,'--emit-lowir','-O0','--validate-lowir','-o',ir,src]
  r=subprocess.run(list(map(str,command)),capture_output=True,text=True,timeout=60);log.write_text(r.stdout+r.stderr)
  assert r.returncode==(1 if b==0 else 0),(name,b,r.returncode,r.stderr)
  item=dict(binary=b,exit=r.returncode,command=list(map(str,command)),log_path=str(log),log_sha256=shared.sha(log))
  if b==1:
   shared.run([ROOT/'dev/lowir2native-ref','-O0','-o',exe,ir]);shared.run([exe])
   item.update(path=str(ir),sha256=shared.sha(ir),native_path=str(exe),native_sha256=shared.sha(exe),native_exit=0)
  row['outputs'].append(item)
 result['positives'].append(row)
OUT.write_text(json.dumps(result,indent=2)+'\n');print(len(CASES),'rejection and two new native proofs retained')
