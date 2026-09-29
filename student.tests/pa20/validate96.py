#!/usr/bin/env python3
"""Required PA20 handoff checks, personal controls and unchanged-coverage proof."""
from pathlib import Path
import hashlib,json,re,subprocess,sys,time
ROOT=Path(__file__).resolve().parents[2]
WORK,OUT=[Path(p).resolve() for p in sys.argv[1:]]
WORK.mkdir(parents=True,exist_ok=True)
ENTRY='e59e46fa8ccef7dda44cbf57f5d9f4075801861a'
def sha(p):return hashlib.sha256(Path(p).read_bytes()).hexdigest()
result=dict(entry=ENTRY,compiler_sha256=sha(ROOT/'dev/cppgm++'),checks={})
result['handoff']={'stage_base': 'a9b24ab68f1a75288df10161cb171fa239e1409a', 'last_reviewed': 'a9b24ab68f1a75288df10161cb171fa239e1409a', 'implementation_commits': ['beb83901', '6fbfd99f', '5d2e6a65'], 'complete_stage': False, 'implementation_handoff': True, 'completed_groups': ['scalar array member helper representation and parameter shape', 'captureless callable entry/default ownership and per-entry storage'], 'unfinished_implementation': [{'owner': 'nontrivial aggregate helper ABI', 'required_failures': 2}, {'owner': 'capture environments', 'required_failures': 17}, {'owner': 'constructor conversion integration', 'required_failures': 2}, {'owner': 'retained declarations/lifecycle', 'required_failures': 2}], 'boundary': 'Remaining work needs capture/environment records, constructor/member transfer ABI decisions, or retained declaration/lifecycle facts; it cannot be supplied by scalar-array transport or callable entry/default changes.', 'independent_review': {'status': 'pending', 'commits': ['beb83901', '6fbfd99f', '5d2e6a65', '2cbd6b8d', '14fff139', '58c89851', 'fe0c1722', 'dbd1a8f6', 'df239d8e', '0dd795a1'], 'questions': ['helper key completeness and initializer sequencing', 'closure entry demand, shared body facts, default binding and per-function storage', 'stage-scoped performance cost acceptance', 'prior range/deduction architecture and reference corrections']}, 'requirements_waived': False}
def check(name,command):
 start=time.monotonic()
 p=subprocess.run(command,cwd=ROOT,stdout=subprocess.PIPE,stderr=subprocess.STDOUT,text=True)
 log=WORK/(name+'.log');log.write_text(p.stdout)
 result['checks'][name]=dict(command=command,exit=p.returncode,elapsed_s=time.monotonic()-start,
     log=str(log),log_sha256=sha(log),summaries=[l for l in p.stdout.splitlines() if '=====' in l or 'audit passed' in l or 'warning]' in l])
 OUT.write_text(json.dumps(result,indent=2)+'\n');print(name,p.returncode,flush=True);return p
stage=check('stageTests',['make','test-pa20'])
prior=check('priorThroughTests',['bash','-c',"n=20; if [ \"$n\" -le 1 ]; then echo '===== ALL TESTS PASSED SUCCESSFULLY! (0/0) ====='; else make test-report-through-pa$((n - 1)); fi"])
audit=check('fileAudit',['perl','scripts/cppgm_file_audit.pl','--stage','pa20','--paths','dev/src'])
through=check('throughPA20',['make','test-report-through-pa20'])
controls=[]
for name,version in [('deduction',94),('initialization',94),('range',95),('aggregate',96),('closure',96)]:
 work=WORK/name
 p=check(name,['python3',str(ROOT/f'student.tests/pa20/{name}{version}.py'),str(ROOT/'dev/cppgm++'),str(work)])
 rows=json.loads((work/'results.json').read_text());controls+=rows
 (ROOT/f'student.tests/pa20/{name}96-final.json').write_text(json.dumps(rows,indent=2)+'\n')
 assert p.returncode==0 and all(r['passed'] for r in rows)
entry=set(json.loads((ROOT/'student.tests/pa20/validation95.json').read_text())['progress']['current_failures'])
current=set(re.findall(r'^(pa20/[^:]+): ERROR:',stage.stdout,re.M))
assert len(entry)==34 and not current-entry,sorted(current-entry)
count=re.search(r'TEST SUMMARY: (\d+) / (\d+) TESTS PASSED',stage.stdout)
assert count and int(count[2])==144 and int(count[1])==144-len(current)
assert len(current)<len(entry) and prior.returncode==audit.returncode==0
result['progress']=dict(entry_pass=110,total=144,entry_failures=sorted(entry),current_failures=sorted(current),
 fixed=sorted(entry-current),new_failures=sorted(current-entry),current_pass=int(count[1]),accepted=True)
paths=subprocess.check_output(['git','ls-files','pa20/tests','scripts','shared','pa20/Makefile','pa20/README.md'],cwd=ROOT,text=True).splitlines()
manifest=[]
for path in paths:
 p=ROOT/path;content=str(p.readlink()).encode() if p.is_symlink() else p.read_bytes()
 old=subprocess.check_output(['git','show',ENTRY+':'+path],cwd=ROOT)
 assert content==old,path
 manifest.append([path,hashlib.sha256(content).hexdigest()])
result['coverage']=dict(files=len(manifest),unchanged_files=len(manifest),manifest_sha256=hashlib.sha256(json.dumps(manifest).encode()).hexdigest(),
 source_inputs=sum(p.endswith('.t') for p,_ in manifest),status_sidecars=sum(p.endswith('.ref.exit_status') for p,_ in manifest),
 references=sum(p.endswith('.ref') for p,_ in manifest),comparison='unchanged',reference_corrections=0)
result['controls']=dict(total=len(controls),native=sum(r['expected']=='native' for r in controls),
 rejections=sum(r['expected']=='reject' for r in controls),passed=sum(r['passed'] for r in controls))
assert check('sourceToNativeTrace',['python3',str(ROOT/'student.tests/pa20/trace96.py'),str(ROOT/'dev/cppgm++'),str(WORK/'trace'),str(ROOT/'student.tests/pa20/trace96.json')]).returncode==0
assert sha(ROOT/'dev/cppgm++')==result['compiler_sha256']
OUT.write_text(json.dumps(result,indent=2)+'\n')
