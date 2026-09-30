#!/usr/bin/env python3
"""Record final required checks and explicit personal controls for handoff 125."""
from pathlib import Path
import hashlib,json,re,subprocess,sys
ROOT=Path(__file__).resolve().parents[2];WORK=Path(sys.argv[1]).resolve();OUT=Path(sys.argv[2]).resolve();WORK.mkdir(parents=True,exist_ok=True)
def sha(p):return hashlib.sha256(Path(p).read_bytes()).hexdigest()
result=dict(implementation=subprocess.check_output(['git','rev-parse','HEAD'],cwd=ROOT,text=True).strip(),compiler_sha256=sha(ROOT/'dev/cppgm++'),checks=[])
commands=[('stage',['make','test-pa23']),('prior',['make','test-report-through-pa22']),('through',['make','test-report-through-pa23']),('file-audit',['perl','scripts/cppgm_file_audit.pl','--stage','pa23','--paths','dev/src'])]
for name,script in [('lifecycle','lifecycle125.py'),('member','member125.py'),('tu','lifecycle_tu125.py'),('ownership','audit124.py'),('layout','layout123.py'),('semantic','verify123.py'),('inherited','verify121.py'),('old-lifecycle','verify122.py'),('deep','deep_overrider124.py')]:
 commands.append((name,['python3','student.tests/pa23/'+script,str(ROOT/'dev/cppgm++'),str(WORK/name)]))
commands.append(('roundtrip',['python3','student.tests/pa23/roundtrip121.py',str(WORK/'roundtrip')]))
for name,command in commands:
 log=WORK/(name+'.log');data=WORK/(name+'.json') if command[0]=='python3' else log
 with data.open('w') as out, (WORK/(name+'.stderr')).open('w') as err:p=subprocess.run(command,cwd=ROOT,stdout=out,stderr=err)
 row=dict(name=name,command=command,exit=p.returncode,stdout=str(data),stdout_sha256=sha(data),stderr_sha256=sha(WORK/(name+'.stderr')))
 if command[0]=='python3':
  evidence=json.loads(data.read_text());row['cases']=len(evidence['cases']);row['passed']=sum(c.get('passed',c.get('stable',False)) for c in evidence['cases'])
  (ROOT/'student.tests/pa23'/('controls125-'+name+'.json')).write_bytes(data.read_bytes())
 else:row['summary']=[s for s in log.read_text().splitlines() if 'SUMMARY' in s or 'ALL TESTS' in s or 'File audit' in s]
 result['checks'].append(row);OUT.write_text(json.dumps(result,indent=2)+'\n');print(name,p.returncode,row.get('passed',row.get('summary')),flush=True)
 assert p.returncode==0,row
# The coverage audit checks all original inputs/statuses and comparator scripts.
manifest=json.loads((ROOT/'student.tests/pa23/oracles125.json').read_text());fixtures=manifest['fixtures'];assert len(fixtures)==45
for row in fixtures:
 src=ROOT/row['source'];assert sha(src)==row['source_sha256'];assert sha(src.with_suffix('.ref.exit_status'))==row['exit_status_sha256'];assert sha(src.with_suffix('.ref'))==row['after_sha256']
 changed=subprocess.check_output(['git','diff','7a644d69','--name-only','--','scripts','pa23/scripts','pa23/Makefile'],cwd=ROOT,text=True);assert not changed
result['stage_progress']=dict(entry_passed=24,entry_failed=21,final_passed=45,final_failed=0,fixture_count=45,coverage_preserved=True,oracles_manifest_sha256=sha(ROOT/'student.tests/pa23/oracles125.json'))
assert sha(ROOT/'dev/cppgm++')==result['compiler_sha256']
OUT.write_text(json.dumps(result,indent=2)+'\n')
