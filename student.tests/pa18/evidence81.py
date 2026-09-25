#!/usr/bin/env python3
"""Verify loop 81's complete implementation handoff evidence: WORK OUT."""
from pathlib import Path
import hashlib,json,subprocess,sys
root=Path(__file__).resolve().parents[2];work=Path(sys.argv[1]).resolve();out=Path(sys.argv[2]);v=work/'validation'
def sha(p):return hashlib.sha256(Path(p).read_bytes()).hexdigest()
def read(p):return json.loads(Path(p).read_text())
checks=read(v/'checks.json');progress=read(v/'stage-progress.json')
assert checks['stage']['exit']==2 and all(c['exit']==0 for n,c in checks.items() if n!='stage')
assert len(progress['entry_failures'])==25 and len(progress['final_failures'])==24 and not progress['new_failures']
assert progress['inputs']==420 and progress['files']==1686
assert all(sha(root/p)==h for p,h in progress['fixture_sha256'].items())
assert '2609 / 2609' in (v/'prior.log').read_text() and '396 / 420' in (v/'stage.log').read_text()
controls={};total=0
for name,c in read(v/'validate73_controls/results.json').items():
 if 'count' not in c:continue
 p=v/'validate73_controls'/name/'results.json';rows=read(p);assert all(r['passed'] for r in rows)
 controls[name]=dict(count=len(rows),path=str(p),sha256=sha(p));total+=len(rows)
for name in ('audit74_controls','list75_controls','inherit76_controls','nested77_controls','lookup77_controls','audit78_controls','audit78_declarations','syntax79_controls','signature79_controls','course79','scalar80_controls','result81_controls','course81'):
 p=v/name/'results.json';rows=read(p);assert all(r['passed'] for r in rows)
 controls[name]=dict(count=len(rows),path=str(p),sha256=sha(p));total+=len(rows)
 if name in ('result81_controls','course81'):controls[name]['results']=rows
entry=read(work/'controls-entry-final/results.json');assert len(entry)==74
perfpath=root/'student.tests/pa18/loop81-performance.json';perf=read(perfpath)
assert perf.get('finished_utc') and len(perf['workloads'])==15
cc=root/'dev/cppgm++';assert sha(cc)==sha(work/'final-cppgm')==perf['binaries'][1]['sha256']==progress['compiler_sha256']
assert sha(work/'entry-cppgm')==perf['binaries'][0]['sha256']
traces={}
for name in ('signature79','scalar80','result81'):
 source=root/'student.tests/pa18'/(name+'_trace.cpp')
 traces[name]=dict(source=source.read_text(),source_sha256=sha(source),lowir_sha256=sha(v/(name+'.lowir')),native_sha256=sha(v/(name+'.exe')),commands={k:c for k,c in checks.items() if k.startswith(name+'-')})
inspection=read(v/'result81_inspection/results.json');assert len(inspection)==10 and all(r['native_exit']==0 for r in inspection)
for family in ('runtime-named-result','runtime-known-branch'):
 w=perf['workloads'][family];a,b=w['outputs']
 assert b['native']['payload_bytes']<a['native']['payload_bytes']
 assert w['runtime']['median_b_over_a']<1
plan=(root/'pa18/plan.md').read_text()
assert 'Stage base commit: `94dcb8ad21664137e87d574e878c14a4a047348a`.' in plan
assert 'Last reviewed commit: `82fca940b1849d90deffbaba29ee162946f3e23c`.' in plan
result=dict(entry_commit='6924787714fac6bdeb0b3df29df49bb7cbe5fa7e',implementation_commits=['7b8c98a6','4ab09a9b'],current_commit=subprocess.check_output(['git','rev-parse','HEAD'],text=True).strip(),compiler_sha256=sha(cc),
 checks=checks,stage_progress=progress,personal_control_count=total,controls=controls,entry_controls=entry,inspection=inspection,traces=traces,
 performance=dict(path=str(perfpath),sha256=sha(perfpath)),intermediate_performance=dict(path='student.tests/pa18/loop81-performance-before-demand.json',sha256=sha(root/'student.tests/pa18/loop81-performance-before-demand.json'),explanation='An initial 1.6% paired compiler overhead on explicit-only conversions motivated request-driven summaries. Final explicit-only LowIR and native bytes match entry; all intermediate measurements and checks are preserved.'),
 boundary='Completed named scalar return summaries through implicit conversions, conditional values/lifetimes and retained explicit/address function uses. Qualified conversion-address lookup and canonical class-alias member pointer ownership are repaired. Remaining 24 course failures require array materialization, class-result ABI/emission and unrelated scalar representation owners. No reference or comparison changes.',
 independent_review='Prior handoffs 79/80 plus summary proof eligibility and validity, selected-branch cleanup, emission roots/address retention and qualified conversion lookup. These review obligations remain distinct from known unfinished implementation.')
out.write_text(json.dumps(result,indent=2)+'\n');print('Evidence verified: 396/420, prior 2609/2609, file audit,',total,'controls, 10 inspection programs, unchanged fixtures and frozen performance.')
