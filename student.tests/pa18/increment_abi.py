#!/usr/bin/env python3
"""Exact increment/decrement ABI grammar and graph reader/writer roundtrips.
Proof: doc/itanium-mangling.txt unary-expression and prefix productions (lines 511-515).
Run TOOL API WORK; no host-compiler ABI oracle.
"""
from pathlib import Path
import json, subprocess, sys
TOOL,API,WORK=map(Path,sys.argv[1:]);TOOL=TOOL.resolve();API=API.resolve();WORK.mkdir(parents=True,exist_ok=True)
CASES=[]
for code in ('pp','pp_','mm','mm_'):
 CASES.append((code,'let-expr P template-param 0\nlet-expr S unary '+code+' P\nlet-type T decltype S\nfunction f T T\n','_Z1fDT'+code+'T_ES_\n'))

rows=[]
for name,source,expected in CASES:
 p=WORK/(name+'.facts');p.write_text(source);out=p.with_suffix('.names')
 for tool in (TOOL,API):
  command=[tool,'-o',out,p] if tool==TOOL else [tool,p]
  r=subprocess.run(command,capture_output=True,text=True,timeout=30)
  text=out.read_text() if tool==TOOL and r.returncode==0 else r.stdout
  passed=(r.returncode==0 and text==expected) if expected is not None else r.returncode!=0
  rows.append(dict(name=name,tool=str(tool),source=source,expected=expected,exit=r.returncode,output=text,diagnostic=r.stderr,passed=passed))
  print(name,tool.name,'PASS' if passed else 'FAIL',flush=True)
(WORK/'results.json').write_text(json.dumps(rows,indent=2)+'\n')
sys.exit(0 if all(r['passed'] for r in rows) else 1)
