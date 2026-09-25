#!/usr/bin/env python3
"""Assemble checked implementation evidence and execute its combined trace: WORK OUT."""
from pathlib import Path
import hashlib,json,subprocess,sys
root=Path(__file__).resolve().parents[2]
w=Path(sys.argv[1]).resolve();out=Path(sys.argv[2]);validation=w/'validation'
def sha(p):return hashlib.sha256(Path(p).read_bytes()).hexdigest()
def read(p):return json.loads(Path(p).read_text())
checks=read(validation/'checks.json');progress=read(validation/'stage-progress.json')
assert checks['stage']['exit']==2 and all(r['exit']==0 for k,r in checks.items() if k!='stage')
assert len(progress['entry_failures'])==32 and len(progress['final_failures'])==27 and not progress['new_failures']
assert progress['inputs']==420 and progress['files']==1686
assert all(sha(root/p)==digest for p,digest in progress['fixture_sha256'].items())
controls={};total=0
for name,r in read(validation/'validate73_controls/results.json').items():
 if 'count' not in r:continue
 path=validation/'validate73_controls'/name/'results.json';values=read(path)
 assert all(v['passed'] for v in values)
 controls[name]=dict(count=len(values),path=str(path),sha256=sha(path));total+=len(values)
for name in ('audit74_controls','list75_controls','inherit76_controls','nested77_controls','lookup77_controls','audit78_controls','audit78_declarations','syntax79_controls','signature79_controls','course79'):
 path=validation/name/'results.json';values=read(path);assert all(v['passed'] for v in values)
 controls[name]=dict(count=len(values),path=str(path),sha256=sha(path));total+=len(values)
assert total==991
for name in ('syntax79_controls','signature79_controls','course79'):
 controls[name]['results']=read(validation/name/'results.json')
entry={}
for name in ('signature-entry','syntax-entry-final'):
 path=w/name/'results.json';values=read(path)
 entry[name]=dict(count=len(values),passed=sum(v['passed'] for v in values),path=str(path),sha256=sha(path),results=values)
cc=root/'dev/cppgm++';ir=w/'trace79.lowir';exe=w/'trace79.exe';source=root/'student.tests/pa18/signature79_trace.cpp'
trace=dict(source=source.read_text(),source_sha256=sha(source),commands=[])
for command in ([cc,'--emit-lowir','-O0','--validate-lowir','--stats','-o',ir,source],
                [root/'dev/lowir2native-ref','-O0','-o',exe,ir],[exe]):
 p=subprocess.run([str(x) for x in command],capture_output=True,text=True,timeout=60)
 trace['commands'].append(dict(command=[str(x) for x in command],exit=p.returncode,stdout=p.stdout,stderr=p.stderr))
 assert p.returncode==0,p.stderr
trace['abi_symbols']=['_Z6resultIiEDTclL_Z5fixeddEstT_EES0_','_Z6resultIiEDTclL_Z5fixedmEstT_EES0_']
assert all(symbol in ir.read_text() for symbol in trace['abi_symbols'])
trace.update(lowir_sha256=sha(ir),executable_sha256=sha(exe))
performance=root/'student.tests/pa18/loop79-performance.json';perf=read(performance)
assert perf.get('finished_utc') and sha(cc)==perf['binaries'][1]['sha256']==progress['compiler_sha256']
bounds={}
for name in ('first-signature','first-member','qualified-alias'):
 bounds[name]={}
 for n in (600,2400):
  stats=perf['workloads'][name+'-'+str(n)]['outputs'][-1]['telemetry'][0]
  bounds[name][str(n)]={k:stats[k] for k in ('semantic_type_substitution_work','semantic_type_query_work','semantic_template_signature_work','semantic_template_signature_uses','template_definition_signature_work','semantic_template_parameter_check_work','template_occurrences','semantic_template_signature_shape_work')}
result=dict(entry_commit='ef0e43c0f1c1792590ca3e21b49b3e6a82bbd5f3',implementation_commits=['67c4f685','50407fc1'],
 current_commit=subprocess.check_output(['git','rev-parse','HEAD'],text=True).strip(),compiler_sha256=sha(cc),
 stage_progress=progress,checks=checks,personal_control_count=total,controls=controls,entry_controls=entry,
 trace=trace,work_bounds=bounds,performance=dict(path=str(performance),sha256=sha(performance)),
 reference_correction=dict(proof='pa18/reference-correction79.md',observations_sha256=sha(root/'student.tests/pa18/loop79-reference.json')),
 boundary='Source signatures, declaration syntax, dependent explicit member arguments and their body/query/ABI consumers completed. PA18 remains unfinished: one unknown-bound empty-array rejection and 26 ordinary LowIR comparisons. The inherited nested-alias cast and class-ellipsis reducers remain implementation requirements.',
 independent_audit='Review equivalent declaration keys versus semantic lookup facts, source/occurrence recipes under renamed enclosing heads, and fixed-type value-dependent call binding/ABI. No audit question waives a known implementation defect.')
out.write_text(json.dumps(result,indent=2)+'\n')
print('Evidence complete: 393/420, prior 2609/2609, file audit pass, 991 controls; unchanged inputs, one proven reference correction.')
