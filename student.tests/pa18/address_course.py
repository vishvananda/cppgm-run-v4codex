#!/usr/bin/env python3
"""Execute fixed course inputs, comparing outcomes to their preserved LowIR oracle."""
from pathlib import Path
import hashlib,json,re,subprocess,sys
ROOT=Path(__file__).resolve().parents[2]
ENTRY,FINAL,WORK=map(Path,sys.argv[1:]);WORK.mkdir(parents=True,exist_ok=True)
def failures(p):return set(re.findall(r'pa18/[^\s:]+\.t',p.read_text()))
rows=[]
for i,name in enumerate(sorted(failures(ENTRY)-failures(FINAL))):
 src=ROOT/name;ir=WORK/(str(i)+'.lowir')
 row=dict(path=name,source_sha256=hashlib.sha256(src.read_bytes()).hexdigest())
 r=subprocess.run([ROOT/'dev/cppgm++','--emit-lowir','-O0','--validate-lowir','-o',ir,src],capture_output=True,text=True,timeout=30)
 row.update(compiler_exit=r.returncode,diagnostic=r.stderr)
 lowir_only='main(' not in src.read_text() and 'main (' not in src.read_text()
 row['oracle']='validated-lowir (source has no entry point)' if lowir_only else 'same executable exit as checked reference'
 row['passed']=r.returncode==0
 if not lowir_only and not r.returncode:
  results=[]
  for j,input in enumerate((src.with_suffix('.ref'),ir)):
   exe=WORK/(str(i)+'-'+str(j)+'.exe')
   b=subprocess.run([ROOT/'dev/lowir2native-ref','-O0','-o',exe,input],capture_output=True,text=True,timeout=30)
   e=subprocess.run([exe],timeout=30).returncode if not b.returncode else None
   results.append(dict(backend_exit=b.returncode,diagnostic=b.stderr,execution_exit=e))
  row['executions']=results;row['passed']=all(x['backend_exit']==0 for x in results) and results[0]['execution_exit']==results[1]['execution_exit']
 rows.append(row);print(name,'PASS' if row['passed'] else 'FAIL',flush=True)
(WORK/'results.json').write_text(json.dumps(rows,indent=2)+'\n')
assert all(x['passed'] for x in rows),rows
