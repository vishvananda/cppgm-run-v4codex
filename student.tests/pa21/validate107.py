#!/usr/bin/env python3
"""Full-expression implementation gates, coverage inventory and explicit personal suites."""
from pathlib import Path
import hashlib,json,re,subprocess,sys
ROOT=Path(__file__).resolve().parents[2]
WORK,OUT=[Path(p).resolve() for p in sys.argv[1:]]
WORK.mkdir(parents=True,exist_ok=True)
assert not OUT.exists()
def sha(p):return hashlib.sha256(Path(p).read_bytes()).hexdigest()
def git(*args):return subprocess.check_output(['git',*args],cwd=ROOT,text=True)
result=dict(entry_commit='1422565795ca5d689fe63bbfbaaba7afa0607858',stage_base='ac988ea33d4997b44e82baaca5a86623fff3127a',compiler_sha256=sha(ROOT/'dev/cppgm++'),gates={},controls={})
def save():OUT.write_text(json.dumps(result,indent=2)+'\n')
commands={
 'priorThroughTests':'n=21; if [ "$n" -le 1 ]; then echo "===== ALL TESTS PASSED SUCCESSFULLY! (0/0) ====="; else make test-report-through-pa$((n - 1)); fi',
 'stageTests':'make test-pa21',
 'throughStageTests':'make test-report-through-pa21',
 'fileAudit':'perl scripts/cppgm_file_audit.pl --stage pa21 --paths dev/src',
}
for name,cmd in commands.items():
 log=WORK/(name+'.log')
 with log.open('w') as f:p=subprocess.run(cmd,shell=True,cwd=ROOT,stdout=f,stderr=subprocess.STDOUT)
 result['gates'][name]=dict(command=cmd,exit=p.returncode,output=log.read_text(),log_sha256=sha(log));save()
 print(name,p.returncode,flush=True)
 assert p.returncode in (0,2) if name in ('stageTests','throughStageTests') else p.returncode==0
fail=lambda text:sorted(set(re.findall(r'pa21/tests/[^ :]+\.t(?=: ERROR:)',text)))
before=fail((WORK/'entry-stage.log').read_text());after=fail(result['gates']['stageTests']['output'])
assert len(before)==24 and len(after)<len(before) and not set(after)-set(before)
assert re.search(r'/\s*116 TESTS PASSED',result['gates']['stageTests']['output'])
assert fail(result['gates']['throughStageTests']['output'])==after
assert '3697 / 3712 TESTS PASSED' in result['gates']['throughStageTests']['output']
(OUT.parent/'through107.json').write_text(json.dumps(dict(result['gates']['throughStageTests'],compiler_sha256=result['compiler_sha256']),indent=2)+'\n')
result['stageProgressPreserved']=dict(passed=True,resolved=sorted(set(before)-set(after)),entry_failures=before,current_failures=after,new_failures=[],required_cases=116)
paths=[]
for path in git('ls-files').splitlines():
 m=re.match(r'pa(\d+)/(tests/|scripts/|Makefile)',path)
 if (m and int(m[1])<=21) or path.startswith('scripts/') or path in ('Makefile','TESTING_AND_REFERENCES.md'):paths.append(path)
changed=git('diff','--name-only',result['stage_base'],'--',*paths).splitlines()
assert changed==sorted(['pa21/tests/general/100-typeid-template-template-argument-typeinfo-name.ref']+[r['path'] for r in json.loads((ROOT/'student.tests/pa21/reference106-revision.json').read_text())['files']]),changed
stage_sources=git('ls-tree','-r','--name-only',result['stage_base'],'pa21/tests').splitlines()
assert stage_sources==git('ls-files','pa21/tests').splitlines()
result['coverage']=dict(contract_paths=len(paths),only_stage_change=changed,required_sources={p:sha(ROOT/p) for p in stage_sources if p.endswith('.t')},fixture_and_harness_sha256={p:sha(ROOT/p) for p in paths})
reference=subprocess.run([sys.executable,'student.tests/pa21/reference102.py'],cwd=ROOT,capture_output=True,text=True)
assert reference.returncode==0
result['reference']=json.loads(reference.stdout);save()
reference=subprocess.run([sys.executable,'student.tests/pa21/reference106.py'],cwd=ROOT,capture_output=True,text=True)
assert reference.returncode==0
result['reference106']=json.loads(reference.stdout)
reference=subprocess.run([sys.executable,'student.tests/pa21/reference106_execution.py',str(WORK/'reference106')],cwd=ROOT,capture_output=True,text=True)
assert reference.returncode==0
result['reference106_execution']=json.loads(reference.stdout);save()
suites={'full_expression':'pa21/full_expression107.py','source_exceptions':'pa21/exceptions106.py','composition':'pa21/audit105.py','prefix_unwind':'pa21/audit105_eh.py','rtti':'pa21/rtti102.py',
        'host_rtti':'pa21/host_rtti102.py','captures':'pa21/capture103.py','capture_unwind':'pa21/capture_eh103.py',
        'lists':'pa21/list104.py','list_unwind':'pa21/list_eh104.py','inherited_captures':'pa20/capture98.py'}
for name,script in suites.items():
 work=WORK/('validated-'+name);log=WORK/('validated-'+name+'.log')
 cmd=[sys.executable,str(ROOT/'student.tests'/script),str(ROOT/'dev/cppgm++'),str(work)]
 with log.open('w') as f:p=subprocess.run(cmd,cwd=ROOT,stdout=f,stderr=subprocess.STDOUT)
 data=json.loads((work/'results.json').read_text());rows=data['rows'] if isinstance(data,dict) else data
 failed=[r['name'] for r in rows if not r.get('passed',r.get('host_exit')==0)]
 assert not failed or (name=='rtti' and failed==['public_base_inside_private_derived']),(name,failed)
 assert p.returncode==(1 if failed else 0)
 result['controls'][name]=dict(command=cmd,exit=p.returncode,total=len(rows),passed=len(rows)-len(failed),failed=failed,evidence=data)
 save();print(name,len(rows)-len(failed),'/',len(rows),flush=True)
assert sha(ROOT/'dev/cppgm++')==result['compiler_sha256']
result['validated_worktree_diff_sha256']=hashlib.sha256(git('diff',result['entry_commit'],'--','dev','student.tests/pa21').encode()).hexdigest()
save()
