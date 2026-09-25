#!/usr/bin/env python3
"""Itanium tl/il grammar, graph fact roundtrips and source projection: TOOL API CONTROLS WORK."""
from pathlib import Path
import hashlib,json,subprocess,sys
TOOL,API,CONTROLS,WORK=map(Path,sys.argv[1:]);WORK.mkdir(parents=True,exist_ok=True)
rows=[]
# doc/itanium-mangling.txt: tl <type> <expression>* E; il <expression>* E;
# DT <expression> E wraps decltype, and the repeated parameter substitutes S_.
for name,source,expected in [
 ('typed','let-expr One value int 1\nlet-expr List init-list int One\n','_Z1fDTtliLi1EEES_\n'),
 ('bare','let-expr One value int 1\nlet-expr List init-list - One\n','_Z1fDTilLi1EEES_\n'),
 ('empty','let-expr List init-list int\n','_Z1fDTtliEES_\n')]:
 source+='let-type Result decltype List\nfunction f Result Result\n';src=WORK/(name+'.facts');src.write_text(source)
 for tool in (TOOL.resolve(),API.resolve()):
  out=WORK/(name+'.names');cmd=[tool,'-o',out,src] if tool==TOOL.resolve() else [tool,src]
  r=subprocess.run(cmd,capture_output=True,text=True,timeout=30)
  output=out.read_text() if tool==TOOL.resolve() and r.returncode==0 else r.stdout
  rows.append(dict(name=name,tool=str(tool),source=source,exit=r.returncode,output=output,diagnostic=r.stderr,expected=expected,passed=r.returncode==0 and output==expected))
path=CONTROLS/'typed_list_abi.lowir';data=path.read_text()
rows.append(dict(name='typed-source-projection',path=str(path),sha256=hashlib.sha256(path.read_bytes()).hexdigest(),expected_fragment='tlT_fp_E',passed='tlT_fp_E' in data))
(WORK/'results.json').write_text(json.dumps(rows,indent=2)+'\n')
assert all(r['passed'] for r in rows),rows
print('Seven ABI checks pass: tl/il fact roundtrips and source-faithful typed list query names.')
