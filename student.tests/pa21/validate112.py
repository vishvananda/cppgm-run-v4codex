#!/usr/bin/env python3
"""PA21 implementation gates and unchanged-coverage evidence, with explicit controls."""
from pathlib import Path
import hashlib,json,re,subprocess,sys
ROOT=Path(__file__).resolve().parents[2]
WORK,OUT=[Path(x).resolve() for x in sys.argv[1:3]]
WORK.mkdir(parents=True,exist_ok=True)
assert not OUT.exists() or '--resume' in sys.argv
sha=lambda p:hashlib.sha256(Path(p).read_bytes()).hexdigest()
def git(*args):return subprocess.check_output(['git',*args],cwd=ROOT,text=True)
entry='9457e2fca6bdb876eaee509b0481f631b13587b4'
result=json.loads(OUT.read_text()) if OUT.exists() else dict(entry=entry,compiler_sha256=sha(ROOT/'dev/cppgm++'),gates={},controls={})
assert result['entry']==entry and result['compiler_sha256']==sha(ROOT/'dev/cppgm++')
def save():OUT.write_text(json.dumps(result,indent=2)+'\n')
commands={
 'stageTests':'make test-pa21',
 'throughStageTests':'make test-report-through-pa21',
 'fileAudit':'perl scripts/cppgm_file_audit.pl --stage pa21 --paths dev/src',
}
prior=Path('/tmp/pa21-112/prior-candidate.log')
assert 'ALL TESTS PASSED SUCCESSFULLY! (3596 / 3596)' in prior.read_text()
assert sha(ROOT/'dev/cppgm++')==sha(Path('/tmp/pa21-112/compiler-B'))
result['gates']['priorThroughTests']=dict(command='make test-report-through-pa20',exit=0,output=prior.read_text(),log_sha256=sha(prior))
for name,cmd in commands.items():
 log=WORK/(name+'.log')
 if name in result['gates']:
  assert result['gates'][name]['exit']==0 and result['gates'][name]['log_sha256']==sha(log)
  continue
 with log.open('w') as f:p=subprocess.run(cmd,shell=True,cwd=ROOT,stdout=f,stderr=subprocess.STDOUT)
 result['gates'][name]=dict(command=cmd,exit=p.returncode,output=log.read_text(),log_sha256=sha(log));save()
 print(name,p.returncode,flush=True)
 assert p.returncode==0,(name,log.read_text())
fail=lambda s:sorted(set(re.findall(r'pa21/tests/[^ :]+\.t(?=: ERROR:)',s)))
before=fail(json.loads((ROOT/'student.tests/pa21/validation111.json').read_text())['gates']['stageTests']['output'])
after=fail(result['gates']['stageTests']['output'])
assert len(before)==2 and not after and not set(after)-set(before)
assert '(116 / 116)' in result['gates']['stageTests']['output']
assert fail(result['gates']['throughStageTests']['output'])==after
assert '(3712 / 3712)' in result['gates']['throughStageTests']['output']
assert '(3596 / 3596)' in result['gates']['priorThroughTests']['output']
result['stageProgress']=dict(entry_failures=before,current_failures=after,resolved=sorted(set(before)-set(after)),cases=116)
paths=[]
for p in git('ls-files').splitlines():
 m=re.match(r'pa(\d+)/(tests/|scripts/|Makefile)',p)
 if (m and int(m[1])<=21) or p.startswith('scripts/') or p in ('Makefile','TESTING_AND_REFERENCES.md'):paths.append(p)
changed=git('diff','--name-only',entry,'--',*paths).splitlines()
from reference112 import changes
assert changed==sorted(changes),changed
sources=[p for p in paths if p.startswith('pa21/tests/') and p.endswith('.t')]
assert len(sources)==116
assert git('ls-tree','-r','--name-only',entry,'pa21/tests').splitlines()==git('ls-files','pa21/tests').splitlines()
result['coverage']=dict(contract_paths=len(paths),only_changes=changed,required_sources={p:sha(ROOT/p) for p in sources},
 inventory_sha256=hashlib.sha256('\n'.join(p+' '+sha(ROOT/p) for p in paths).encode()).hexdigest())
for number in (102,106,108,110,112):
 p=subprocess.run([sys.executable,str(ROOT/f'student.tests/pa21/reference{number}.py'),'--check'],capture_output=True,text=True,cwd=ROOT)
 assert p.returncode==0,p.stderr
 result[f'reference{number}']=dict(exit=p.returncode,output_sha256=hashlib.sha256(p.stdout.encode()).hexdigest())
save()
suites={'dispatch':'pa21/dispatch112.py','automatic_arrays':'pa16/initialization.py','continuation':'pa21/continuation111.py','construction':'pa21/construction110.py','closure':'pa21/closure110.py','jumps':'pa21/audit109.py',
 'ownership':'pa21/ownership108.py','full_expression':'pa21/full_expression107.py','exceptions':'pa21/exceptions106.py',
 'composition':'pa21/audit105.py','prefix_unwind':'pa21/audit105_eh.py','rtti':'pa21/rtti102.py',
 'host_rtti':'pa21/host_rtti102.py','captures':'pa21/capture103.py','capture_unwind':'pa21/capture_eh103.py',
 'lists':'pa21/list104.py','list_unwind':'pa21/list_eh104.py','inherited_captures':'pa20/capture98.py'}
for name,script in suites.items():
 work=WORK/name;log=WORK/(name+'.log')
 if name in result['controls']:
  assert result['controls'][name]['evidence_sha256']==sha(work/'results.json')
  continue
 cmd=[sys.executable,str(ROOT/'student.tests'/script),str(ROOT/'dev/cppgm++'),str(work)]
 with log.open('w') as f:p=subprocess.run(cmd,cwd=ROOT,stdout=f,stderr=subprocess.STDOUT)
 data=json.loads((work/'results.json').read_text());rows=data['rows'] if isinstance(data,dict) else data
 failed=[r['name'] for r in rows if not r.get('passed',r.get('host_exit')==0)]
 assert not failed or (name=='rtti' and failed==['public_base_inside_private_derived']) or (name=='automatic_arrays' and failed==['lifecycle_multitu_0','lifecycle_multitu_1']),(name,failed)
 if name=='automatic_arrays' and failed:
  # Preserve the original results. These two inherited freestanding-backend
  # failures reference __builtin_abort despite the correct object=abort.
  # Require unchanged baseline IR and execute both source orders hosted.
  limits=[]
  baseline=Path('/tmp/pa21-112/compiler-A')
  assert sha(baseline)==json.loads((ROOT/'student.tests/pa21/performance112.json').read_text())['binaries'][0]['sha256']
  for index,label in enumerate(failed):
   ir=work/(label+'.lowir');obj=ir.with_suffix('.o');exe=ir.with_suffix('.host');old=work/(label+'-entry.lowir')
   sources=[ROOT/'student.tests/pa16/initialization'/x for x in ('lifecycle_caller.cpp','lifecycle_second.cpp')]
   if index:sources.reverse()
   commands=[]
   for proof_cmd in ([baseline,'--emit-lowir','-O0','-o',old,*sources],
      [ROOT/'dev/cppgm++-ref','-c','-O0','-o',obj,ir],['g++','-no-pie',obj,'-o',exe],[exe]):
    run=subprocess.run(list(map(str,proof_cmd)),capture_output=True,text=True,timeout=30)
    commands.append(dict(argv=list(map(str,proof_cmd)),exit=run.returncode,stdout=run.stdout,stderr=run.stderr))
    assert run.returncode==0,commands
   assert sha(old)==sha(ir)
   run=subprocess.run([ROOT/'dev/lowir2native-ref','-O0','-o',exe.with_suffix('.native'),ir],capture_output=True,text=True,timeout=30)
   assert run.returncode!=0 and 'undefined native symbol: __builtin_abort' in run.stderr
   limits.append(dict(name=label,entry_and_final_ir_sha256=sha(ir),commands=commands,freestanding_exit=run.returncode,freestanding_diagnostic=run.stderr))
  result['automatic_array_backend_limits']=limits
 assert p.returncode==(1 if failed else 0)
 result['controls'][name]=dict(command=cmd,exit=p.returncode,total=len(rows),passed=len(rows)-len(failed),failed=failed,
   evidence=str(work/'results.json'),evidence_sha256=sha(work/'results.json'),log_sha256=sha(log))
 save();print(name,len(rows)-len(failed),'/',len(rows),flush=True)
work=WORK/'reference112'
p=subprocess.run([sys.executable,str(ROOT/'student.tests/pa21/reference112_execution.py'),str(work)],capture_output=True,text=True,cwd=ROOT)
assert p.returncode==0,p.stderr
result['reference112_execution']=dict(exit=0,evidence=str(work/'results.json'),evidence_sha256=sha(work/'results.json'),rows=json.loads((work/'results.json').read_text())['rows'])
save()
work=WORK/'zero-slot'
p=subprocess.run([sys.executable,str(ROOT/'student.tests/pa21/zero_slot108.py'),str(work)],capture_output=True,text=True,cwd=ROOT)
assert p.returncode==0,p.stderr
rows=json.loads((work/'results.json').read_text())
result['controls']['zero_slot']=dict(exit=0,total=len(rows),passed=len(rows),failed=[],evidence=str(work/'results.json'),evidence_sha256=sha(work/'results.json'))
assert sha(ROOT/'dev/cppgm++')==result['compiler_sha256']
result['implementation_diff_sha256']=hashlib.sha256(git('diff',entry,'--','dev').encode()).hexdigest()
save()
