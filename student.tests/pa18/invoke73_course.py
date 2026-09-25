#!/usr/bin/env python3
"""Validate repaired course LowIR; execute where its reference can link.
CC WORK BASELINE FINAL. No source/fixture modification or external stubs.
"""
from pathlib import Path
import hashlib,json,subprocess,sys
ROOT=Path(__file__).resolve().parents[2]
cc=Path(sys.argv[1]).resolve();work=Path(sys.argv[2]);work.mkdir(parents=True,exist_ok=True)
def failures(p):return {s.split(': ERROR:')[0] for s in Path(p).read_text().splitlines() if '.t: ERROR:' in s}
fixed=failures(sys.argv[3])-failures(sys.argv[4]);rows=[]
for path in sorted(fixed):
 src=ROOT/path;ir=work/(src.stem+'.lowir');exe=ir.with_suffix('.exe')
 reject=src.with_suffix('.ref.exit_status').read_text().strip()=='EXIT_FAILURE'
 inputs=[src]
 if not reject and 'int main(' not in src.read_text():
  driver=work/'entry.cpp';driver.write_text('int main(){}\n');inputs.append(driver)
 cmd=[cc,'--emit-lowir','-O0','--validate-lowir','-o',ir,*inputs]
 r=subprocess.run(cmd,capture_output=True,text=True,timeout=30)
 row=dict(path=path,source_sha256=hashlib.sha256(src.read_bytes()).hexdigest(),expected='reject' if reject else 'execute',compiler_exit=r.returncode,diagnostic=r.stderr,passed=r.returncode==(1 if reject else 0))
 if not reject and r.returncode==0:
  b=subprocess.run([ROOT/'dev/lowir2native-ref','-O0','-o',exe,ir],capture_output=True,text=True,timeout=30)
  row.update(backend_exit=b.returncode,backend_diagnostic=b.stderr)
  row['native_exit']=subprocess.run([exe],timeout=30).returncode if not b.returncode else None
  row['passed']=b.returncode==row['native_exit']==0
 rows.append(row);print(path,row['passed'],flush=True)
(work/'results.json').write_text(json.dumps(rows,indent=2)+'\n')
assert len(rows)==7 and all(r['passed'] for r in rows)
