#!/usr/bin/env python3
"""Seal audited PA22 sources, executed gate logs, controls and frozen evidence."""
from pathlib import Path
import hashlib,json,re,subprocess
ROOT=Path(__file__).resolve().parents[2]; WORK=Path('/tmp/pa22-120')
BASE='a8482d768bd2dcede42ea63ef39e39cf3245c380'; ENTRY='17bc7a06'; CODE='2b1a02c9'
def sha(path):return hashlib.sha256(Path(path).read_bytes()).hexdigest()
def git(*args):return subprocess.check_output(['git',*args],cwd=ROOT,text=True)
def read(path):return json.loads(Path(path).read_text())
assert not git('diff',CODE,'--','dev').strip(), 'implementation changed after freezing'
final=sha(ROOT/'dev/cppgm++');assert final==sha(WORK/'final/cppgm++')
result=dict(stage='pa22',phase='final independent audit',spec_alignment='aligned',base=BASE,entry=ENTRY,implementation=CODE,
 compiler_sha256=final,spec_sha256=sha(ROOT/'spec.md'),readme_sha256=sha(ROOT/'pa22/README.md'),
 commits=git('log','--format=%H %s',BASE+'..'+CODE).splitlines(),commands=[],evidence={},sources={},fixtures=[])
for command,name,code,needle in [
 ('make test-report-through-pa22','entry-through.log',0,'(3811 / 3811)'),
 ('make test-report-through-pa22','fixed-through.log',2,'100-public-qualified-base-typedef-ambiguous-subobject'),
 ('make test-report-through-pa22','final-through.log',0,'(3811 / 3811)'),
 ('make test-pa22','pa22.log',0,'(99 / 99)'),
 ('perl scripts/cppgm_file_audit.pl --stage pa22 --paths dev/src','file-audit.log',0,'File audit passed for pa22 with 3 warning(s).')]:
 p=WORK/name;s=p.read_text();assert needle in s
 result['commands'].append(dict(command=command,log=str(p),log_sha256=sha(p),exit=code,
  provenance='completed exec session exit recorded during audit; this recorder checks retained log and source identity',summary=s.strip().splitlines()[-1]))
assert len(re.findall(r'^===== pa\d+ =====$',(WORK/'final-through.log').read_text(),re.M))==22
primary=Path('/home/vishvananda/work/.ralph/v4codex-gpt-6-astra-xhigh/last-test.log')
result['supplied_primary_log']=dict(path=str(primary),sha256=sha(primary),summary=primary.read_text().strip().splitlines()[-1])
paths=git('diff','--name-only',BASE,CODE,'--','dev').splitlines()
result['changed_implementation_paths']=paths[:]
# Include unchanged phase owners that the independent end-to-end review consumed.
paths+=['dev/src/'+p for p in ['lowering/driver.cpp','lowering/construction.cpp','lowering/lifecycle_order.cpp',
 'semantic/declaration.cpp','semantic/transfer_actions.cpp','semantic/template_instantiation.cpp','semantic/template_binding.h',
 'semantic/fact_store.h','semantic/fact_store.cpp','syntax/occurrence.cpp','syntax/cursor.h','syntax/ast.h',
 'preprocess/source.h','preprocess/token_cursor.h','support/id_index.h','lowir/validator.cpp']]
result['sources']={p:sha(ROOT/p) for p in sorted(set(paths))}
changed_contract=git('diff','--name-only',BASE,CODE,'--','pa*/tests/**','pa*/README.md','pa*/scripts/**','scripts/**','TESTING_AND_REFERENCES.md','spec.md').splitlines()
expected=[
 'pa22/tests/general/100-public-qualified-base-typedef-ambiguous-subobject.ref',
 'pa22/tests/general/300-const-member-function-pointer-address-call.ref',
 'pa22/tests/general/300-structured-bool-conditional-member-pointer-dead-branch.ref',
 'pa22/tests/spec/300-member-pointer-parameter-variadic-deduction.ref',
 'pa22/tests/spec/300-overloaded-member-pointer-function-template-deduction.ref']
assert changed_contract==expected,changed_contract
result['only_contract_changes']=changed_contract
before=git('ls-tree','-r','--name-only',BASE,'--','pa22/tests').splitlines()
now=git('ls-files','pa22/tests').splitlines();assert before==now
for p in now:
 result['fixtures'].append(dict(path=p,sha256=sha(ROOT/p)))
assert sum(p.endswith('.t') for p in now)==99
roundtrips=read(ROOT/'student.tests/pa22/audit120-roundtrips.json');assert len(roundtrips)==99
for row in roundtrips:
 if row['status']=='EXIT_SUCCESS':assert row['stable_roundtrip'] and row['lowir_sha256']==sha((ROOT/row['source']).with_suffix('.my'))
assert sum(r['status']=='EXIT_SUCCESS' for r in roundtrips)==95
controls={}
for p in sorted((ROOT/'student.tests/pa22').glob('audit120-inherited*.json')):
 d=read(p);rows=d if isinstance(d,list) else d.get('cases',[])
 if rows:assert all(x['passed'] for x in rows)
 else:assert d['passed']
 controls[p.name]=len(rows)
assert sum(controls.values())==126
new=read(ROOT/'student.tests/pa22/audit120-controls.json');assert len(new['cases'])==26 and all(c['passed'] for c in new['cases'])
assert new['compiler_sha256']==final
assert read(ROOT/'student.tests/pa22/reference120.json')['passed']
for label in ['common','affected']:
 p=ROOT/('student.tests/pa22/performance120-'+label+'.json');d=read(p)
 assert d['binaries'][1]['sha256']==final and d['binaries'][0]['sha256']==sha(WORK/'entry/cppgm++')
 for w in d['workloads'].values():
  assert w['identical_lowir'] and all(o['checked_exit']==0 for o in w['outputs'])
  for phase in ['compiler','runtime']:
   m=w[phase];assert len(m['observations'])==20 and len(m['paired_b_over_a'])==4
   assert all(o['checked_exit']==0 for o in m['observations'])
native=read(ROOT/'student.tests/pa22/audit120-native.json');assert len(native)==14
assert all(r['native_text_sha256'][0]==r['native_text_sha256'][1] for r in native)
result.update(inherited_controls=controls,new_controls=26,stable_roundtrips=95,rejections=4,performance_workloads=14,
 required_exit_status='pass',unreviewed_handoffs=[],remaining_in_scope_defects=[],
 deferred=['PA23 virtual inheritance and polymorphic multiple inheritance/broader RTTI','PA24+ native backend, later optimization/debug and PA34 self-hosting'])
for pattern in ['audit120*.json','performance120*.json','reference120.json']:
 for p in (ROOT/'student.tests/pa22').glob(pattern):
  if p.name!='audit120-validation.json':result['evidence'][str(p.relative_to(ROOT))]=sha(p)
result['evidence']['student.tests/pa22/benchmark120.py']=sha(ROOT/'student.tests/pa22/benchmark120.py')
(ROOT/'student.tests/pa22/audit120-validation.json').write_text(json.dumps(result,indent=2)+'\n')
print('Sealed:',len(result['sources']),'reviewed source identities;',len(result['commits']),'stage commits; 152 personal controls; 99 course cases; 14 benchmarks')
