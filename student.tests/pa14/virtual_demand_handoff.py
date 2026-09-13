#!/usr/bin/env python3
"""Record the completed virtual behavior group and the remaining fact boundary."""
from pathlib import Path
import hashlib,json,sys
ROOT=Path(__file__).resolve().parents[2]
WORK=Path(sys.argv[1]).resolve()
def sha(p):return hashlib.sha256(Path(p).read_bytes()).hexdigest()
def file(p):return dict(path=str(p),sha256=sha(p))
def doc(name):return json.loads((ROOT/'student.tests/pa14'/name).read_text())
data=doc('virtual-demand-performance.json');noise=doc('virtual-demand-noise.json')
checks=doc('virtual-demand-validation.json')['checks']
assert len(checks)==63 and all(r['exit_code']==0 for r in checks)
for row in checks:assert sha(row['log'])==row['log_sha256']
verification=WORK/'verification-combined.log'
assert '14504 total frozen performance observations verified' in verification.read_text()
assert sha(ROOT/'dev/cppgm++')==data['binaries'][1]['sha256']
count=sum(len(row[phase]['observations'])+len(row[phase]['warmups']) for report in (data,noise) for row in report['workloads'].values() for phase in ('compiler','runtime') if phase in row)
assert count==392
base='8af3c149454e4e43e441206e6978f4d1300e079b'
plan=(ROOT/'pa14/plan.md').read_text()
assert all(marker+': `'+base+'`.' in plan for marker in ['Stage base commit','Last reviewed commit'])
result=dict(harness_sha256=sha(__file__),stage_base=base,last_reviewed=base,status='virtual-group-complete; full-stage architecture open',
 implementation_commits=['8d596c53','9ec76f55'],accepted_binary=data['binaries'][1],
 observations=count,total_observations=14112+count,stage_tests=314,prior_tests=1621,through_tests=1935,
 parity_inputs=349,native_programs=35,virtual_api_runs=12,inherited_failure_api_runs=18,
 checks=checks[:4],verification=file(verification),
 evidence=[file(ROOT/'student.tests/pa14'/name) for name in ['virtual-demand-performance.json','virtual-demand-noise.json','virtual-demand-validation.json','virtual-demand-abi-validation.json','virtual-demand-proofs.json','virtual-demand-layout.json']],
 documentation=[file(ROOT/'pa14/plan.md'),file(ROOT/'pa14/performance.md')],
 boundary='Default occurrences, destructor exception truth and shared constructor/destructor action state need a separate insertion/completeness/cycle audit across type-only queries and execution. Completed virtual/ABI dependencies do not supply those keys.',
 storage='63 groups of archived identical LowIR were verified before deduplication; all paths/hashes preserved, shared files read-only, 5157573103 logical bytes recovered.')
(ROOT/'student.tests/pa14/virtual-demand-handoff.json').write_text(json.dumps(result,indent=2)+'\n')
print('Virtual behavior group complete: 314/1621/1935 tests, 14504 observations; full-stage architecture remains open')
