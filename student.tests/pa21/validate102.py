#!/usr/bin/env python3
"""Validated PA21 implementation checkpoint, not whole-stage certification.

Run CC WORK OUT.json ENTRY_LOG. Preserve observations and the unchanged test
manifest; a supplied freestanding RTTI limitation is checked against host RTTI.
"""
from pathlib import Path
import hashlib,json,re,subprocess,sys,time
ROOT=Path(__file__).resolve().parents[2]
CC,WORK,OUT,ENTRY=[Path(x).resolve() for x in sys.argv[1:]]
BASE='ac988ea33d4997b44e82baaca5a86623fff3127a'
WORK.mkdir(parents=True,exist_ok=True)
assert not OUT.exists()
def sha(p):return hashlib.sha256(Path(p).read_bytes()).hexdigest()
result=dict(stage_base=BASE,compiler_sha256=sha(CC),checks={})
def save():OUT.write_text(json.dumps(result,indent=2)+'\n')
def check(name,command,expected=0):
 start=time.monotonic();p=subprocess.run([str(x) for x in command],cwd=ROOT,capture_output=True,text=True,timeout=1800)
 log=WORK/(name+'.log');log.write_text(p.stdout+p.stderr)
 result['checks'][name]=dict(command=[str(x) for x in command],exit=p.returncode,elapsed_s=time.monotonic()-start,
  log=str(log),log_sha256=sha(log),summaries=[s for s in (p.stdout+p.stderr).splitlines() if '=====' in s or 'audit passed' in s or '[warning]' in s])
 save();print(name,p.returncode,flush=True)
 assert p.returncode==expected,(name,p.returncode,expected,str(log))
 return log
stage=check('stageTests',['make','test-pa21'],2)
check('priorThroughTests',['make','test-report-through-pa20'])
check('fileAudit',['perl','scripts/cppgm_file_audit.pl','--stage','pa21','--paths','dev/src'])
check('freestandingControls',['python3',ROOT/'student.tests/pa21/rtti102.py',CC,WORK/'native'],1)
rows=json.loads((WORK/'native/results.json').read_text())
limitations=[r for r in rows if not r['passed']]
assert [r['name'] for r in limitations]==['public_base_inside_private_derived']
assert limitations[0]['compiler_exit']==0 and limitations[0]['backend_exit']==0 and limitations[0]['native_exit']==1
check('hostRttiControls',['python3',ROOT/'student.tests/pa21/host_rtti102.py',CC,WORK/'host'])
host=json.loads((WORK/'host/results.json').read_text())
assert next(r for r in host['rows'] if r['name']==limitations[0]['name'])['lowir_sha256']==[sha(WORK/'native'/ (limitations[0]['name']+'.lowir'))]
result['controls']=rows;result['host_controls']=host
result['external_limitation']=dict(name=limitations[0]['name'],same_lowir=True,freestanding_exit=1,host_exit=0,owner='supplied freestanding dynamic_cast runtime; compiler emits ABI-correct VMI RTTI')
check('referenceProof',['python3',ROOT/'student.tests/pa21/reference102.py'])
def failures(path):return sorted(set(re.findall(r'^(pa21/\S+\.t): ERROR:',path.read_text(),re.M)))
initial,current=failures(ENTRY),failures(stage)
assert len(initial)==92 and len(current)<len(initial)
assert set(current)<=set(initial)
result['progress']=dict(entry_log_sha256=sha(ENTRY),initial_failures=initial,current_failures=current,resolved=sorted(set(initial)-set(current)),initial_passed=24,current_passed=116-len(current),test_count=116)
changed=subprocess.check_output(['git','diff','--name-only',BASE,'--','pa21/tests'],cwd=ROOT,text=True).splitlines()
assert changed==['pa21/tests/general/100-typeid-template-template-argument-typeinfo-name.ref']
paths=subprocess.check_output(['git','ls-files','pa21/tests','pa21/README.md','pa21/Makefile','pa21/scripts','scripts','shared'],cwd=ROOT,text=True).splitlines()
files=[]
for path in paths:
 p=ROOT/path;content=str(p.readlink()).encode() if p.is_symlink() else p.read_bytes()
 if path not in changed:assert content==subprocess.check_output(['git','show',BASE+':'+path],cwd=ROOT),path
 files.append((path,hashlib.sha256(content).hexdigest()))
result['coverage']=dict(files=len(files),manifest_sha256=hashlib.sha256(json.dumps(files).encode()).hexdigest(),unchanged_except_proved_reference=changed)
result['source_hashes']={str(p.relative_to(ROOT)):sha(p) for p in sorted((ROOT/'dev/src').rglob('*')) if p.is_file()}
assert sha(CC)==result['compiler_sha256'];save()
