#!/usr/bin/env python3
"""Explicit PA18 full-stage handoff validation: WORK. Run after performance."""
from pathlib import Path
import concurrent.futures,hashlib,json,re,subprocess,sys
ROOT=Path(__file__).resolve().parents[2];W=Path(sys.argv[1]).resolve();W.mkdir(parents=True,exist_ok=True)
ENTRY='01b47c20b41f68d3142df6a8691fc905c20341a8';cc=ROOT/'dev/cppgm++';checks={}
def sha(p):return hashlib.sha256(Path(p).read_bytes()).hexdigest()
def check(name,command):
 p=subprocess.run([str(x) for x in command],cwd=ROOT,capture_output=True,text=True)
 log=W/(name+'.log');log.write_text(p.stdout+p.stderr)
 result=dict(command=[str(x) for x in command],exit=p.returncode,path=str(log),sha256=sha(log))
 print(name,p.returncode,flush=True)
 return name,result
def record(result):
 name,data=result;checks[name]=data
 (W/'checks.json').write_text(json.dumps(checks,indent=2)+'\n')
 assert data['exit']==0,(name,data)
# These root reports share counts and run strictly sequentially.
for name,command in (
 ('stage',['make','test-pa18']),
 ('prior',['bash','-c','n=18; if [ "$n" -le 1 ]; then echo \'===== ALL TESTS PASSED SUCCESSFULLY! (0/0) =====\'; else make test-report-through-pa$((n - 1)); fi']),
 ('through',['make','test-report-through-pa18']),
 ('file-audit',['perl','scripts/cppgm_file_audit.pl','--stage','pa18','--paths','dev/src'])):
 record(check(name,command))
scripts=('validate73_controls','audit74_controls','list75_controls','inherit76_controls','nested77_controls','lookup77_controls','audit78_controls','audit78_declarations','syntax79_controls','signature79_controls','course79','scalar80_controls','result81_controls','audit82_controls','array83_controls','object84_controls','discard85_controls','storage85_controls','audit86_controls','boundary87_controls','discard85_scaling','result81_inspection','list75_scaling','inherit76_scaling','nested77_scaling','list75_course','inherit76_course')
with concurrent.futures.ThreadPoolExecutor(max_workers=4) as pool:
 futures=[pool.submit(check,s,['python3',ROOT/'student.tests/pa18'/(s+'.py'),cc,W/s]) for s in scripts]
 for f in concurrent.futures.as_completed(futures):record(f.result())
record(check('pa16-array-controls',['python3',ROOT/'student.tests/pa16/initialization.py',cc,W/'pa16-array-controls']))
for number in (83,85,87):record(check('reference-proof-'+str(number),['python3',ROOT/f'student.tests/pa18/reference{number}.py']))
record(check('reference-proof-84-chain',['python3',ROOT/'student.tests/pa18/reference87_chain.py',W/'reference84-chain']))
for name in ('signature79','scalar80','result81','audit82','array83','object84','discard85','audit86'):
 record(check(name+'-trace',[cc,'--emit-lowir','-O0','--validate-lowir','--stats','-o',W/(name+'.lowir'),ROOT/'student.tests/pa18'/(name+'_trace.cpp')]))
 record(check(name+'-backend',[ROOT/'dev/lowir2native-ref','-O0','-o',W/(name+'.exe'),W/(name+'.lowir')]))
 record(check(name+'-execution',[W/(name+'.exe')]))
record(check('abi',['python3',ROOT/'student.tests/pa18/list75_abi.py',ROOT/'dev/abimangle','/tmp/pa18-loop75/check-api',W/'list75_controls',W/'abi']))
files=subprocess.check_output(['git','ls-files','pa18/tests'],cwd=ROOT,text=True).splitlines()
assert len(files)==1686 and sum(p.endswith('.t') for p in files)==420
suites=[f'pa{i}/tests' for i in range(1,19)]
original=subprocess.check_output(['git','ls-tree','-r','--name-only',ENTRY,'--',*suites],cwd=ROOT,text=True).splitlines()
current=subprocess.check_output(['git','ls-files',*suites],cwd=ROOT,text=True).splitlines();assert original==current
allowed={r['path']:r for r in json.loads((ROOT/'student.tests/pa18/reference87-revisions.json').read_text())['revisions']}
changed=subprocess.check_output(['git','diff','--name-only',ENTRY,'--',*suites],cwd=ROOT,text=True).splitlines();assert set(changed)==set(allowed)
for p in changed:
 before=subprocess.check_output(['git','show',ENTRY+':'+p],cwd=ROOT)
 assert hashlib.sha256(before).hexdigest()==allowed[p]['before_sha256'] and sha(ROOT/p)==allowed[p]['after_sha256']
assert not subprocess.check_output(['git','diff',ENTRY,'--','scripts','Makefile','spec.md','AGENTS.md'],cwd=ROOT)
assert '420 / 420' in (W/'stage.log').read_text() and '2609 / 2609' in (W/'prior.log').read_text()
assert '3029 / 3029' in (W/'through.log').read_text()
stage=(W/'stage.log').read_text();assert not re.findall(r'^pa18/.*: ERROR:',stage,re.M)
result=dict(entry_failures=3,final_failures=0,inputs=420,files=len(files),reference_revisions=allowed,fixture_sha256={p:sha(ROOT/p) for p in files},compiler_sha256=sha(cc),checks=checks)
(W/'stage-progress.json').write_text(json.dumps(result,indent=2)+'\n')
print('Verified handoff: 3 -> 0 failures; 420 inputs; 1686 fixture paths; only three documented oracle repairs.')
