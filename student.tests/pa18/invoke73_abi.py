#!/usr/bin/env python3
"""Source this/invoke ABI identities and fact roundtrips. TOOL API CONTROLS WORK."""
from pathlib import Path
import hashlib,json,subprocess,sys
TOOL,API,CONTROLS,WORK=map(Path,sys.argv[1:]);WORK.mkdir(parents=True,exist_ok=True)
rows=[]
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
# doc/itanium-mangling.txt: <function-param> ::= fpT; DT <expression> E.
source='let-expr Self this\nlet-type Result decltype Self\nfunction f Result Result\n'
src=WORK/'this.facts';src.write_text(source)
for tool in (TOOL.resolve(),API.resolve()):
 out=WORK/'this.names';cmd=[tool,'-o',out,src] if tool==TOOL.resolve() else [tool,src]
 r=subprocess.run(cmd,capture_output=True,text=True,timeout=30)
 output=out.read_text() if tool==TOOL.resolve() and r.returncode==0 else r.stdout
 rows.append(dict(name='this-expression-roundtrip',tool=str(tool),source=source,exit=r.returncode,output=output,diagnostic=r.stderr,expected='_Z1fDTfpTES_\n',passed=r.returncode==0 and output=='_Z1fDTfpTES_\n'))
# Check the production query projection, including its source intrinsic name.
for name,part in [('pack_forwarding','16__builtin_invoke'),('fixed_direct','_Z4callIiEiT_')]:
 path=CONTROLS/(name+'.lowir');data=path.read_text()
 rows.append(dict(name=name,path=str(path),sha256=sha(path),expected_fragment=part,passed=part in data))
(WORK/'results.json').write_text(json.dumps(rows,indent=2)+'\n')
assert all(r['passed'] for r in rows),rows
print('Four ABI checks pass: fpT fact roundtrips and source-faithful invocation query names.')
