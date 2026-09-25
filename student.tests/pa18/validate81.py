#!/usr/bin/env python3
"""Explicit loop 81 checks and immutable course coverage: WORK ENTRY_LOG."""
from pathlib import Path
import hashlib,json,re,subprocess,sys
root=Path(__file__).resolve().parents[2];w=Path(sys.argv[1]).resolve();w.mkdir(parents=True,exist_ok=True)
entry=Path(sys.argv[2]).resolve();cc=root/'dev/cppgm++';checks={}
def sha(p):return hashlib.sha256(Path(p).read_bytes()).hexdigest()
def check(name,command,expected=0):
 p=subprocess.run([str(x) for x in command],cwd=root,capture_output=True,text=True)
 log=w/(name+'.log');log.write_text(p.stdout+p.stderr)
 checks[name]=dict(command=[str(x) for x in command],exit=p.returncode,path=str(log),sha256=sha(log))
 (w/'checks.json').write_text(json.dumps(checks,indent=2)+'\n');print(name,p.returncode,flush=True)
 assert p.returncode==expected,(name,p.stderr)
check('stage',['make','test-pa18'],2)
check('prior',['make','test-report-through-pa17'])
check('file-audit',['perl','scripts/cppgm_file_audit.pl','--stage','pa18','--paths','dev/src'])
for script in ('validate73_controls','audit74_controls','list75_controls','inherit76_controls','nested77_controls','lookup77_controls','audit78_controls','audit78_declarations','syntax79_controls','signature79_controls','course79','scalar80_controls','result81_controls','result81_inspection','list75_scaling','inherit76_scaling','nested77_scaling','list75_course','inherit76_course'):
 check(script,['python3',root/'student.tests/pa18'/(script+'.py'),cc,w/script])
check('abi',['python3',root/'student.tests/pa18/list75_abi.py',root/'dev/abimangle','/tmp/pa18-loop75/check-api',w/'list75_controls',w/'abi'])
for name in ('signature79','scalar80','result81'):
 check(name+'-trace',[cc,'--emit-lowir','-O0','--validate-lowir','--stats','-o',w/(name+'.lowir'),root/'student.tests/pa18'/(name+'_trace.cpp')])
 check(name+'-backend',[root/'dev/lowir2native-ref','-O0','-o',w/(name+'.exe'),w/(name+'.lowir')])
 check(name+'-execution',[w/(name+'.exe')])
# Execute both repaired original inputs, plus the inherited nested-alias cast
# whose runtime status was still marked unfinished at entry.
sys.path.insert(0,str(root/'student.tests/pa18'));import ordering_controls as runner
runner.GOOD={p: (root/'pa18/tests/general'/(p+'.t')).read_text() for p in ('300-abstract-array-parameter-sfinae','500-constructor-pack-default-rewritten-pointer','300-function-template-nested-alias-explicit-call','500-bool-alias-function-template-result-metadata')};runner.BAD={}
assert runner.run(cc,w/'course81')
def failures(p):return sorted(set(re.findall(r'^(pa18/[^:]+): ERROR:',Path(p).read_text(),re.M)))
a,b=failures(entry),failures(w/'stage.log');assert len(a)==25 and len(b)<len(a) and set(b)<=set(a)
files=subprocess.check_output(['git','ls-files','pa18/tests'],cwd=root,text=True).splitlines()
assert len([p for p in files if p.endswith('.t')])==420
for p in files:assert (root/p).read_bytes()==subprocess.check_output(['git','show','69247877:'+p],cwd=root),p
result=dict(entry_failures=a,final_failures=b,new_failures=sorted(set(b)-set(a)),inputs=420,files=len(files),resolved_failures=sorted(set(a)-set(b)),fixture_sha256={p:sha(root/p) for p in files},compiler_sha256=sha(cc))
(w/'stage-progress.json').write_text(json.dumps(result,indent=2)+'\n')
print('Verified progress:',len(a),'->',len(b),'failures;',len(files),'unchanged fixture/reference files.',flush=True)
