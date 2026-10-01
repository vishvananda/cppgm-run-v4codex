#!/usr/bin/env python3
"""Summarize completed required commands and unchanged coverage for PA29 handoff164."""
import collections,hashlib,json,pathlib,re,subprocess,sys
root=pathlib.Path(__file__).resolve().parents[2];out=pathlib.Path(sys.argv[1]).resolve();ev=root/'student.tests/pa29/evidence164';ev.mkdir(exist_ok=True)
entry='e5690337eb69b59468d989a143030bd8dbdc8ee0'
def sha(data):return hashlib.sha256(data).hexdigest()
def git(*args):return subprocess.check_output(['git',*args],cwd=root)
def save(name,value):(ev/name).write_text(json.dumps(value,indent=2)+'\n')
def failures(path):return sorted(set(re.findall(r'(pa29/tests/[^\s:]+\.t): ERROR:',path.read_text())))
before=failures(out/'baseline.log');after=failures(out/'stage-required.log')
assert len(before)==58 and len(after)==54 and set(after)<set(before)
assert failures(out/'through-final.log')==after
assert '4538 / 4538' in (out/'prior-final.log').read_text()
assert '349 / 403' in (out/'stage-required.log').read_text()
assert '4887/4941' in (out/'through-final.log').read_text().replace(' ','')
assert 'File audit passed for pa29' in (out/'file-audit-final.log').read_text()
fixtures={}
for name in git('ls-files','pa29/tests').decode().splitlines():
 data=(root/name).read_bytes();assert data==git('show',entry+':'+name);fixtures[name]=sha(data)
assert sum(p.endswith('.t') for p in fixtures)==403
assert not git('diff',entry,'--','pa29/tests','pa29/scripts','scripts','Makefile','pa29/Makefile')
save('coverage.json',dict(source_count=403,all_fixtures_and_sidecars_unchanged=True,fixture_sha256=fixtures,comparison_and_discovery_changes=[],reference_corrections=[]))
save('stage-delta.json',dict(entry=entry,code_tip=git('rev-parse','HEAD').decode().strip(),total=403,before_pass=345,after_pass=349,entry_failures=before,final_failures=after,new_failures=[],resolved=sorted(set(before)-set(after))))
checks={}
for command,log,status in [('make test-pa29','stage-required.log',2),('make test-report-through-pa28','prior-final.log',0),('make test-report-through-pa29','through-final.log',2),('perl scripts/cppgm_file_audit.pl --stage pa29 --paths dev/src','file-audit-final.log',0)]:
 checks[command]=dict(exit_status=status,log=str(out/log),log_sha256=sha((out/log).read_bytes()))
for script,folder,name in [('controls164.py','controls-ref','controls.json'),('inspection164.py','inspection-final','inspection.json')]:
 data=json.loads((out/folder/('results.json' if name=='controls.json' else 'inspection.json')).read_text());assert all(r['passed'] for r in data['checks']);save(name,data)
 checks['python3 student.tests/pa29/'+script+' '+str(out/folder)]=dict(exit_status=0,checks=len(data['checks']))
prior=json.loads((root/'student.tests/pa29/evidence163/remaining.json').read_text())
rows=prior['failures'] if 'failures' in prior else prior['remaining']
rows=[r for r in rows if r['test'] in after]
assert len(rows)==54
save('remaining.json',dict(entry=entry,stage='pa29',passed=349,total=403,counts=dict(collections.Counter(r['owner'] for r in rows)),failures=rows,additional_unfinished=[dict(owner='evaluation-context and initialized storage',reducer='student.tests/pa29/pending164/evaluation-context.cpp',disposition='Unfinished implementation: constexpr result and emitted initialization must agree'),dict(owner='source invocation context',test='pa29/tests/run/800-source-location-builtin-macros-run.t',disposition='Unfinished implementation: source coordinates/caller defaults need separate invocation facts')]))
save('validation.json',dict(entry=entry,code_tip=git('rev-parse','HEAD').decode().strip(),compiler_sha256=sha((root/'dev/cppgm++').read_bytes()),checks=checks,stage_progress='PASS: four existing failures removed; no new failures or coverage change',prior_passed=4538,stage_passed=349,stage_total=403,independent_review='pending; review markers preserved',whole_stage='unfinished implementation and independent audit remain'))
print('PA29 implementation164 validation evidence: PASS',flush=True)
