#!/usr/bin/env python3
"""PA21 implementation gates and unchanged-coverage evidence, with explicit controls."""
from pathlib import Path
import hashlib,json,re,subprocess,sys
ROOT=Path(__file__).resolve().parents[2]
WORK,OUT=[Path(x).resolve() for x in sys.argv[1:]]
WORK.mkdir(parents=True,exist_ok=True)
assert not OUT.exists()
sha=lambda p:hashlib.sha256(Path(p).read_bytes()).hexdigest()
def git(*args):return subprocess.check_output(['git',*args],cwd=ROOT,text=True)
entry='0e5a32ddec4027d67d38aa98263f964b383dbed2'
result=dict(entry=entry,compiler_sha256=sha(ROOT/'dev/cppgm++'),gates={},controls={})
def save():OUT.write_text(json.dumps(result,indent=2)+'\n')
commands={
 'stageTests':'make test-pa21',
 'throughStageTests':'make test-report-through-pa21',
 'fileAudit':'perl scripts/cppgm_file_audit.pl --stage pa21 --paths dev/src',
}
prior=Path('/tmp/pa21-111/prior-candidate.log')
assert 'ALL TESTS PASSED SUCCESSFULLY! (3596 / 3596)' in prior.read_text()
assert sha(ROOT/'dev/cppgm++')==sha(Path('/tmp/pa21-111/compiler-B'))
result['gates']['priorThroughTests']=dict(command='make test-report-through-pa20',exit=0,output=prior.read_text(),log_sha256=sha(prior))
for name,cmd in commands.items():
 log=WORK/(name+'.log')
 with log.open('w') as f:p=subprocess.run(cmd,shell=True,cwd=ROOT,stdout=f,stderr=subprocess.STDOUT)
 result['gates'][name]=dict(command=cmd,exit=p.returncode,output=log.read_text(),log_sha256=sha(log));save()
 print(name,p.returncode,flush=True)
 assert p.returncode in (0,2) if name in ('stageTests','throughStageTests') else p.returncode==0
fail=lambda s:sorted(set(re.findall(r'pa21/tests/[^ :]+\.t(?=: ERROR:)',s)))
before=fail(json.loads((ROOT/'student.tests/pa21/validation110.json').read_text())['gates']['stageTests']['output'])
after=fail(result['gates']['stageTests']['output'])
assert len(before)==3 and len(after)<3 and not set(after)-set(before)
assert re.search(r'/\s*116 TESTS PASSED',result['gates']['stageTests']['output'])
assert fail(result['gates']['throughStageTests']['output'])==after
assert f'{3712-len(after)} / 3712 TESTS PASSED' in result['gates']['throughStageTests']['output']
result['stageProgress']=dict(entry_failures=before,current_failures=after,resolved=sorted(set(before)-set(after)),cases=116)
paths=[]
for p in git('ls-files').splitlines():
 m=re.match(r'pa(\d+)/(tests/|scripts/|Makefile)',p)
 if (m and int(m[1])<=21) or p.startswith('scripts/') or p in ('Makefile','TESTING_AND_REFERENCES.md'):paths.append(p)
changed=git('diff','--name-only',entry,'--',*paths).splitlines()
assert changed==[],changed
sources=[p for p in paths if p.startswith('pa21/tests/') and p.endswith('.t')]
assert len(sources)==116
assert git('ls-tree','-r','--name-only',entry,'pa21/tests').splitlines()==git('ls-files','pa21/tests').splitlines()
result['coverage']=dict(contract_paths=len(paths),only_changes=changed,required_sources={p:sha(ROOT/p) for p in sources},
 inventory_sha256=hashlib.sha256('\n'.join(p+' '+sha(ROOT/p) for p in paths).encode()).hexdigest())
for number in (102,106,108,110):
 p=subprocess.run([sys.executable,str(ROOT/f'student.tests/pa21/reference{number}.py')],capture_output=True,text=True,cwd=ROOT)
 assert p.returncode==0,p.stderr
 result[f'reference{number}']=dict(exit=p.returncode,output_sha256=hashlib.sha256(p.stdout.encode()).hexdigest())
save()
suites={'continuation':'pa21/continuation111.py','construction':'pa21/construction110.py','closure':'pa21/closure110.py','jumps':'pa21/audit109.py',
 'ownership':'pa21/ownership108.py','full_expression':'pa21/full_expression107.py','exceptions':'pa21/exceptions106.py',
 'composition':'pa21/audit105.py','prefix_unwind':'pa21/audit105_eh.py','rtti':'pa21/rtti102.py',
 'host_rtti':'pa21/host_rtti102.py','captures':'pa21/capture103.py','capture_unwind':'pa21/capture_eh103.py',
 'lists':'pa21/list104.py','list_unwind':'pa21/list_eh104.py','inherited_captures':'pa20/capture98.py'}
for name,script in suites.items():
 work=WORK/name;log=WORK/(name+'.log')
 cmd=[sys.executable,str(ROOT/'student.tests'/script),str(ROOT/'dev/cppgm++'),str(work)]
 with log.open('w') as f:p=subprocess.run(cmd,cwd=ROOT,stdout=f,stderr=subprocess.STDOUT)
 data=json.loads((work/'results.json').read_text());rows=data['rows'] if isinstance(data,dict) else data
 failed=[r['name'] for r in rows if not r.get('passed',r.get('host_exit')==0)]
 assert not failed or (name=='rtti' and failed==['public_base_inside_private_derived']),(name,failed)
 assert p.returncode==(1 if failed else 0)
 result['controls'][name]=dict(command=cmd,exit=p.returncode,total=len(rows),passed=len(rows)-len(failed),failed=failed,
   evidence=str(work/'results.json'),evidence_sha256=sha(work/'results.json'),log_sha256=sha(log))
 save();print(name,len(rows)-len(failed),'/',len(rows),flush=True)
work=WORK/'zero-slot'
p=subprocess.run([sys.executable,str(ROOT/'student.tests/pa21/zero_slot108.py'),str(work)],capture_output=True,text=True,cwd=ROOT)
assert p.returncode==0,p.stderr
rows=json.loads((work/'results.json').read_text())
result['controls']['zero_slot']=dict(exit=0,total=len(rows),passed=len(rows),failed=[],evidence=str(work/'results.json'),evidence_sha256=sha(work/'results.json'))
assert sha(ROOT/'dev/cppgm++')==result['compiler_sha256']
result['implementation_diff_sha256']=hashlib.sha256(git('diff',entry,'--','dev').encode()).hexdigest()
save()
