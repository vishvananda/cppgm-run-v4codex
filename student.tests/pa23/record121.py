#!/usr/bin/env python3
"""Seal handoff evidence without changing required fixtures or Ralph state."""
from pathlib import Path
import hashlib,json,os,re,subprocess
ROOT=Path(__file__).resolve().parents[2];ART=Path(os.environ['RALPH_ARTIFACT_DIR'])/'pa23-121'
def sha(path):return hashlib.sha256(Path(path).read_bytes()).hexdigest()
def failures(text):return sorted(set(re.findall(r'(pa23/tests/[^ :]+\.t): ERROR:',text)))
base_log=Path('/home/vishvananda/work/.ralph/v4codex-gpt-6-astra-xhigh/last-test.log')
before=base_log.read_text();after=(ART/'stage-delivery.log').read_text();prior=(ART/'prior-delivery.log').read_text();audit=(ART/'audit-delivery.log').read_text()
old,new=failures(before),failures(after)
assert len(old)==42 and len(new)==28 and not set(new)-set(old)
assert '17 / 45 TESTS PASSED' in after and '3811 / 3811' in prior and 'ALL TESTS PASSED SUCCESSFULLY' in prior
assert 'File audit passed for pa23' in audit
fixtures=sorted((ROOT/'pa23/tests/general').glob('*.t'));assert len(fixtures)==45
changed=subprocess.check_output(['git','diff','--name-only','f33dd0775073bf5db6665fb2f4f504783159df76','--','pa*/tests','pa*/scripts','scripts','reference-binaries'],cwd=ROOT,text=True).splitlines();assert not changed,changed
controls=json.loads((ROOT/'student.tests/pa23/controls121-native.json').read_text())
failed=[r['name'] for r in controls['cases'] if not r['passed']];assert failed==['virtual-member-pointer'];assert len(controls['cases'])==33
roundtrips=json.loads((ROOT/'student.tests/pa23/roundtrips121-delivery.json').read_text());assert len(roundtrips['cases'])==41 and all(r['stable'] for r in roundtrips['cases'])
result=dict(stage_base='f33dd0775073bf5db6665fb2f4f504783159df76',last_reviewed_commit='f33dd0775073bf5db6665fb2f4f504783159df76',implementation_commit='6ae06461',compiler_sha256=sha(ROOT/'dev/cppgm++'),baseline_log_sha256=sha(base_log),baseline_passed=3,baseline_failures=old,stage_passed=17,stage_total=45,stage_failures=new,resolved_failures=sorted(set(old)-set(new)),new_failures=[],contract_changes=changed,prior_passed=3811,prior_total=3811,prior_stages=22,file_audit='pass; three inherited header-organization warnings',personal_passed=32,personal_total=33,unfinished_personal=failed,stable_roundtrips=41,independent_review='pending; implementation handoff does not certify the whole stage',checks=[])
assert result['compiler_sha256']==sha(ART/'delivery-cppgm++')
for name,cmd,status in [('stage','make test-pa23',2),('prior','make test-report-through-pa22',0),('audit','perl scripts/cppgm_file_audit.pl --stage pa23 --paths dev/src',0)]:
 log=ART/(name+'-delivery.log');result['checks'].append(dict(command=cmd,observed_exit=status,log=str(log),log_sha256=sha(log)))
result['fixture_manifest']=[dict(path=str(p.relative_to(ROOT)),source_sha256=sha(p),reference_sha256=sha(p.with_suffix('.ref')),expected_status=p.with_suffix('.ref.exit_status').read_text().strip(),observed_status=p.with_suffix('.my.exit_status').read_text().strip()) for p in fixtures]
standalone=json.loads((ROOT/'student.tests/pa23/native121.json').read_text())['cases']
result['standalone_passed']=sum(r['passed'] for r in standalone);result['standalone_total']=len(standalone)
result['standalone_failures']=[r['name'] for r in standalone if not r['passed']]
assert result['standalone_failures']==['crosscast-null-and-miss','crosscast-private-base','virtual-member-pointer','exception-rtti']
result['reference_backend_limits']='student.tests/pa23/backend-limits121.json'
result['native_text']={}
result['view_comparisons']=[]
for mode in ('common','views'):
 perf=json.loads((ROOT/('student.tests/pa23/performance121-'+mode+'.json')).read_text());assert perf['binaries'][1]['sha256']==result['compiler_sha256']
 for name,w in perf['workloads'].items():
  if mode=='views' and not w['identical_lowir']:
   work=ART/'performance-views';src=work/(name+'.t');src.write_text(w['source'])
   for lane,suffix in [(0,'ref'),(1,'my')]:
    src.with_suffix('.'+suffix).write_bytes((work/(name+str(lane)+'.lowir')).read_bytes())
    src.with_suffix('.'+suffix+'.exit_status').write_text('EXIT_SUCCESS\n')
   compared=subprocess.run([str(ROOT/'pa23/scripts/compare_results.pl'),'ref','my',str(src)],cwd=ROOT/'pa23',capture_output=True,text=True)
   assert compared.returncode==0,(name,compared.stdout,compared.stderr)
   result['view_comparisons'].append(dict(name=name,relaxed_equivalent=True))
  row=[]
  for i in (0,1):
   exe=ART/('performance-'+mode)/(name+str(i));target=ART/'inspection.text'
   subprocess.run(['objcopy','-O','binary','--only-section=.text',str(exe),str(target)],check=True)
   row.append(dict(bytes=target.stat().st_size,sha256=sha(target)))
  result['native_text'][mode+'/'+name]=dict(lanes=row,identical=row[0]==row[1])
(ROOT/'student.tests/pa23/validation121.json').write_text(json.dumps(result,indent=2)+'\n')
print('42 -> 28 existing failures; prior 3811/3811; audit pass; 32/33 controls, one recorded unfinished behavior; 41 stable roundtrips')
