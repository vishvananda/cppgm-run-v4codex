#!/usr/bin/env python3
"""Verify the accumulated audit's source, measurement, coverage and code boundary."""
import hashlib,json,pathlib,re,subprocess,sys
from audit214_history import measurement
ROOT=pathlib.Path(__file__).resolve().parents[2]
E=ROOT/'student.tests/pa32/evidence214'
def sha(p):return hashlib.sha256(pathlib.Path(p).read_bytes()).hexdigest()
def git(*args):return subprocess.check_output(['git',*args],cwd=ROOT,text=True).strip()
def read(n):return json.loads((E/n).read_text())
def main():
 b=read('binding.json');art=pathlib.Path(b['artifact_root'])
 for name,digest in b['sources'].items():assert sha(ROOT/name)==digest,name
 for name,digest in b['evidence'].items():assert sha(E/name)==digest,name
 for name,digest in b['artifacts'].items():assert sha(name)==digest,name
 for name,digest in b['binaries'].items():assert sha(name)==digest,name
 assert sha(ROOT/'dev/cppgm++')==sha(art/'final-cppgm++')
 assert sha(ROOT/'dev/lowiropt')==sha(art/'final-lowiropt')
 # No harness, oracle, contract, or course-coverage edits across the range.
 for name in git('diff','--name-only',b['last_reviewed']).splitlines():
  assert name.startswith(('dev/','student.tests/pa32/')) or name in ['pa32/plan.md','pa32/audit.md'],name
 assert git('rev-parse','HEAD:pa32/tests')==b['contract_tree']
 assert not git('diff',b['last_reviewed'],'--','spec.md','pa8/lowir.md','pa32/README.md','scripts','reference-binaries')
 c={r['name']:r for r in read('checks.json')}
 for row in c.values():assert sha(row['log'])==row['sha256']
 failed={'stage','through','debug','debug-driver-o1','debug-driver-o2','debug-driver-o3'}
 for name,row in c.items():assert row['exit_code']==(2 if name in failed else 0),(name,row['exit_code'])
 def log(n):return pathlib.Path(c[n]['log']).read_text()
 assert 'ALL TESTS PASSED SUCCESSFULLY! (5178 / 5178)' in log('prior')
 assert 'File audit passed for pa32 with 4 warning(s)' in log('file')
 assert '5380 / 5397 TESTS PASSED' in log('through')
 old=(art/'entry-stage.log').read_text();new=log('stage')
 def failures(s):return set(re.findall(r'^(pa32/[^:]+): ERROR:',s,re.M))
 assert failures(old)==failures(new)==set(b['remaining_failures']) and len(failures(new))==17
 assert all('202 / 219 TESTS PASSED' in s for s in [old,new])
 for name in ['normal-objects','debug-objects']:assert 'PASS (25/25)' in log(name)
 for level,n in [(1,3),(2,1),(3,1)]:assert f'pa32 tests/debuginfo/o{level}: PASS ({n}/{n})' in log('debug')
 for name in ['reducers','entry-reducers']:
  rows=json.loads((art/name/'runs.json').read_text())
  failed_rows=[r for r in rows if r['exit_code']]
  assert bool(failed_rows)==(name=='entry-reducers')
  if name=='entry-reducers':
   assert any('undefined' in r['stderr'] for r in failed_rows)
   assert len(failed_rows)==4
 # The three source/template traces include independent checked executions,
 # full in-memory validation, consumed debug MIR, and direct/replayed ELF.
 for trace in ['trace','loop-trace','memory-trace']:
  records=json.loads((art/'checks'/trace/'trace.json').read_text())
  assert all(r['exit_code']==0 for r in records)
  assert sum('--validate-lowir' in r['command'] for r in records)==4
 count=0
 for lane in b['performance_lanes']:
  d=read(lane+'.json');count+=measurement(d)
  for binary in d['binaries'].values():assert sha(binary['path'])==binary['sha256']
  for workload,s in d['summary'].items() if lane!='selfhost' else []:
   images=d['images'][workload]
   for label in 'AB':
    assert sha(art/lane/(workload+label+'.o'))==images[label]['object_sha256']
    assert sha(art/lane/(workload+label))==images[label]['executable_sha256']
   if lane in ['objects','loops','memory']:
    for ext,key in [('.lowir','lowir_sha256'),('.cpp','main_sha256')]:assert sha(art/lane/(workload+ext))==d['inputs'][workload][key]
   else:
    assert images['A']['object_sha256']==images['B']['object_sha256']
    assert sha(art/lane/(workload+'.cpp'))==d['inputs'][workload]
  if lane=='selfhost':
   assert d['images']['A']==d['images']['B']
   for path,digest in d['inputs'].items():assert sha(path)==digest
 assert count==b['sample_count']
 bounds=read('bounds.json')
 assert bounds[0]['stats']['memory_work']*10==bounds[2]['stats']['memory_work']
 assert bounds[3]['stats']['memory_declined']==1
 assert any(v.get('cases')==2136 for v in bounds)
 loop=read('loop-bounds.json')
 for row in loop:
  if row.get('case')=='one-function-budget':assert row['telemetry']['loop_clone_reserved']<=256
  if row.get('case')=='body-budget':assert row['telemetry']['loop_clone_reserved']==0
 if '--records' in sys.argv:
  tip=git('rev-parse','HEAD^')
  for file in ['pa32/plan.md','pa32/audit.md']:
   assert 'Last reviewed commit: '+tip in (ROOT/file).read_text()
   assert 'Stage base commit: '+b['stage_base'] in (ROOT/file).read_text()
  assert set(git('diff','--name-only','HEAD^','HEAD').splitlines())=={'pa32/plan.md','pa32/audit.md'}
  assert not git('status','--porcelain')
 print(f'PA32 audit 214 PASS: 17/219 unchanged failures; prior/file gates; {count} current and 4536 historical samples;'+(' committed clean boundary' if '--records' in sys.argv else ' current evidence'))
if __name__=='__main__':main()
