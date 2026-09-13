#!/usr/bin/env python3
"""Preserve entry/current evidence; language proof comes from N3485, not compiler agreement."""
from pathlib import Path
import hashlib,json,os,subprocess,sys
ROOT=Path(__file__).resolve().parents[2]
A,B,CONTROLS,WORK,OUT=map(lambda x:Path(x).resolve(),sys.argv[1:6]);WORK.mkdir(parents=True,exist_ok=True)
def sha(p):return hashlib.sha256(Path(p).read_bytes()).hexdigest()
def binary(p):return dict(path=str(p),sha256=sha(p))
controls=json.loads(CONTROLS.read_text());assert controls['binary']['sha256']==sha(B)
result=dict(harness=binary(__file__),control_manifest=binary(CONTROLS),standard=binary(ROOT/'doc/n3485.txt'),binaries=[binary(A),binary(B)],
 rules=['N3485 13.3.1.3 [over.match.ctor]/1: copy candidates are converting constructors; direct candidates are all constructors.',
 'N3485 13.3.1.4 [over.match.copy]/1: copy initialization combines converting constructors and eligible non-explicit conversion functions.',
 'N3485 13.3.1.7 [over.match.list]/1: copy-list selects among all constructors and rejects an explicit winner.',
 'N3485 8.5.1 [dcl.init.aggr]/2,7: elements are copy-initialized and omitted elements use an empty initializer list.',
 'N3485 12.8 [class.copy]/31-32: elision requires valid copy/move selection; eligible returns first use an rvalue source.',
 'N3485 14.6 [temp.res]/8 and PA14 definition-time supported-body checking apply fixed obligations before instantiation.'],cases=[])
for row in controls['checks']:
 src=Path(row['source_path']);assert sha(src)==row['source_sha256']
 ir=WORK/(row['name']+'.lowir');log=WORK/(row['name']+'.log')
 p=subprocess.run([str(A),'--emit-lowir','-O0','-o',str(ir),str(src),'--validate-lowir'],capture_output=True,text=True)
 log.write_text(p.stdout+p.stderr)
 item=dict(name=row['name'],source=binary(src),required_exit=int(row['reject']),current_exit=row['exit_code'],entry_exit=p.returncode,entry_log=binary(log))
 if not row['reject'] and p.returncode==0:
  exe=WORK/row['name'];p=subprocess.run([str(ROOT/'dev/lowir2native-ref'),'-O0','-o',str(exe),str(ir)],capture_output=True,text=True)
  assert p.returncode==0,p.stderr
  p=subprocess.run([str(exe)],timeout=10)
  item['entry_native']=dict(**binary(exe),exit_code=p.returncode)
 if 'native' in row:item['current_native']=row['native']
 result['cases'].append(item)
OUT.write_text(json.dumps(result,indent=2)+'\n')
print(len(result['cases']),'language controls preserved;',sum(r['entry_exit']!=r['required_exit'] for r in result['cases']),'entry status defects;',sum(r.get('entry_native',{}).get('exit_code',0)!=0 for r in result['cases']),'entry native failures')
