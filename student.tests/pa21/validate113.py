#!/usr/bin/env python3
"""Fresh PA21 whole-stage control rerun; compiler WORK OUTPUT."""
from pathlib import Path
import hashlib,json,subprocess,sys
ROOT=Path(__file__).resolve().parents[2]
CC,WORK,OUT=[Path(x).resolve() for x in sys.argv[1:4]]
WORK.mkdir(parents=True,exist_ok=True)
sha=lambda p:hashlib.sha256(Path(p).read_bytes()).hexdigest()
result=json.loads(OUT.read_text()) if OUT.exists() else dict(compiler_sha256=sha(CC),entry_compiler_sha256=sha('/tmp/pa21-113/compiler-A'),controls={})
assert result['compiler_sha256']==sha(CC)
def save():OUT.write_text(json.dumps(result,indent=2)+'\n')
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
 cmd=[sys.executable,str(ROOT/'student.tests'/script),str(CC),str(work)]
 with log.open('w') as f:p=subprocess.run(cmd,cwd=ROOT,stdout=f,stderr=subprocess.STDOUT)
 data=json.loads((work/'results.json').read_text());rows=data['rows'] if isinstance(data,dict) else data
 failed=[r['name'] for r in rows if not r.get('passed',r.get('host_exit')==0)]
 assert not failed or (name=='rtti' and failed==['public_base_inside_private_derived']) or (name=='automatic_arrays' and failed==['lifecycle_multitu_0','lifecycle_multitu_1']),(name,failed)
 if name=='automatic_arrays' and failed:
  # Preserve the original results. These two inherited freestanding-backend
  # failures reference __builtin_abort despite the correct object=abort.
  # Require unchanged baseline IR and execute both source orders hosted.
  limits=[]
  baseline=Path('/tmp/pa21-113/compiler-A')
  assert sha(baseline)==result['entry_compiler_sha256']
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
for number in (102,106,108,110,112,113):
 p=subprocess.run([sys.executable,str(ROOT/f'student.tests/pa21/reference{number}.py'),'--check'],capture_output=True,text=True,cwd=ROOT)
 assert p.returncode==0,p.stderr
 result[f'reference{number}']=dict(exit=p.returncode,output_sha256=hashlib.sha256(p.stdout.encode()).hexdigest())
for number in (112,113):
 work=WORK/f'reference{number}'
 p=subprocess.run([sys.executable,str(ROOT/f'student.tests/pa21/reference{number}_execution.py'),str(work)],capture_output=True,text=True,cwd=ROOT)
 assert p.returncode==0,p.stderr
 result[f'reference{number}_execution']=dict(exit=0,evidence=str(work/'results.json'),evidence_sha256=sha(work/'results.json'),rows=json.loads((work/'results.json').read_text()))
work=WORK/'zero-slot'
p=subprocess.run([sys.executable,str(ROOT/'student.tests/pa21/zero_slot108.py'),str(work)],capture_output=True,text=True,cwd=ROOT)
assert p.returncode==0,p.stderr
rows=json.loads((work/'results.json').read_text())
result['controls']['zero_slot']=dict(exit=0,total=len(rows),passed=len(rows),failed=[],evidence=str(work/'results.json'),evidence_sha256=sha(work/'results.json'))
assert sha(CC)==result['compiler_sha256']
save()
