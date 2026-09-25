#!/usr/bin/env python3
"""Explicit implementation 85 checks: WORK ENTRY_LOG."""
from pathlib import Path
import hashlib,json,re,subprocess,sys
ROOT=Path(__file__).resolve().parents[2];W=Path(sys.argv[1]).resolve();W.mkdir(parents=True,exist_ok=True)
ENTRY='f953e42a875bc8f7e3b9882a324db843f0c4191a';cc=ROOT/'dev/cppgm++';checks={}
def sha(p):return hashlib.sha256(Path(p).read_bytes()).hexdigest()
def check(name,command,expected=0):
 p=subprocess.run([str(x) for x in command],cwd=ROOT,capture_output=True,text=True)
 log=W/(name+'.log');log.write_text(p.stdout+p.stderr)
 checks[name]=dict(command=[str(x) for x in command],exit=p.returncode,path=str(log),sha256=sha(log))
 (W/'checks.json').write_text(json.dumps(checks,indent=2)+'\n');print(name,p.returncode,flush=True)
 assert p.returncode==expected,(name,p.stderr)
check('stage',['make','test-pa18'],2)
check('prior',['make','test-report-through-pa17'])
check('file-audit',['perl','scripts/cppgm_file_audit.pl','--stage','pa18','--paths','dev/src'])
for script in ('validate73_controls','audit74_controls','list75_controls','inherit76_controls','nested77_controls','lookup77_controls','audit78_controls','audit78_declarations','syntax79_controls','signature79_controls','course79','scalar80_controls','result81_controls','audit82_controls','array83_controls','object84_controls','discard85_controls','storage85_controls','discard85_scaling','result81_inspection','list75_scaling','inherit76_scaling','nested77_scaling','list75_course','inherit76_course'):
 check(script,['python3',ROOT/'student.tests/pa18'/(script+'.py'),cc,W/script])
check('abi',['python3',ROOT/'student.tests/pa18/list75_abi.py',ROOT/'dev/abimangle','/tmp/pa18-loop75/check-api',W/'list75_controls',W/'abi'])
for name in ('signature79','scalar80','result81','audit82','array83','object84','discard85'):
 check(name+'-trace',[cc,'--emit-lowir','-O0','--validate-lowir','--stats','-o',W/(name+'.lowir'),ROOT/'student.tests/pa18'/(name+'_trace.cpp')])
 check(name+'-backend',[ROOT/'dev/lowir2native-ref','-O0','-o',W/(name+'.exe'),W/(name+'.lowir')])
 check(name+'-execution',[W/(name+'.exe')])
check('reference-proof',['python3',ROOT/'student.tests/pa18/reference85.py'])
check('reference-observations',['python3',ROOT/'student.tests/pa18/reference85_observe.py',W/'reference-observations'])
check('empty-reference-proof',['python3',ROOT/'student.tests/pa18/reference84.py'])
# Previous corrected references and array controls remain explicitly validated.
check('array-reference-proof',['python3',ROOT/'student.tests/pa18/reference83.py'])
check('array-reference-observations',['python3',ROOT/'student.tests/pa18/reference83_observe.py',W/'array-reference-observations'])
check('pa16-array-controls',['python3',ROOT/'student.tests/pa16/initialization.py',cc,W/'pa16-array-controls'])
import ordering_controls as runner
runner.GOOD={p:(ROOT/'pa18/tests/general'/(p+'.t')).read_text() for p in ('300-abstract-array-parameter-sfinae','500-constructor-pack-default-rewritten-pointer','300-function-template-nested-alias-explicit-call','500-bool-alias-function-template-result-metadata')};runner.BAD={}
assert runner.run(cc,W/'course81')
runner.GOOD={p:(ROOT/'pa18/tests/general'/(p+'.t')).read_text() for p in ('100-dependent-remove-reference-transform-forwarding','300-adl-overload-set-argument-deduction','500-inherited-constructor-template-member-alias-pack')}
assert runner.run(cc,W/'course84')
def failures(p):return sorted(set(re.findall(r'^(pa18/[^:]+): ERROR:',Path(p).read_text(),re.M)))
a,b=failures(sys.argv[2]),failures(W/'stage.log');assert len(a)==6 and len(b)==3 and set(b)<set(a)
files=subprocess.check_output(['git','ls-files','pa18/tests'],cwd=ROOT,text=True).splitlines()
assert len(files)==1686 and len([p for p in files if p.endswith('.t')])==420
revisions=json.loads((ROOT/'student.tests/pa18/reference85-revisions.json').read_text())
allowed={r['path']:r for r in revisions['revisions']};suites=[f'pa{i}/tests' for i in range(1,19)]
changed=subprocess.check_output(['git','diff','--name-only',ENTRY,'--',*suites],cwd=ROOT,text=True).splitlines()
assert set(changed)==set(allowed)
for p in changed:
 before=subprocess.check_output(['git','show',ENTRY+':'+p],cwd=ROOT)
 assert hashlib.sha256(before).hexdigest()==allowed[p]['before_sha256']
 assert sha(ROOT/p)==allowed[p]['after_sha256']
original=subprocess.check_output(['git','ls-tree','-r','--name-only',ENTRY,'--',*suites],cwd=ROOT,text=True).splitlines()
current=subprocess.check_output(['git','ls-files',*suites],cwd=ROOT,text=True).splitlines();assert original==current
assert not subprocess.check_output(['git','diff',ENTRY,'--','scripts','Makefile','spec.md','AGENTS.md'],cwd=ROOT)
result=dict(entry_failures=a,final_failures=b,new_failures=sorted(set(b)-set(a)),inputs=420,files=len(files),resolved_failures=sorted(set(a)-set(b)),fixture_sha256={p:sha(ROOT/p) for p in files},reference_revisions=allowed,compiler_sha256=sha(cc))
(W/'stage-progress.json').write_text(json.dumps(result,indent=2)+'\n')
print('Verified progress: 6 -> 3 failures; 420 unchanged inputs; 1686 retained fixture files; three standard-proved oracle edits.',flush=True)
