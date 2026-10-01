#!/usr/bin/env python3
"""Verify the implementation handoff against current source and frozen evidence."""
import json,pathlib,hashlib,subprocess,re
root=pathlib.Path(__file__).resolve().parents[2];out=root/'student.tests/pa29/evidence184'
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def read(n):return json.loads((out/(n+'.json')).read_text())
def git(*args):return subprocess.check_output(['git',*args],cwd=root,text=True)
validated=read('validated-source')
assert all(sha(root/p)==v for p,v in validated['files'].items())
assert validated['compiler_sha256']==sha(root/'dev/cppgm++')
assert not git('diff','52c00f4d','--','dev')
assert not git('diff','68ee6f8d','--','pa29/tests','pa29/scripts','pa29/Makefile','TESTING_AND_REFERENCES.md')
entry_plan=git('show','68ee6f8d:pa29/plan.md');plan=(root/'pa29/plan.md').read_text();markers={}
for key in ['Stage base commit','Last reviewed commit','Previous reviewed commit','Audit entry']:
 value=re.search(re.escape(key)+r': `([^`]+)`',entry_plan).group(1)
 assert f'{key}: `{value}`' in plan
 markers[key]=value
controls=read('controls');inspection=read('inspection');validation=read('validation');delta=read('stage-delta');remaining=read('remaining')
assert len(controls['cases'])==42 and not any('failure' in c for c in controls['cases'])
assert len(inspection['rows'])==194 and all(r['status']==0 for r in inspection['rows'])
assert all(c['compiler_sha256']==validated['compiler_sha256'] for c in [controls,inspection,validation])
assert [c['status'] for c in validation['checks']]==[2,0,2,0]
assert delta['entry_failures']==14 and delta['final_failures']==13 and delta['new_failures']==[]
assert sum('Unfinished' in f['disposition'] for f in remaining['failures'])==10
assert sum('Independent' in f['disposition'] for f in remaining['failures'])==3
for check in validation['checks']:assert sha(pathlib.Path(check['log']))==check['sha256']
for c in controls['cases']:assert sha(root/c['source'])==c['sha256']
for c in inspection['cases']:assert sha(root/c['source'])==c['source_sha256']
for p,v in read('coverage')['files'].items():assert sha(root/p)==v
binaries=read('common-performance')['binaries']
assert binaries==read('owner-performance')['binaries']
for v in binaries.values():assert sha(pathlib.Path(v['path']))==v['sha256']
assert binaries['B']['sha256']==validated['compiler_sha256']
assert len(read('common-performance')['runs'])+len(read('owner-performance')['runs'])==328
assert len(read('scaling')['observations'])==24
paths=git('ls-files','--cached','--others','--exclude-standard','student.tests/pa29/source184','student.tests/pa29/test184.py','student.tests/pa29/inspect184.py','student.tests/pa29/inspection184-facts.cpp','student.tests/pa29/validate184.py','student.tests/pa29/performance184.py','student.tests/pa29/analyze184.py','student.tests/pa29/handoff184.py').splitlines()
manifest=dict(entry_commit=git('rev-parse','68ee6f8d').strip(),implementation_commit=git('rev-parse','52c00f4d').strip(),previous_turn_classification='progress: committed guide/default implementation and validated 15 to 14 failure reduction, revalidated by entry state',review_markers=markers,binaries=binaries,compiler_sources=len(validated['files']),stage_inputs=403,coverage_paths=len(read('coverage')['files']),checks=dict(priorThroughTests='pass',fileAudit='pass',stageProgress='pass',stageTests='incomplete: 390/403'),controls=42,inspection_commands=194,performance_observations=328,launchers=8,scaling_checks=24,personal_sources_and_harnesses={p:sha(root/p) for p in paths},evidence={p.name:sha(p) for p in sorted(out.glob('*.json')) if p.name!='manifest.json'},notes=['The preliminary rejection expectation for a scalar conversion to const reference was incorrect; reference-temporaries.cpp now verifies static-initialization temporaries. Course coverage is unchanged.','The first lifetime control observed a temporary before its full expression ended; it was corrected to observe after the boundary. Both compilers agreed before/after; no implementation change was needed.','Preliminary inspection found only constructor section ordering differences for virtual-base; final comparison compares complete named sections and symbols with no omitted section.','All preliminary observations are preserved, as are prior handoffs and performance evidence.','Implementation group complete; independent cumulative review and whole-stage work remain outstanding.'])
(out/'manifest.json').write_text(json.dumps(manifest,indent=2)+'\n')
print('Completion evidence checked:',manifest['compiler_sources'],'source files,',manifest['coverage_paths'],'coverage paths; review markers preserved')
