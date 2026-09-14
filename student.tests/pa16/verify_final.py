#!/usr/bin/env python3
"""Read-only verification of the independent PA16 final audit; optional --clean."""
from pathlib import Path
import hashlib, json, re, subprocess, sys
ROOT=Path(__file__).resolve().parents[2]
def run(*args):return subprocess.check_output(args,cwd=ROOT,text=True).strip()
def sha(path):
 h=hashlib.sha256()
 with Path(path).open('rb') as f:
  for chunk in iter(lambda:f.read(1024*1024),b''):h.update(chunk)
 return h.hexdigest()
def file_tree(root,files):
 h=hashlib.sha256();files=sorted(files)
 for f in files:h.update(str(f).encode()+b'\0'+bytes.fromhex(sha(Path(root)/f)))
 return [len(files),h.hexdigest()]
def directory_tree(path):
 p=Path(path);return file_tree(p,[f.relative_to(p) for f in p.rglob('*') if f.is_file()])
def tracked(paths):return run('git','ls-files',*paths).splitlines()
def validate_performance(path,expected_binary=None):
 p=json.loads(Path(path).read_text());assert p['finished_utc']
 assert p['flags']==['--emit-lowir','-O0']
 if expected_binary:assert p['binaries'][1]['sha256']==expected_binary
 for b in p['binaries']:assert sha(b['path'])==b['sha256']
 count=0
 for name,w in p['workloads'].items():
  assert hashlib.sha256(w['source'].encode()).hexdigest()==w['source_sha256'],name
  for source in w.get('input_paths',[]):assert sha(source)==w['source_sha256']
  for phase in ['compiler','runtime']:
   if phase not in w:continue
   x=w[phase];rows=x['observations'];count+=len(rows)+len(x['warmups'])
   assert all(r['checked_exit']==0 and r['wall_s']>0 and r['rss_kib']>=0 for r in rows+x['warmups'])
   if w['comparison']=='exact':
    assert [r['binary'] for r in x['warmups']]==[0,1]
    assert [r['binary'] for r in rows]==[0]*4+[0,1,1,0]*4
    assert len(x['paired_b_over_a'])==4
    aa=[r['wall_s'] for r in rows[:4]]
    assert x['aa_range_s']==[min(aa),max(aa)]
    for k,pair in zip(range(4,20,4),x['paired_b_over_a']):
     a=sum(r['wall_s'] for r in rows[k:k+4] if r['binary']==0)
     b=sum(r['wall_s'] for r in rows[k:k+4] if r['binary']==1)
     assert abs(pair-b/a)<1e-12
   else:
    assert w['comparison']=='entry-rejected' and w['entry_behavior']['compile_exit']!=0
    assert [r['binary'] for r in rows]==[1]*6
  if w['comparison']=='exact':
   assert w['outputs'][0]['sha256']==w['outputs'][1]['sha256']
   if 'runtime' in w:assert w['outputs'][0]['native']['sha256']==w['outputs'][1]['native']['sha256']
  for out in w['outputs']:
   assert sha(out['path'])==out['sha256']
   if 'native' in out:
    n=out['native'];assert n['checked_exit']==0 and sha(n['path'])==n['sha256']
    assert n['code_and_alignment_bytes']+n['global_data_bytes']==n['executable_payload_bytes']
 return count
def main():
 p=json.loads((ROOT/'student.tests/pa16/final-checkpoint.json').read_text())
 impl=p['implementation_paths'];protected=p['protected_paths']
 assert not run('git','diff','--name-only',p['code_tip'],'--',*impl)
 assert file_tree(ROOT,tracked(impl))==p['implementation_tree']
 assert sha(ROOT/'dev/cppgm++')==p['compiler_sha256']
 assert not run('git','diff','--name-only',p['entry_commit'],'--',*protected)
 assert tracked(protected)==run('git','ls-tree','-r','--name-only',p['entry_commit'],'--',*protected).splitlines()
 assert file_tree(ROOT,tracked(protected))==p['protected_tree']
 assert file_tree(ROOT,p['historical_files'])==p['historical_tree']
 assert not run('git','diff','--name-only',p['entry_commit'],'--',*p['historical_files'])
 assert run('git','rev-list','--reverse',p['stage_base']+'..'+p['code_tip']).splitlines()==p['reviewed_stage_commits']
 for path in ['pa16/plan.md','pa16/audit.md']:
  content=(ROOT/path).read_text()
  assert re.search(r'Stage base commit: `([^`]+)`',content)[1]==p['stage_base']
  assert re.search(r'Last reviewed commit: `([^`]+)`',content)[1]==p['code_tip']
 for path,digest in p['final_artifacts'].items():assert sha(ROOT/path)==digest,path
 for path,digest in p['retained_trial_records'].items():assert sha(path)==digest,path
 validation=p['validation'];assert sha(validation['record'])==validation['sha256']
 v=json.loads(Path(validation['record']).read_text());assert v['compiler_sha256']==p['compiler_sha256']
 assert directory_tree(validation['directory'])==validation['tree']
 for row in v['checks']+v['controls']:
  assert row['exit_code']==0 and sha(row['log'])==row['sha256'],row['name']
 for check in p['required_results']:
  row=next(r for r in v['checks'] if r['name']==check['name'])
  assert row['command']==check['command'] and check['success'] in Path(row['log']).read_text()
 through=Path(next(r['log'] for r in v['checks'] if r['name']=='throughTests')).read_text()
 assert len(re.findall(r'^===== pa\d+ =====$',through,re.M))==16
 native=rejected=0
 for suite in p['personal_suites']:
  log=Path(next(r['log'] for r in v['controls'] if r['name']==suite['name'])).read_text()
  if 'result' in suite:
   d=json.loads(Path(suite['result']).read_text());rows=d if isinstance(d,list) else d['rows']
   assert all(r.get('passed',r.get('native_exit')==0) for r in rows)
   got=sum(r.get('native_exit')==0 for r in rows)
   assert got==suite['native'] and len(rows)-got==suite['rejection']
  else:assert suite['summary'] in log
  native+=suite['native'];rejected+=suite['rejection']
 assert [native,rejected]==p['personal_counts']==[289,93]
 revisions=json.loads((ROOT/'student.tests/pa16/initialization-reference-revisions.json').read_text())
 result=json.loads((ROOT/'student.tests/pa16/result-reference-revision.json').read_text())
 revisions.append(dict(path=result['path'],before_sha256=result['old_sha256'],after_sha256=result['new_sha256']))
 courses=[f'pa{n}/tests' for n in range(1,17)]
 assert sorted(r['path'] for r in revisions)==run('git','diff','--name-only',p['stage_base'],'--',*courses).splitlines()
 assert len(revisions)==25
 for r in revisions:
  assert sha(ROOT/r['path'])==r['after_sha256']
  before=subprocess.check_output(['git','show',p['stage_base']+':'+r['path']],cwd=ROOT)
  assert hashlib.sha256(before).hexdigest()==r['before_sha256']
 total=0
 for campaign in p['campaigns']:
  count=validate_performance(ROOT/campaign['path'],p['compiler_sha256'] if campaign['final_binary'] else None)
  assert count==campaign['observations'];total+=count
  d=json.loads((ROOT/campaign['path']).read_text())
  assert d['harness_sha256']==sha(ROOT/campaign['producer'])
  assert d['shared_harness_sha256']==sha(ROOT/'student.tests/pa10/benchmark.py')
  if campaign['final_binary']:assert d['source_commit']==p['code_tip']
 assert total==p['performance_observations']==1560
 if '--clean' in sys.argv:assert not run('git','status','--short')
 print('PA16 final audit verified: 2266/2266 course, 16 stages; 289 native/93 rejection controls; 1560 frozen observations; 25 inherited oracle proofs and all historical evidence preserved.')
if __name__=='__main__':main()
