#!/usr/bin/env python3
"""Verify the implementation handoff's source, fixture, failure and measurement evidence."""
from pathlib import Path
import hashlib,json,re,subprocess
ROOT=Path(__file__).resolve().parents[2]
def git(*args):return subprocess.check_output(['git',*args],cwd=ROOT,text=True).strip()
def sha(p):return hashlib.sha256(Path(p).read_bytes()).hexdigest()
def source_digest():
 h=hashlib.sha256()
 for name in sorted(git('ls-files','dev').splitlines()):
  h.update(name.encode()+b'\0');h.update((ROOT/name).read_bytes());h.update(b'\0')
 return h.hexdigest()
def verify():
 evidence=json.loads((ROOT/'student.tests/pa17/handoff.json').read_text())
 assert source_digest()==evidence['source_digest']
 subprocess.check_call(['git','merge-base','--is-ancestor',evidence['source_commit'],'HEAD'],cwd=ROOT)
 assert not git('diff',evidence['stage_base'],'--',*evidence['protected_paths'])
 plan=(ROOT/'pa17/plan.md').read_text()
 for field in ['Stage base commit','Last reviewed commit']:assert f'{field}: `{evidence["stage_base"]}`' in plan
 for entry in evidence['logs'].values():assert sha(entry['path'])==entry['sha256']
 prior=Path(evidence['logs']['prior']['path']).read_text();stage=Path(evidence['logs']['stage']['path']).read_text()
 assert 'ALL TESTS PASSED SUCCESSFULLY! (2266 / 2266)' in prior
 assert 'TEST SUMMARY: 207 / 343 TESTS PASSED' in stage
 through=Path(evidence['logs']['through']['path']).read_text()
 assert 'TEST SUMMARY: 2473 / 2609 TESTS PASSED' in through
 assert all(line.startswith('pa17/') for line in through.splitlines() if ': ERROR:' in line)
 assert len(re.findall(r'^pa17/[^:]+: ERROR:',stage,re.M))==136
 delta=evidence['failure_delta'];assert len(delta['entry_failures'])==211 and len(delta['final_failures'])==136
 assert not delta['new_failures'] and set(delta['final_failures'])<set(delta['entry_failures'])
 assert len(delta['fixed'])==75
 assert len(list((ROOT/'pa17/tests').glob('*/*.t')))==343
 assert 'File audit passed for pa17' in Path(evidence['logs']['file_audit']['path']).read_text()
 controls=json.loads((ROOT/'student.tests/pa17/entity-controls-results.json').read_text())
 assert len(controls)==34 and all(r['passed'] for r in controls)
 assert sum(r['expected']=='native' and r['native_exit']==0 and r['backend_exit']==0 for r in controls)==24
 assert sum(r['expected']=='reject' and r['compiler_exit']!=0 for r in controls)==10
 perf=json.loads((ROOT/'student.tests/pa17/entity-performance.json').read_text())
 assert perf['source_commit']==evidence['source_commit'] and not perf['source_diff']
 assert perf['finished_utc'] and perf['binaries'][1]['sha256']==sha(ROOT/'dev/cppgm++')
 for name,w in perf['workloads'].items():
  assert 'compiler' in w and all(r['checked_exit']==0 for r in w['compiler']['observations'])
  if w['comparison']=='exact':assert len({r['sha256'] for r in w['outputs']})==1
  else:assert w['entry_behavior']['compile_exit']!=0
  if name.startswith('runtime-'):
   assert 'runtime' in w and all(r['checked_exit']==0 for r in w['runtime']['observations'])
   assert len({r['native']['sha256'] for r in w['outputs']})==1
 print('PA17 implementation handoff verified: 75 fixed, no regressions; 136 failures remain for implementation.')
if __name__=='__main__':verify()
