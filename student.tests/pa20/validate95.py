#!/usr/bin/env python3
"""Required PA20 handoff checks and unchanged-contract proof. Run WORK OUTPUT."""
from pathlib import Path
import hashlib, json, re, subprocess, sys, time
ROOT = Path(__file__).resolve().parents[2]
WORK, OUT = [Path(p).resolve() for p in sys.argv[1:]]
WORK.mkdir(parents=True,exist_ok=True)
BASE = 'a9b24ab68f1a75288df10161cb171fa239e1409a'
def sha(path): return hashlib.sha256(Path(path).read_bytes()).hexdigest()
ENTRY='ef897177cd34e8bf1e878a7eb94208237240c0f1'
result = dict(base=BASE,entry=ENTRY,compiler_sha256=sha(ROOT/'dev/cppgm++'),checks={})
def check(name, command):
    start=time.monotonic()
    p=subprocess.run(command,cwd=ROOT,stdout=subprocess.PIPE,stderr=subprocess.STDOUT,text=True)
    log=WORK/(name+'.log');log.write_text(p.stdout)
    row=dict(command=command,exit=p.returncode,elapsed_s=time.monotonic()-start,log=str(log),log_sha256=sha(log),
        summaries=[l for l in p.stdout.splitlines() if '=====' in l or 'audit passed' in l or 'warning]' in l])
    result['checks'][name]=row
    OUT.write_text(json.dumps(result,indent=2)+'\n')
    print(name,p.returncode,flush=True)
    return p
stage=check('stageTests',['make','test-pa20'])
prior=check('priorThroughTests',['bash','-c',"n=20; if [ \"$n\" -le 1 ]; then echo '===== ALL TESTS PASSED SUCCESSFULLY! (0/0) ====='; else make test-report-through-pa$((n - 1)); fi"])
audit=check('fileAudit',['perl','scripts/cppgm_file_audit.pl','--stage','pa20','--paths','dev/src'])
through=check('throughPA20',['make','test-report-through-pa20'])
controls=[]
for name in ('deduction','initialization','range'):
    work=WORK/name
    script = name+('95' if name=='range' else '94')
    p=check(name,['python3',str(ROOT/f'student.tests/pa20/{script}.py'),str(ROOT/'dev/cppgm++'),str(work)])
    rows=json.loads((work/'results.json').read_text());controls+=rows
    (ROOT/f'student.tests/pa20/{name}95-final.json').write_text(json.dumps(rows,indent=2)+'\n')
    assert p.returncode==0 and all(r['passed'] for r in rows)
entry=set(json.loads((ROOT/'student.tests/pa20/validation94.json').read_text())['progress']['current_failures'])
failures=lambda log:set(re.findall(r'^(pa20/[^:]+): ERROR:',log,re.M))
current=failures(stage.stdout)
assert len(entry)==53
assert not current-entry, sorted(current-entry)
count=re.search(r'TEST SUMMARY: (\d+) / (\d+) TESTS PASSED',stage.stdout)
assert count and int(count[2])==144 and int(count[1])==144-len(current)
assert len(current)<len(entry) and prior.returncode==audit.returncode==0
result['progress']=dict(entry_pass=91,total=144,entry_failures=sorted(entry),current_failures=sorted(current),
    fixed=sorted(entry-current),new_failures=sorted(current-entry),current_pass=int(count[1]),accepted=True)
paths=subprocess.check_output(['git','ls-files','pa20/tests','scripts','shared','pa20/Makefile','pa20/README.md'],cwd=ROOT,text=True).splitlines()
corrections=json.loads((ROOT/'student.tests/pa20/reference95-revisions.json').read_text())
revised={r['path']:r for r in corrections['revisions']}
manifest=[]
for path in paths:
    p=ROOT/path
    if p.is_symlink():
        content=str(p.readlink()).encode()
    else: content=p.read_bytes()
    old=subprocess.check_output(['git','show',BASE+':'+path],cwd=ROOT)
    if path in revised:
        assert hashlib.sha256(old).hexdigest()==revised[path]['before_sha256']
        assert hashlib.sha256(content).hexdigest()==revised[path]['after_sha256']
    else: assert content==old,path
    manifest.append([path,hashlib.sha256(content).hexdigest()])
result['coverage']=dict(files=len(manifest),unchanged_files=len(manifest)-len(revised),proved_reference_corrections=len(revised),manifest_sha256=hashlib.sha256(json.dumps(manifest).encode()).hexdigest(),
    source_inputs=sum(p.endswith('.t') for p,_ in manifest),status_sidecars=sum(p.endswith('.ref.exit_status') for p,_ in manifest),
    references=sum(p.endswith('.ref') for p,_ in manifest),comparison='unchanged')
result['controls']=dict(total=len(controls),native=sum(r['expected']=='native' for r in controls),rejections=sum(r['expected']=='reject' for r in controls),passed=sum(r['passed'] for r in controls))
assert sha(ROOT/'dev/cppgm++')==result['compiler_sha256']
assert check('referenceReconstruction',['python3',str(ROOT/'student.tests/pa20/reference95.py')]).returncode==0
assert check('nativeRange',['python3',str(ROOT/'student.tests/pa20/native95.py'),str(ROOT/'dev/cppgm++'),str(WORK/'native'),str(ROOT/'student.tests/pa20/native95.json')]).returncode==0
assert check('sourceToNativeTrace',['python3',str(ROOT/'student.tests/pa20/trace95.py'),str(ROOT/'dev/cppgm++'),str(WORK/'trace'),str(ROOT/'student.tests/pa20/trace95.json')]).returncode==0
assert sha(ROOT/'dev/cppgm++')==result['compiler_sha256']
OUT.write_text(json.dumps(result,indent=2)+'\n')
