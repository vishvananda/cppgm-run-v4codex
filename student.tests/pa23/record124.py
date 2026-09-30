#!/usr/bin/env python3
"""Seal the checkpoint audit against immutable fixtures and frozen binaries."""
from pathlib import Path
import hashlib,json,re,subprocess,sys
ROOT=Path(__file__).resolve().parents[2];ART=Path(sys.argv[1]).resolve()
BASE='f33dd0775073bf5db6665fb2f4f504783159df76';ENTRY='2bba72734caa8bdf772418ad6e96775e76ed984d'
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def git(*args):return subprocess.check_output(['git',*args],cwd=ROOT,text=True).strip()
def read(name):return json.loads((ROOT/'student.tests/pa23'/name).read_text())
def failures(text):return sorted(set(re.findall(r'(pa23/tests/[^ :]+\.t): ERROR:',text)))
old=failures((ART/'stage-entry.log').read_text());new=failures((ART/'stage-final.log').read_text())
assert len(old)==21 and old==new
for name,marker in [('stage','24 / 45 TESTS PASSED'),('prior','ALL TESTS PASSED SUCCESSFULLY! (3811 / 3811)'),('through','3835 / 3856 TESTS PASSED'),('audit','File audit passed for pa23 with 3 warning(s).')]:
 assert marker in (ART/(name+'-final.log')).read_text(),name
assert not git('diff','--name-only',ENTRY,'--',':(glob)pa*/tests/**',':(glob)pa*/scripts/**','scripts','reference-binaries')
correction='pa23/tests/general/100-diamond-virtual-destructor-slot-merge.ref'
assert git('diff','--name-only',BASE,'--',':(glob)pa*/tests/**',':(glob)pa*/scripts/**','scripts','reference-binaries')==correction
oldref=git('show',BASE+':'+correction);newref=(ROOT/correction).read_text().strip()
assert oldref.replace('global @__rtti_class_D [binding=weak, object=_ZTI1D] = {\n  ptr addr @__external_rtti_vtable____vmi_class_type_info + 16\n  ptr addr @__typeinfo_name__class_D\n  i32 0','global @__rtti_class_D [binding=weak, object=_ZTI1D] = {\n  ptr addr @__external_rtti_vtable____vmi_class_type_info + 16\n  ptr addr @__typeinfo_name__class_D\n  i32 1')==newref
fixtures=sorted((ROOT/'pa23/tests/general').glob('*.t'));assert len(fixtures)==45
for p in fixtures:assert subprocess.check_output(['git','show',BASE+':'+str(p.relative_to(ROOT))],cwd=ROOT)==p.read_bytes()
compiler=sha(ROOT/'dev/cppgm++');assert compiler==sha(ART/'final-v2')
counts={}
for file,count,failed in [('audit124-final.json',21,[]),('controls124-semantic.json',26,[]),('controls124-layout.json',19,['construct-shared-once','construct-hidden-vptr-target']),('controls124-lifecycle.json',20,[]),('controls124-inherited.json',33,['virtual-member-pointer'])]:
 d=read(file);assert d['compiler_sha256']==compiler and len(d['cases'])==count
 assert [c['name'] for c in d['cases'] if not c['passed']]==failed
 counts[file]=dict(passed=count-len(failed),total=count,failures=failed)
for file in ('deep124-entry.json','deep124-final.json'):
 d=read(file);assert len(d['cases'])==2 and d['cases'][0]['passed']
 assert d['cases'][1]['compile_exit']==0 and d['cases'][1]['runtime_exit']==-11
assert read('deep124-final.json')['compiler_sha256']==compiler
native=read('native124.json');assert len(native['cases'])==19
assert [c['name'] for c in native['cases'] if not c['passed'] and not c['unfinished']]==['shared-rtti']
limit=read('backend-limit124.json');assert all(c['hosted_run']['exit']==0 for c in limit['cases'])
assert limit['cases'][0]['private_run']['exit']==1 and limit['cases'][1]['standalone_run']['exit']==1
roundtrips=read('roundtrips124.json');assert len(roundtrips['cases'])==44 and all(c['stable'] for c in roundtrips['cases'])
for file,workloads in [('performance124.json',18),('performance124-forest.json',1)]:
 d=read(file);assert len(d['workloads'])==workloads and d['binaries'][1]['sha256']==compiler
 for b in d['binaries']:assert sha(Path(b['path']))==b['sha256']
 for w in d['workloads'].values():
  assert w['identical_text'] and all(o['checked_exit']==0 for o in w['outputs'])
  for phase in ('compiler','runtime'):
   assert len(w[phase]['observations'])==20 and len(w[phase]['paired_b_over_a'])==4
historical=[]
for file in ('performance121-common.json','performance121-views.json','performance122.json','performance122-repeat.json','performance123.json','performance123-repeat.json'):
 d=read(file)
 for b in d['binaries']:assert sha(Path(b['path']))==b['sha256']
 historical.append(dict(path=file,sha256=sha(ROOT/'student.tests/pa23'/file),frozen_binaries_verified=True))
reviewed=git('rev-parse','HEAD')
assert not git('diff','HEAD','--','dev'), 'commit implementation before sealing review marker'
result=dict(stage_base=BASE,review_entry=ENTRY,last_reviewed_commit=reviewed,previous_goal_turn='progress: committed handoff 123 improved 20/45 to 24/45; no live interrupted process at audit entry',compiler_sha256=compiler,entry_compiler_sha256=sha(ART/'entry'),stage_passed=24,stage_total=45,entry_failures=old,final_failures=new,new_failures=[],prior_passed=3811,prior_total=3811,prior_stages=22,through_passed=3835,through_total=3856,file_audit='pass; three inherited header organization warnings',controls=counts,deep_overrider=dict(semantic_checks_passed=2,total=2,entry_runtime_exit=-11,final_runtime_exit=-11,remaining_owner='virtual-base lifecycle: retained nested variant of the incomplete C2 construction path'),roundtrips=44,historical_performance=historical,reference_changes_this_audit=[],accumulated_reference_correction=correction,reference_proof='pa23/reference-correction122.md',reference_bundle_revision='c2f713cd70d06170632bfde3e75dd6fe1aa44d98',compiler_build_config=(ROOT/'obj/dev/.compile_config').read_text(),implementation_tree=git('rev-parse','HEAD:dev'),reviewed_commits=git('rev-list','--reverse',BASE+'..'+reviewed).splitlines(),checks=[])
for label,command,code in [('stage','make test-pa23',2),('prior','n=23; if [ "$n" -le 1 ]; then echo \'===== ALL TESTS PASSED SUCCESSFULLY! (0/0) =====\'; else make test-report-through-pa$((n - 1)); fi',0),('through','make test-report-through-pa23',2),('audit','perl scripts/cppgm_file_audit.pl --stage pa23 --paths dev/src',0)]:
 log=ART/(label+'-final.log');result['checks'].append(dict(command=command,observed_exit=code,log=str(log),log_sha256=sha(log)))
result['fixture_manifest']=[dict(path=str(p.relative_to(ROOT)),source_sha256=sha(p),reference_sha256=sha(p.with_suffix('.ref')),expected_status=p.with_suffix('.ref.exit_status').read_text().strip()) for p in fixtures]
result['evidence']={str(p.relative_to(ROOT)):sha(p) for p in sorted((ROOT/'student.tests/pa23').glob('*124*.json')) if p.name!='validation124.json'}
result['abi_reference']=dict(url='https://itanium-cxx-abi.github.io/cxx-abi/abi.html#rtti-layout',snapshot=str(ART/'itanium-abi.html'),sha256=sha(ART/'itanium-abi.html'))
(ROOT/'student.tests/pa23/validation124.json').write_text(json.dumps(result,indent=2)+'\n')
print('Audit validated:',reviewed,'; prior 3811/3811; PA23 unchanged 24/45; file audit pass; 21 audit controls; all fixture hashes retained')
