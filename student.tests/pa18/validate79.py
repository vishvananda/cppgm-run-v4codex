#!/usr/bin/env python3
"""Explicit implementation 79 checks, preserving every exit/log: WORK ENTRY_STAGE_LOG."""
from pathlib import Path
import hashlib,json,re,subprocess,sys
root=Path(__file__).resolve().parents[2];w=Path(sys.argv[1]).resolve();w.mkdir(parents=True,exist_ok=True)
entry=Path(sys.argv[2]).resolve();cc=root/'dev/cppgm++';rows={}
def sha(p):return hashlib.sha256(Path(p).read_bytes()).hexdigest()
def check(name,command,expected=0):
 p=subprocess.run([str(x) for x in command],cwd=root,capture_output=True,text=True)
 log=w/(name+'.log');log.write_text(p.stdout+p.stderr)
 rows[name]=dict(command=[str(x) for x in command],exit=p.returncode,path=str(log),sha256=sha(log))
 (w/'checks.json').write_text(json.dumps(rows,indent=2)+'\n');print(name,p.returncode,flush=True)
 assert p.returncode==expected,(name,p.stderr)
check('stage',['make','test-pa18'],2)
check('prior',['bash','-c','n=18; if [ "$n" -le 1 ]; then echo \'===== ALL TESTS PASSED SUCCESSFULLY! (0/0) =====\'; else make test-report-through-pa$((n - 1)); fi'])
check('file-audit',['perl','scripts/cppgm_file_audit.pl','--stage','pa18','--paths','dev/src'])
for script in ('validate73_controls','audit74_controls','list75_controls','inherit76_controls','nested77_controls','lookup77_controls','audit78_controls','audit78_declarations','syntax79_controls','signature79_controls','course79','list75_scaling','inherit76_scaling','nested77_scaling','list75_course','inherit76_course'):
 check(script,['python3',root/'student.tests/pa18'/(script+'.py'),cc,w/script])
check('abi',['python3',root/'student.tests/pa18/list75_abi.py',root/'dev/abimangle','/tmp/pa18-loop75/check-api',w/'list75_controls',w/'abi'])
check('trace',[cc,'--emit-lowir','-O0','--validate-lowir','--stats','-o',w/'trace.lowir',root/'student.tests/pa18/signature79_trace.cpp'])
check('trace-backend',[root/'dev/lowir2native-ref','-O0','-o',w/'trace.exe',w/'trace.lowir'])
check('trace-execution',[w/'trace.exe'])
trace=(w/'trace.lowir').read_text()
for symbol in ('_Z6resultIiEDTclL_Z5fixeddEstT_EES0_', '_Z6resultIiEDTclL_Z5fixedmEstT_EES0_'):
 assert symbol in trace,symbol
def failures(p):return sorted(set(re.findall(r'^(pa18/[^:]+): ERROR:',Path(p).read_text(),re.M)))
a,b=failures(entry),failures(w/'stage.log');assert len(a)==32 and len(b)<len(a) and set(b)<=set(a)
files=subprocess.check_output(['git','ls-files','pa18/tests'],cwd=root,text=True).splitlines()
assert len([p for p in files if p.endswith('.t')])==420
corrected='pa18/tests/general/300-function-template-result-first-lookup.ref.exit_status'
for p in files:
 before=subprocess.check_output(['git','show','ef0e43c0:'+p],cwd=root)
 if p==corrected: assert before==b'EXIT_SUCCESS\n' and (root/p).read_bytes()==b'EXIT_FAILURE\n'
 else: assert (root/p).read_bytes()==before,p
result=dict(entry_failures=a,final_failures=b,new_failures=sorted(set(b)-set(a)),inputs=420,files=len(files),reference_correction=corrected,resolved_failures=sorted(set(a)-set(b)),fixture_sha256={p:sha(root/p) for p in files},compiler_sha256=sha(cc))
(w/'stage-progress.json').write_text(json.dumps(result,indent=2)+'\n')
print('Stage progress preserved:',len(b),'failures; 420 unchanged inputs;',len(files),'fixture/reference files; one proven exit-status correction.',flush=True)
