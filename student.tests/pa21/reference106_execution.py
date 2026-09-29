#!/usr/bin/env python3
"""Execute original/revised LowIR independently of student emission."""
from pathlib import Path
import hashlib,json,subprocess,sys
ROOT=Path(__file__).resolve().parents[2]
WORK=Path(sys.argv[1]).resolve();WORK.mkdir(parents=True,exist_ok=True)
manifest=json.loads((ROOT/'student.tests/pa21/reference106-revision.json').read_text())
def run(argv):
 p=subprocess.run(list(map(str,argv)),cwd=ROOT,capture_output=True,text=True,timeout=30)
 return dict(argv=list(map(str,argv)),exit=p.returncode,stdout=p.stdout,stderr=p.stderr)
rows=[]
for record in manifest['files']:
 for original in (True,False):
  name=Path(record['path']).stem+('-original' if original else '-revised')
  ir=WORK/(name+'.lowir');obj=ir.with_suffix('.o');exe=WORK/name
  data=subprocess.check_output(['git','show',manifest['base']+':'+record['path']],cwd=ROOT) if original else (ROOT/record['path']).read_bytes()
  ir.write_bytes(data)
  assert hashlib.sha256(data).hexdigest()==record['original_sha256' if original else 'revised_sha256']
  commands=[]
  for cmd in ([ROOT/'dev/cppgm++-ref','-c','-O0','-o',obj,ir],['g++','-no-pie',obj,'-o',exe],[exe]):
   commands.append(run(cmd))
   if commands[-1]['exit']:break
  assert (commands[-1]['exit']!=0)==original,(name,commands)
  rows.append(dict(name=name,commands=commands,lowir_sha256=hashlib.sha256(data).hexdigest()))
result=dict(revision=manifest['revision'],proof='pa21/reference-corrections106.md',rows=rows)
(WORK/'results.json').write_text(json.dumps(result,indent=2)+'\n')
print(json.dumps(result,indent=2))
