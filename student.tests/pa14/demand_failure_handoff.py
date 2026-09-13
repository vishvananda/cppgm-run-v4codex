#!/usr/bin/env python3
"""Record accepted binaries, preserved trials and terminal required checks."""
from pathlib import Path
import hashlib,json,subprocess,sys
ROOT=Path(__file__).resolve().parents[2];WORK=Path(sys.argv[1]).resolve()
def sha(path):return hashlib.sha256(Path(path).read_bytes()).hexdigest()
def file(path):return dict(path=str(path),sha256=sha(path))
accepted=json.loads((ROOT/'student.tests/pa14/demand-failure-performance.json').read_text())['binaries'][1]
assert sha(ROOT/'dev/cppgm++')==accepted['sha256']
result=dict(implementation_commit='b49acc80',entry_commit='1978d615',accepted_binary=accepted,
 harness_sha256=sha(__file__),checks=[],evidence=[],final_controls=[],trial=dict(status='rejected',reason='No repeatable compiler latency benefit; wide case +0.61%, mixed other compiler pairs, no text or generated-code reduction.',patch=file(WORK/'complete-fastpath.patch')))
for name,command in [('stage','make test-pa14'),('prior','make test-report-through-pa13'),('through','make test-report-through-pa14'),('file-audit','perl scripts/cppgm_file_audit.pl --stage pa14 --paths dev/src')]:
 log=WORK/(name+'-restored.log');assert ('file audit passed' if name=='file-audit' else 'all tests passed') in log.read_text().lower()
 result['checks'].append(dict(name=name,command=command,exit_code=0,log=str(log),log_sha256=sha(log)))
for name in ['proofs','layout','validation','performance','fastpath-validation','fastpath-performance']:
 result['evidence'].append(file(ROOT/f'student.tests/pa14/demand-failure-{name}.json'))
for mode in ['release','sanitized']:
 folder=WORK/'validation-final'/('demands-'+mode)
 result['final_controls'].append(dict(mode=mode,source=file(ROOT/'student.tests/pa14/demand-failures.cc'),binary=file(folder/'probe'),manifest=file(folder/'checks.json'),logs=[file(row['log']) for row in json.loads((folder/'checks.json').read_text())]))
result['verifications']=[dict(observations=count,**file(WORK/name)) for name,count in [('verification-history.log',13776),('verification-accepted.log',336)]]
result['total_observations']=14112
assert '13776 total frozen performance observations verified' in (WORK/'verification-history.log').read_text()
assert '336 demand-state observations' in (WORK/'verification-accepted.log').read_text()
(ROOT/'student.tests/pa14/demand-failure-handoff.json').write_text(json.dumps(result,indent=2)+'\n')
print('Accepted terminal-state compiler restored exactly; 14,112 observations and required reports verified')
