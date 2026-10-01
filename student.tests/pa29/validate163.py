#!/usr/bin/env python3
"""Record final compiler gates, fixture invariance and the implementation boundary."""
import hashlib,json,pathlib,re,subprocess,sys
root=pathlib.Path(__file__).resolve().parents[2];out=pathlib.Path(sys.argv[1]).resolve();ev=root/'student.tests/pa29/evidence163';ev.mkdir(exist_ok=True)
entry='3bc61ecba6d68eb2821022d7d58ac53ae5305f84'
def sha(data):return hashlib.sha256(data).hexdigest()
def git(*args):return subprocess.check_output(['git',*args],cwd=root)
def save(name,value):(ev/name).write_text(json.dumps(value,indent=2)+'\n')
checks={}
def check(name,args,expected):
 log=out/(name+'.log')
 with log.open('w') as f:p=subprocess.run(args,cwd=root,stdout=f,stderr=subprocess.STDOUT,timeout=1200)
 checks[' '.join(map(str,args))]=dict(exit_status=p.returncode,expected=expected,log=str(log),log_sha256=sha(log.read_bytes()))
 assert p.returncode==expected,(name,p.returncode,log)
 print(name,p.returncode,flush=True)
check('prior-final',['make','test-report-through-pa28'],0)
check('stage-final',['make','test-pa29'],2)
check('through-final',['make','test-report-through-pa29'],2)
check('file-audit-final',['perl','scripts/cppgm_file_audit.pl','--stage','pa29','--paths','dev/src'],0)
for name,script in [('controls','controls163.py'),('controls161','controls161.py'),('controls162','controls162.py')]:
 check(name+'-final',['python3',str(root/'student.tests/pa29'/script),str(out/(name+'-final'))],0)
 data=json.loads((out/(name+'-final')/'results.json').read_text());assert all(row['passed'] for row in data['checks']);save(name+'.json',data)
check('inspection-final',['python3',str(root/'student.tests/pa29/inspection163.py'),str(out/'inspection-final'),str(out/'ir-object')],0)
save('inspection.json',json.loads((out/'inspection-final/inspection.json').read_text()))
def failures(path):return sorted(set(re.findall(r'(pa29/tests/[^\s:]+\.t): ERROR:',path.read_text())))
before=failures(out/'baseline.log');after=failures(out/'stage-final.log');assert len(before)==65 and len(after)==58 and set(after)<set(before)
assert failures(out/'through-final.log')==after
assert '4538 / 4538' in (out/'prior-final.log').read_text()
assert '345 / 403' in (out/'stage-final.log').read_text()
assert '4883/4941' in (out/'through-final.log').read_text().replace(' ','')
fixtures={}
for name in git('ls-files','pa29/tests').decode().splitlines():
 data=(root/name).read_bytes();assert data==git('show',entry+':'+name);fixtures[name]=sha(data)
assert sum(p.endswith('.t') for p in fixtures)==403
assert not git('diff',entry,'--','pa29/tests','pa29/scripts','scripts','Makefile','pa29/Makefile')
save('coverage.json',dict(source_count=403,all_fixtures_and_sidecars_unchanged=True,fixture_sha256=fixtures,comparison_and_discovery_changes=[],reference_corrections=[]))
save('stage-delta.json',dict(entry=entry,code_tip=git('rev-parse','HEAD').decode().strip(),total=403,before_pass=338,after_pass=345,entry_failures=before,final_failures=after,new_failures=[],resolved=sorted(set(before)-set(after))))
save('validation.json',dict(entry=entry,code_tip=git('rev-parse','HEAD').decode().strip(),compiler_sha256=sha((root/'dev/cppgm++').read_bytes()),checks=checks,stage_progress='PASS: seven existing failures removed; no new failures or coverage change',prior_passed=4538,stage_passed=345,stage_total=403,independent_review='pending; review markers preserved',inspection_correction=dict(initial_assertion='host ELF DWARF line table exists',initial_outcome='failed: inherited host writer has no .debug_line',contract='pa8/lowir.md Debug Locations explicitly permits backends without object-file debug information',final_checks='LowIR/MIR debug transport; ELF nop/pause encoding; no required check removed')))
print('PA29 implementation163 validation handoff: PASS',flush=True)
