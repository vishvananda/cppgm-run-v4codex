#!/usr/bin/env python3
"""Retain independent rejection and positive native proofs for type ownership."""
from pathlib import Path
import json, shutil, subprocess, sys
from check_declaration_types import REJECTIONS
ROOT=Path(__file__).resolve().parents[2]
sys.path.insert(0,str(ROOT/'student.tests/pa10'))
import benchmark as shared
A,B,WORK,OUT=map(Path,sys.argv[1:5]); WORK.mkdir(parents=True,exist_ok=True)
binaries=[A.resolve(),B.resolve()]
result=dict(harness_sha256=shared.sha(__file__),rejection_harness_sha256=shared.sha(ROOT/'student.tests/pa14/check_declaration_types.py'),
 binaries=[dict(path=str(p),sha256=shared.sha(p)) for p in binaries],rejections=[],positives=[])
for i,source in enumerate(REJECTIONS):
 src=WORK/f'reject-{i}.cpp';src.write_text(source)
 row=dict(source_path=str(src),source_sha256=shared.sha(src),outputs=[])
 for b,binary in enumerate(binaries):
  log=WORK/f'reject-{i}-{b}.log';ir=WORK/f'reject-{i}-{b}.lowir'
  r=subprocess.run([binary,'--emit-lowir','-O0','-o',ir,src],capture_output=True,text=True,timeout=60)
  log.write_text(r.stdout+r.stderr)
  assert r.returncode in (0,1) and (b==0 or r.returncode==1),(i,b,r.returncode,r.stderr)
  row['outputs'].append(dict(binary=b,exit=r.returncode,log_path=str(log),log_sha256=shared.sha(log)))
 result['rejections'].append(row)
for name in ('signature-facts.cpp','declaration-types.cpp'):
 src=WORK/name;shutil.copy2(ROOT/'student.tests/pa14'/name,src)
 row=dict(source_path=str(src),source_sha256=shared.sha(src),outputs=[])
 for b,binary in enumerate(binaries):
  ir=WORK/f'{src.stem}-{b}.lowir';log=WORK/f'{src.stem}-{b}.log';exe=WORK/f'{src.stem}-{b}'
  r=subprocess.run([binary,'--emit-lowir','-O0','--validate-lowir','-o',ir,src],capture_output=True,text=True,timeout=60)
  log.write_text(r.stdout+r.stderr)
  assert r.returncode==0,(name,b,r.stderr)
  shared.run([ROOT/'dev/lowir2native-ref','-O0','-o',exe,ir]);shared.run([exe])
  row['outputs'].append(dict(binary=b,exit=0,path=str(ir),sha256=shared.sha(ir),log_path=str(log),log_sha256=shared.sha(log),native_path=str(exe),native_sha256=shared.sha(exe),native_exit=0))
 result['positives'].append(row)
OUT.write_text(json.dumps(result,indent=2)+'\n')
print(len(REJECTIONS),'rejection and two compiler/native proofs retained')
