#!/usr/bin/env python3
"""Execute the completed ordering group, including array cases with open IR diffs."""
from pathlib import Path
import hashlib,json,subprocess,sys
ROOT=Path(__file__).resolve().parents[2]
CC=Path(sys.argv[1]).resolve();WORK=Path(sys.argv[2]);WORK.mkdir(parents=True,exist_ok=True)
CASES=['general/200-function-template-partial-order-class-template-cv',
 'general/200-function-template-partial-order-const-pointer','general/200-function-template-trailing-pack-partial-order',
 'general/200-partial-order-synthetic-virtual-member-emission','general/300-constructor-template-defaulted-forwarding-lvalue-order',
 'spec/100-function-address-prefers-nontemplate','spec/200-function-address-template-partial-ordering',
 'spec/200-function-template-fixed-parameter-default-tail-partial-order','spec/200-function-template-partial-order-const-pointer',
 'spec/200-instantiated-member-alias-cv-partial-order','spec/200-member-assignment-lvalue-beats-forwarding-ref',
 'spec/200-nested-alias-cv-partial-order','spec/300-dependent-decltype-pack-overload-replay',
 'general/200-nested-array-reference-partial-ordering','general/200-range-array-reference-mutable-begin',
 'spec/200-array-reference-cv-partial-ordering']
rows=[]
for i,name in enumerate(CASES):
 src=ROOT/'pa18/tests'/(name+'.t');ir=WORK/(str(i)+'.lowir');exe=WORK/(str(i)+'.exe')
 row=dict(path=str(src.relative_to(ROOT)),source_sha256=hashlib.sha256(src.read_bytes()).hexdigest())
 r=subprocess.run([CC,'--emit-lowir','-O0','--validate-lowir','-o',ir,src],capture_output=True,text=True,timeout=30)
 row.update(compiler_exit=r.returncode,diagnostic=r.stderr)
 # This contract fixture intentionally declares pick/pair/Wrap without definitions.
 # Its owning oracle is validated LowIR; it cannot form a linked executable.
 lowir_only=name=='spec/300-dependent-decltype-pack-overload-replay'
 row['oracle']='validated-lowir' if lowir_only else 'native-exit-zero'
 if r.returncode==0 and not lowir_only:
  b=subprocess.run([ROOT/'dev/lowir2native-ref','-O0','-o',exe,ir],capture_output=True,text=True,timeout=30)
  row.update(backend_exit=b.returncode,backend_diagnostic=b.stderr)
  if b.returncode==0:row['native_exit']=subprocess.run([exe],timeout=30).returncode
 row['passed']=(r.returncode==0 if lowir_only else row.get('native_exit')==0);rows.append(row);print(name,'PASS' if row['passed'] else 'FAIL',flush=True)
(WORK/'results.json').write_text(json.dumps(rows,indent=2)+'\n')
sys.exit(0 if all(r['passed'] for r in rows) else 1)
