#!/usr/bin/env python3
"""Execute repaired stage programs where they define an entry point: CC WORK LOG."""
from pathlib import Path
import sys,subprocess,json,hashlib
ROOT=Path(__file__).resolve().parents[2]
cc=Path(sys.argv[1]).resolve();work=Path(sys.argv[2]);work.mkdir(parents=True,exist_ok=True)
def failures(p):return {s.split(': ERROR:')[0] for s in Path(p).read_text().splitlines() if '.t: ERROR:' in s}
fixed=failures(work.parent/'entry-stage.log')-failures(sys.argv[3]);rows=[]
for path in sorted(fixed):
 src=ROOT/path;ir=work/(src.stem+'.lowir');exe=ir.with_suffix('.exe')
 cmd=[cc,'--emit-lowir','-O0','--validate-lowir','-o',ir,src];r=subprocess.run(cmd,capture_output=True,text=True)
 row=dict(path=path,source_sha256=hashlib.sha256(src.read_bytes()).hexdigest(),compiler_exit=r.returncode,diagnostic=r.stderr,passed=r.returncode==0)
 if r.returncode==0 and 'int main(' in src.read_text():
  b=subprocess.run([ROOT/'dev/lowir2native-ref','-O0','-o',exe,ir],capture_output=True,text=True)
  # Some contract compile-only inputs intentionally name undefined functions.
  row.update(backend_exit=b.returncode,backend_diagnostic=b.stderr)
  if b.returncode==0:
   row['native_exit']=subprocess.run([exe],timeout=30).returncode;row['passed'] &= row['native_exit']==0
  else:row['unlinked_compile_only']=True
 rows.append(row);print(path,row['passed'],flush=True)
(work/'results.json').write_text(json.dumps(rows,indent=2)+'\n')
assert all(r['passed'] for r in rows)
