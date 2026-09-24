#!/usr/bin/env python3
"""Exact sizeof-pack ABI grammar and graph reader/writer roundtrips.
Proof: doc/itanium-mangling.txt expression productions sZ and sP (lines 544-546).
Run TOOL API WORK; no host-compiler ABI oracle.
"""
from pathlib import Path
import json, subprocess, sys
TOOL,API,WORK=map(Path,sys.argv[1:]);TOOL=TOOL.resolve();API=API.resolve();WORK.mkdir(parents=True,exist_ok=True)
CASES=[
 ('template','let-expr P template-param 0\nlet-expr S sizeof-pack P\nlet-type T decltype S\nfunction f T T\n','_Z1fDTsZT_ES_\n'),
 ('function','let-expr P function-param 1\nlet-expr S sizeof-pack P\nlet-type T decltype S\nfunction f T\n','_Z1fDTsZfp0_E\n'),
 ('captured','let-arg A type int\nlet-arg B type long\nlet-expr S sizeof-captured-pack A B\nlet-type T decltype S\nfunction f T T\n','_Z1fDTsPilEES_\n'),
 ('empty','let-expr S sizeof-captured-pack\nlet-type T decltype S\nfunction f T\n','_Z1fDTsPEE\n'),
 ('not-parameter','let-expr P value int 3\nlet-expr S sizeof-pack P\nlet-type T decltype S\nfunction f T\n',None),
]
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
