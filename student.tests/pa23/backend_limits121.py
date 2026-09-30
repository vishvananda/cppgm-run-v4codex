#!/usr/bin/env python3
"""Observe supplied standalone-backend limits without changing checked fixtures."""
from pathlib import Path
import hashlib,json,os,re,subprocess
ROOT=Path(__file__).resolve().parents[2];ART=Path(os.environ['RALPH_ARTIFACT_DIR'])/'pa23-121';WORK=ART/'backend-limits';WORK.mkdir(exist_ok=True)
rows=[]
for name in ('crosscast-null-and-miss','crosscast-private-base','exception-rtti'):
 src=ART/'controls-native'/(name+'.cpp');ir=WORK/(name+'.lowir');exe=ir.with_suffix('.exe')
 p=subprocess.run([str(ROOT/'dev/cppgm++-ref'),'--emit-lowir','-O0','-o',str(ir),str(src)],capture_output=True,text=True)
 row=dict(name=name,source=src.read_text(),reference_compile_exit=p.returncode,reference_diagnostic=p.stderr)
 if not p.returncode:
  row['reference_ir_sha256']=hashlib.sha256(ir.read_bytes()).hexdigest()
  p=subprocess.run([str(ROOT/'dev/lowir2native-ref'),'-O0','-o',str(exe),str(ir)],capture_output=True,text=True)
  row.update(raw_backend_exit=p.returncode,raw_backend_diagnostic=p.stderr)
  if not p.returncode:row['raw_runtime_exit']=subprocess.run([str(exe)]).returncode
  # Work around only the tool's duplicate private symbol aliases to observe
  # the runtime scan. Bodies, data, relocations, order and types are untouched.
  ir2=WORK/(name+'.private-names.lowir');count=[0]
  def rename(m):
   count[0]+=1;return 'object=__probe_private_'+str(count[0])
  fixed=re.sub(r'object=@[^,\]\s]+',rename,ir.read_text());ir2.write_text(fixed)
  row['private_object_name_changes']=count[0];row['probe_ir_sha256']=hashlib.sha256(ir2.read_bytes()).hexdigest()
  p=subprocess.run([str(ROOT/'dev/lowir2native-ref'),'-O0','-o',str(exe),str(ir2)],capture_output=True,text=True)
  row.update(probe_backend_exit=p.returncode,probe_backend_diagnostic=p.stderr)
  if not p.returncode:row['probe_runtime_exit']=subprocess.run([str(exe)],timeout=20).returncode
 if not row['reference_compile_exit']:
  obj=ir.with_suffix('.o');host=ir.with_suffix('.host')
  q=subprocess.run([str(ROOT/'dev/cppgm++-ref'),'-c','-O0','-o',str(obj),str(ir)],capture_output=True,text=True)
  row['hosted_backend_exit']=q.returncode
  if not q.returncode:
   q=subprocess.run(['g++','-no-pie',str(obj),'-o',str(host)],capture_output=True,text=True);row['hosted_link_exit']=q.returncode
   if not q.returncode:row['hosted_runtime_exit']=subprocess.run([str(host)],timeout=20).returncode
 rows.append(row)
(ROOT/'student.tests/pa23/backend-limits121.json').write_text(json.dumps(dict(bundle_source='c2f713cd70d06170632bfde3e75dd6fe1aa44d98',cases=rows),indent=2)+'\n')
print([(r['name'],r.get('raw_backend_diagnostic'),r.get('probe_backend_exit'),r.get('probe_runtime_exit')) for r in rows])
assert all(r.get('hosted_runtime_exit')==0 and r.get('probe_runtime_exit')==1 for r in rows)
