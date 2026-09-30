#!/usr/bin/env python3
"""Reproduce the standalone exception RTTI duplicate-label limit on reference IR."""
from pathlib import Path
import json,re,subprocess,hashlib,sys
ROOT=Path(__file__).resolve().parents[2];WORK=Path(sys.argv[1]).resolve();WORK.mkdir(parents=True,exist_ok=True)
controls=json.loads((ROOT/'student.tests/pa23/controls122.json').read_text())
case=next(c for c in controls['cases'] if c['name']=='delete-throwing-secondary')
src=WORK/'delete-throwing-secondary.cpp';src.write_text(case['sources'][0]);ir=src.with_suffix('.lowir');exe=src.with_suffix('.exe');obj=src.with_suffix('.o')
def invoke(cmd):
 p=subprocess.run([str(x) for x in cmd],capture_output=True,text=True,timeout=30);return dict(exit=p.returncode,diagnostic=p.stderr)
result=dict(bundle_revision='c2f713cd70d06170632bfde3e75dd6fe1aa44d98',source=src.read_text(),reference_compile=invoke([ROOT/'dev/cppgm++-ref','--emit-lowir','-O0','-o',ir,src]))
assert result['reference_compile']['exit']==0
result['reference_ir_sha256']=hashlib.sha256(ir.read_bytes()).hexdigest()
result['standalone']=invoke([ROOT/'dev/lowir2native-ref','-O0','-o',exe,ir])
# The earlier handoff already identified private name collisions. Observe the
# remaining object-label limit with only private names changed in scratch IR.
probe=WORK/'private-name-probe.lowir';counter=[0]
def rename(m):counter[0]+=1;return 'object=__cppgm_probe_'+str(counter[0])
probe.write_text(re.sub(r'object=@[^,\]\s]+',rename,ir.read_text()))
result['private_name_probe']=invoke([ROOT/'dev/lowir2native-ref','-O0','-o',exe,probe])
result['hosted_backend']=invoke([ROOT/'dev/cppgm++-ref','-c','-O0','-o',obj,ir]);assert result['hosted_backend']['exit']==0
result['hosted_link']=invoke(['g++','-no-pie',obj,'-o',exe]);assert result['hosted_link']['exit']==0
result['hosted_execution']=invoke([exe]);assert result['hosted_execution']['exit']==0
assert result['standalone']['exit']!=0 and result['private_name_probe']['exit']!=0
print(json.dumps(result,indent=2))
