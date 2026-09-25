#!/usr/bin/env python3
"""Verify and assemble loop 80 handoff evidence: WORK OUT."""
from pathlib import Path
import hashlib,json,subprocess,sys
root=Path(__file__).resolve().parents[2];w=Path(sys.argv[1]).resolve();out=Path(sys.argv[2]);v=w/'validation-final'
def sha(p):return hashlib.sha256(Path(p).read_bytes()).hexdigest()
def read(p):return json.loads(Path(p).read_text())
checks=read(v/'checks.json');progress=read(v/'stage-progress.json')
assert checks['stage']['exit']==2 and all(c['exit']==0 for n,c in checks.items() if n!='stage')
assert len(progress['entry_failures'])==27 and len(progress['final_failures'])==25 and not progress['new_failures']
assert progress['inputs']==420 and all(sha(root/p)==s for p,s in progress['fixture_sha256'].items())
assert '2609 / 2609' in (v/'prior.log').read_text() and '395 / 420' in (v/'stage.log').read_text()
controls={};total=0
for name,c in read(v/'validate73_controls/results.json').items():
 if 'count' not in c:continue
 p=v/'validate73_controls'/name/'results.json';rows=read(p);assert all(r['passed'] for r in rows)
 controls[name]=dict(count=len(rows),path=str(p),sha256=sha(p));total+=len(rows)
for name in ('audit74_controls','list75_controls','inherit76_controls','nested77_controls','lookup77_controls','audit78_controls','audit78_declarations','syntax79_controls','signature79_controls','course79','scalar80_controls','course80'):
 p=v/name/'results.json';rows=read(p);assert all(r['passed'] for r in rows)
 controls[name]=dict(count=len(rows),path=str(p),sha256=sha(p));total+=len(rows)
 if name in ('scalar80_controls','course80'):controls[name]['results']=rows
assert total==1062,total
entry=read(w/'controls-entry-final/results.json');assert len(entry)==68 and sum(r['passed'] for r in entry)==38
performance=root/'student.tests/pa18/loop80-performance.json';perf=read(performance)
assert perf.get('finished_utc') and len(perf['workloads'])==16
equivalent=[v['outputs'] for v in perf['workloads'].values() if len(v['outputs'])==2]
assert len(equivalent)==11 and sum(a['sha256']==b['sha256'] for a,b in equivalent)==8
assert sum('native' in a for a,b in equivalent)==10
assert all(a['native']['sha256']==b['native']['sha256'] for a,b in equivalent if 'native' in a)
cc=root/'dev/cppgm++';assert sha(cc)==perf['binaries'][1]['sha256']==progress['compiler_sha256']==sha(w/'final-cppgm')
assert sha(w/'entry-cppgm')==perf['binaries'][0]['sha256']
traces={}
for name in ('signature79','scalar80'):
 source=root/'student.tests/pa18'/(name+'_trace.cpp')
 traces[name]=dict(source=source.read_text(),source_sha256=sha(source),lowir_sha256=sha(v/(name+'.lowir')),native_sha256=sha(v/(name+'.exe')),commands={k:c for k,c in checks.items() if k.startswith(name+'-')})
bounds={}
for family in ('pointer-temporaries','cast-reference','constant-reference'):
 bounds[family]={}
 for n in (600,2400):
  stats=perf['workloads'][family+'-'+str(n)]['outputs'][-1]['telemetry']
  bounds[family][str(n)]={k:x for row in stats for k,x in row.items() if k in ('semantic_type_substitution_work','semantic_type_query_work','semantic_conversion_work','semantic_constant_work','semantic_constant_address_work','semantic_candidate_work','instructions','nodes','ir_capacity_bytes')}
assert (w/'class-ellipsis.log').read_text().strip()=='ERROR: invalid variadic value'
assert all(r['passed'] for r in read(w/'nested-entry/results.json'))
intermediate=read(w/'validation/checks.json');assert intermediate['list75_controls']['exit']==1
result=dict(entry_commit='c5e2c2719f1ef004e6bddb86487e797c37471079',implementation_commits=['cec91d23','a87dd911','ce7d3e7f'],current_commit=subprocess.check_output(['git','rev-parse','HEAD'],text=True).strip(),
 compiler_sha256=sha(cc),checks=checks,stage_progress=progress,personal_control_count=total,controls=controls,entry_controls=entry,trace=traces,work_bounds=bounds,
 performance=dict(path=str(performance),sha256=sha(performance)),intermediate_regression=dict(checks=intermediate,explanation='New user-result reference conversion initially ignored the narrowing consumer requested unrounded target. ce7d3e7f repairs it; final inherited list and all other controls pass.'),
 stale_runtime_marker=dict(source='pa18/tests/general/300-function-template-nested-alias-explicit-call.t',entry=read(w/'nested-entry/results.json'),final=controls['course80']['results'][-1],remaining='Array LowIR comparison remains an original failure.'),
 class_ellipsis=dict(source='student.tests/pa18/class_ellipsis_pending.cpp',compiler_exit=1,diagnostic=(w/'class-ellipsis.log').read_text(),log_sha256=sha(w/'class-ellipsis.log'),status='unfinished ordinary variadic lowering'),
 boundary='Scalar/reference cast selection and materialization completed through ordinary, query, runtime, constant, narrowing and exception consumers. Remaining 25 original failures need array materialization policy, object ABI/emission and body-effect summaries. No reference or comparison changes.',
 independent_review='Review prior source-signature changes and current shared cast selection, bit-field/cv rules, constant storage identity and selected user conversion result/exception facts. Review is not waived by these implementation checks.')
out.write_text(json.dumps(result,indent=2)+'\n');print('Evidence verified: 395/420; prior 2609/2609; file audit; 1062 controls; unchanged fixtures; frozen performance.')
