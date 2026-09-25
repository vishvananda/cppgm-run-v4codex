#!/usr/bin/env python3
"""Verify checkpoint audit evidence and bind it to the committed code: WORK OUT."""
from pathlib import Path
import hashlib,json,re,subprocess,sys
root=Path(__file__).resolve().parents[2];work=Path(sys.argv[1]).resolve();out=Path(sys.argv[2]);v=work/'validation'
def sha(p):return hashlib.sha256(Path(p).read_bytes()).hexdigest()
def read(p):return json.loads(Path(p).read_text())
def git(*args):return subprocess.check_output(['git',*args],cwd=root)
base='82fca940b1849d90deffbaba29ee162946f3e23c'
entry=git('rev-parse','85ea42c0').decode().strip();tip=git('rev-parse','HEAD').decode().strip()
assert not git('diff','HEAD','--','dev'),'Code must be frozen.'
checks=read(v/'checks.json');progress=read(v/'stage-progress.json')
assert checks['stage']['exit']==2 and all(c['exit']==0 for n,c in checks.items() if n!='stage')
assert len(progress['entry_failures'])==24 and len(progress['final_failures'])<=24 and not progress['new_failures']
assert progress['inputs']==420 and progress['files']==1686
assert all(sha(root/p)==h for p,h in progress['fixture_sha256'].items())
assert '2609 / 2609' in (v/'prior.log').read_text() and '396 / 420' in (v/'stage.log').read_text()
for c in checks.values():assert sha(c['path'])==c['sha256']
controls={};total=0
for name,c in read(v/'validate73_controls/results.json').items():
 if 'count' not in c:continue
 p=v/'validate73_controls'/name/'results.json';rows=read(p);assert all(r['passed'] for r in rows)
 controls[name]=dict(count=len(rows),path=str(p),sha256=sha(p));total+=len(rows)
for name in ('audit74_controls','list75_controls','inherit76_controls','nested77_controls','lookup77_controls','audit78_controls','audit78_declarations','syntax79_controls','signature79_controls','course79','scalar80_controls','result81_controls','audit82_controls','course81'):
 p=v/name/'results.json';rows=read(p);assert all(r['passed'] for r in rows)
 controls[name]=dict(count=len(rows),path=str(p),sha256=sha(p));total+=len(rows)
 if name=='audit82_controls':controls[name]['results']=rows
assert total==1137+controls['audit82_controls']['count']
perfpath=root/'student.tests/pa18/loop82-performance.json';perf=read(perfpath)
assert perf.get('finished_utc') and perf['commits']==[base,tip]
assert sha(root/'dev/cppgm++')==sha(work/'final-cppgm')==perf['binaries'][1]['sha256']==progress['compiler_sha256']
assert sha(perf['binaries'][0]['path'])==perf['binaries'][0]['sha256']==read(root/'student.tests/pa18/loop79-performance.json')['binaries'][0]['sha256']
for name,item in perf['workloads'].items():
 assert item['compiler']['observations'] and len(item['outputs'])==(1 if item['comparison']=='new-behavior' else 2)
 assert all(row['checked_exit']==0 for row in item['compiler']['observations'])
 if 'runtime' in item:assert all(row['checked_exit']==0 for row in item['runtime']['observations'])
 for output in item['outputs']:
  if 'native' in output:assert output['native']['checked_exit']==0
for family in ('runtime-named-result','runtime-known-branch'):
 w=perf['workloads'][family];a,b=w['outputs']
 assert b['native']['payload_bytes']<a['native']['payload_bytes']
 assert len(w['runtime']['paired_b_over_a'])==4
traces={}
for name in ('signature79','scalar80','result81','audit82'):
 source=root/'student.tests/pa18'/(name+'_trace.cpp')
 traces[name]=dict(source=source.read_text(),source_sha256=sha(source),lowir_sha256=sha(v/(name+'.lowir')),native_sha256=sha(v/(name+'.exe')),commands={k:c for k,c in checks.items() if k.startswith(name+'-')})
inspection=read(v/'result81_inspection/results.json');assert len(inspection)==10 and all(r['native_exit']==0 for r in inspection)
correction='pa18/tests/general/300-function-template-result-first-lookup.ref.exit_status'
assert git('diff','--name-only',base,tip,'--','pa18/tests').decode().splitlines()==[correction]
assert git('diff','--name-only',entry,tip,'--','pa18/tests').decode()==''
reference=read(work/'reference.json');assert [r['exit'] for r in reference['observations']]==[0,1,0,1]
commits=[]
for rev in git('rev-list','--reverse',base+'..'+tip).decode().splitlines():
 commits.append(dict(commit=rev,subject=git('show','-s','--format=%s',rev).decode().strip(),patch_sha256=hashlib.sha256(git('show','--format=fuller',rev)).hexdigest(),paths=git('diff-tree','--no-commit-id','--name-only','-r',rev).decode().splitlines()))
sources={p:sha(root/p) for p in git('diff','--name-only',base,tip,'--','dev').decode().splitlines()}
result=dict(stage_base_commit='94dcb8ad21664137e87d574e878c14a4a047348a',previous_last_reviewed_commit=base,entry_commit=entry,last_reviewed_commit=tip,
 commits=commits,combined_implementation_diff_sha256=hashlib.sha256(git('diff',base,tip,'--','dev')).hexdigest(),implementation_sha256=sources,
 compiler_sha256=sha(root/'dev/cppgm++'),checks=checks,stage_progress=progress,personal_control_count=total,controls=controls,
 entry_controls=read(work/'controls-entry-complete/results.json'),inspection=inspection,traces=traces,
 reference_correction=dict(proof='pa18/reference-correction79.md',proof_sha256=sha(root/'pa18/reference-correction79.md'),observations=reference),
 performance=dict(path=str(perfpath),sha256=sha(perfpath)),
 findings=['Shared conversion-name lookup for ordinary expressions, template binding and queries; indexed retained targets and exact candidate sets.',
 'Source member queries consume fixed base edges, remap naming scope even for a fixed callee, and project target-selected conversion specializations by canonical template/arguments.',
 'Parser-owned angle binding scratch eliminates allocation per angle probe; capacity follows nesting depth.'],
 exploratory_boundary='Two initial explicit throw/catch probes reach the inherited unsupported-statement boundary (PA21); their original log is retained at /tmp/pa18-loop82/controls-entry.log. They are not PA18 acceptance controls. All 1137 inherited controls and all 420 course inputs remain.',
 disposition='Checkpoint audit; full PA18 remains incomplete with the unchanged 24 entry failures. No course coverage/comparison changes in this audit.')
out.write_text(json.dumps(result,indent=2)+'\n');print('Audit evidence verified:',total,'controls; PA18 396/420, prior 2609/2609, file audit, frozen cumulative performance, unchanged course coverage.')
