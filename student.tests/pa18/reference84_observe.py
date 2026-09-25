#!/usr/bin/env python3
"""Reduced bundle observations and execution of independently revised oracles."""
from pathlib import Path
import hashlib,json,subprocess,sys
import object84_controls as controls
ROOT=Path(__file__).resolve().parents[2];W=Path(sys.argv[1]).resolve();W.mkdir(parents=True,exist_ok=True)
rows=[]
def sha(p):return hashlib.sha256(Path(p).read_bytes()).hexdigest()
def execute(name,ir):
 exe=W/(name+'.exe')
 v=subprocess.run([ROOT/'dev/lowir','-o',W/(name+'.roundtrip'),ir],capture_output=True,text=True)
 b=subprocess.run([ROOT/'dev/lowir2native-ref','-O0','-o',exe,ir],capture_output=True,text=True)
 native=subprocess.run([exe],timeout=10).returncode if b.returncode==0 else None
 return dict(validation_exit=v.returncode,backend_exit=b.returncode,backend_diagnostic=b.stderr,native_exit=native,passed=v.returncode==0 and b.returncode==0 and native==0)
for name in ('empty_value_parameter','empty_value_reference','empty_value_placement'):
 source=controls.runner.GOOD[name];src=W/(name+'.cpp');src.write_text(source)
 for label,cc in [('student',ROOT/'dev/cppgm++'),('reference',ROOT/'reference-binaries/cppgm++')]:
  ir=W/(name+'-'+label+'.lowir')
  r=subprocess.run([cc,'--emit-lowir','-O0','-o',ir,src],capture_output=True,text=True)
  assert r.returncode==0,(name,label,r.stderr)
  rows.append(dict(name=name+'-'+label,source=source,source_sha256=sha(src),compiler_sha256=sha(cc),compile_exit=r.returncode,lowir=ir.read_text(),lowir_sha256=sha(ir),**execute(name+'-'+label,ir)))
for row in json.loads((ROOT/'student.tests/pa18/reference84-revisions.json').read_text())['revisions']:
 ir=ROOT/row['path'];rows.append(dict(name=ir.stem,oracle_sha256=sha(ir),**execute(ir.stem,ir)))
(W/'results.json').write_text(json.dumps(rows,indent=2)+'\n')
assert all(r['passed'] for r in rows),rows
print('PASS:',len(rows),'checked reference/student observations and revised oracle executions')
