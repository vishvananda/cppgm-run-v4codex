#!/usr/bin/env python3
"""Record reduced reference observations and execute all revised success fixtures."""
from pathlib import Path
import hashlib,json,subprocess,sys
import array83_controls as controls
ROOT=Path(__file__).resolve().parents[2];W=Path(sys.argv[1]).resolve();W.mkdir(parents=True,exist_ok=True)
rows=[]
def sha(p):return hashlib.sha256(Path(p).read_bytes()).hexdigest()
def run(name,source,cc,expected_compile,expected_native=None,validate=True):
 p=W/(name+'.cpp');p.write_text(source);ir=p.with_suffix('.lowir');exe=p.with_suffix('.exe')
 command=[cc,'--emit-lowir','-O0',*(['--validate-lowir'] if validate else []),'-o',ir,p]
 r=subprocess.run(command,capture_output=True,text=True,timeout=60)
 row=dict(name=name,source=source,source_sha256=sha(p),compiler=str(cc),compiler_sha256=sha(cc),command=[str(x) for x in command],compile_exit=r.returncode,diagnostic=r.stderr)
 ok=(r.returncode==0)==expected_compile
 if not r.returncode:
  row['ir_sha256']=sha(ir)
  r=subprocess.run([ROOT/'dev/lowir2native-ref','-O0','-o',exe,ir],capture_output=True,text=True,timeout=60)
  row['backend_exit']=r.returncode;row['backend_diagnostic']=r.stderr
  if not r.returncode:
   row['native_sha256']=sha(exe);row['native_exit']=subprocess.run([exe],capture_output=True,timeout=10).returncode
  if expected_native is not None:ok &= row.get('native_exit')==expected_native
 row['passed']=ok;rows.append(row)
 (W/'results.json').write_text(json.dumps(rows,indent=2)+'\n');print(name,ok,flush=True)
cc=ROOT/'dev/cppgm++';ref=ROOT/'reference-binaries/cppgm++'
for name in ('union_constant_order','array_identity','array_wide','array_constexpr_conversion','unknown_nested_flat','unknown_braced_string'):
 source=controls.runner.GOOD[name]
 run(name+'-student',source,cc,True,0)
 run(name+'-reference',source,ref,name!='array_constexpr_conversion',1 if name=='union_constant_order' else 0,False)
source=controls.runner.BAD['unknown_empty_pack']
run('empty-pack-student',source,cc,False)
run('empty-pack-reference',source,ref,True,None,False)
assert rows[-1]['backend_exit']==1 # The extension even has an invalid LowIR slot (1x4).
revisions=json.loads((ROOT/'student.tests/pa18/reference83-revisions.json').read_text())
for c in revisions['revisions']:
 if not c['path'].endswith('.ref'):continue
 p=ROOT/c['path'];source=p.with_suffix('.t').read_text()
 expected=1 if p.name=='200-alias-template-template-argument-use-scope.ref' else 0
 run(p.stem+'-student',source,cc,True,expected)
 # The independent revised oracle must validate and preserve fixture execution.
 ir=W/(p.stem+'-oracle.lowir');ir.write_bytes(p.read_bytes());exe=ir.with_suffix('.exe')
 validation=subprocess.run([ROOT/'dev/lowir','-o',ir.with_suffix('.roundtrip'),ir],capture_output=True,text=True)
 b=subprocess.run([ROOT/'dev/lowir2native-ref','-O0','-o',exe,ir],capture_output=True,text=True)
 native=subprocess.run([exe],timeout=10).returncode if not b.returncode else None
 row=dict(name=p.stem+'-oracle',path=str(p),sha256=sha(p),validation_exit=validation.returncode,backend_exit=b.returncode,backend_diagnostic=b.stderr,native_exit=native,passed=not validation.returncode and not b.returncode and native==expected)
 rows.append(row);print(row['name'],row['passed'],flush=True)
(W/'results.json').write_text(json.dumps(rows,indent=2)+'\n')
assert all(r['passed'] for r in rows),[r['name'] for r in rows if not r['passed']]
