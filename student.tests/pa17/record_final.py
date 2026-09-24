#!/usr/bin/env python3
"""Bind completed final-audit observations; does not substitute for execution."""
from pathlib import Path
import hashlib, json, os, subprocess
ROOT=Path(__file__).resolve().parents[2]
WORK=Path(os.environ['RALPH_ARTIFACT_DIR'])/'loop62'
OUT=ROOT/'student.tests/pa17'
def read(name):return json.loads((OUT/name).read_text())
def git(*args):return subprocess.check_output(['git',*args],cwd=ROOT,text=True).strip()
def sha(p):return hashlib.sha256(Path(p).read_bytes()).hexdigest()
def bound(p):
 p=Path(p);return dict(path=str(p.relative_to(ROOT)) if p.is_relative_to(ROOT) else str(p),sha256=sha(p))
def save(name,data):(OUT/name).write_text(json.dumps(data,indent=2)+'\n')
controls=dict(inherited=json.loads((WORK/'inherited-final.json').read_text()),
 closures=json.loads((WORK/'closures-final/results.json').read_text()),
 regions=json.loads((WORK/'regions-final/results.json').read_text()),
 structure=json.loads((WORK/'regions-final/structure.json').read_text()),
 abi=json.loads((WORK/'abi-checked/results.json').read_text()),
 audit=json.loads((WORK/'final-current/results.json').read_text()),
 audit_entry=json.loads((WORK/'final-entry/results.json').read_text()))
save('final-controls.json',controls)
e=dict(entry=git('rev-parse','0970f3f0'),implementation_tip=git('rev-parse','12140852'),implementation_tree=git('rev-parse','12140852:dev'),
 compiler_sha256=sha(WORK/'final-cppgm++'),stage=dict(passed=343,total=343,through_passed=2609,through_total=2609,stages=17),
 course_trees=read('handoff61-evidence.json')['course_trees'],checks=[],artifacts=[],historical=[],
 remaining_defects=[],unaudited_handoffs=[],reference_changes=[],
 performance_acceptance='PA17/O0 stage-scoped acceptance; no mandated numerical ceiling. Preserve historical diagnostics, semantic costs, work/growth bounds and later-stage ownership.',
 advisory_warnings=['lowering/procedural.h header division','semantic/analyzer.h header division','semantic/model.h header division'],
 corrected_invocation=dict(log=bound(WORK/'abi-final.log'),explanation='Passed CLI abimangle to API harness; rebuilt intended API harness and reran successfully.'))
for command,filename,required in [
 ('make test-pa17','pa17.log','(343 / 343)'),
 ('make test-report-through-pa17','exit-through-pa17.log','(2609 / 2609)'),
 ('perl scripts/cppgm_file_audit.pl --stage pa17 --paths dev/src','exit-file-audit.log','File audit passed for pa17'),
 ('python3 student.tests/pa17/checkpoint60_replay.py dev/cppgm++ WORK/inherited-final WORK/inherited-final.json','inherited-final.log','checkpoint60 43 / 43'),
 ('python3 student.tests/pa17/closure_controls.py dev/cppgm++ WORK/closures-final','closures-final.log','auto_deleted_member_address PASS'),
 ('python3 student.tests/pa17/region_controls.py dev/cppgm++ WORK/regions-final','regions-final.log',"'conditional_no_live_prefix': True"),
 ('python3 student.tests/pa17/final_controls.py dev/cppgm++ WORK/final-current','final-current.log','parameter_size PASS'),
 ('python3 student.tests/pa17/closure_abi.py dev/cppgm++ WORK/check-abi WORK/abi-checked','abi-checked.log','source closure numbering/native check pass'),
 ('python3 student.tests/pa9/check.py --api WORK/check-abi && python3 student.tests/pa9/check_final.py --api WORK/check-abi','pa9-controls.log','8 exact/roundtrip probes, 11 controlled rejections pass'),
 ('python3 student.tests/pa17/storage_reference_corrections.py','references.log','Six storage references follow'),
]:
 path=WORK/filename;text=path.read_text();assert required in text
 e['checks'].append(dict(command=command.replace('WORK',str(WORK)),exit_code=0,**bound(path),log_text=text,required_text=required))
for filename in ['final-controls.json','final-range.json','final-trace.cpp','final-trace.json','final_controls.py','final_benchmark.py','final-performance.json','final-stage-performance.json','verify_final.py','record_final.py']:
 e['artifacts'].append(bound(OUT/filename))
for filename in ['spec.md','pa17/README.md','pa17/plan.md','pa17/audit.md','pa17/final-performance.md','pa17/plan-loop61.md','pa17/audit-loop60.md']:
 e['artifacts'].append(bound(ROOT/filename))
# Preserve every pre-audit stage artifact; only the two compact current records
# move to named historical copies. Check bytes, not merely a prior assertion.
paths=git('ls-tree','-r','--name-only',e['entry'],'--','pa17','student.tests/pa17').splitlines()
for path in paths:
 if '/tests/' in path or '/scripts/' in path:continue
 if path in ('pa17/plan.md','pa17/audit.md'):continue
 data=subprocess.check_output(['git','show',e['entry']+':'+path],cwd=ROOT)
 if (ROOT/path).is_symlink():
  assert data.decode()==os.readlink(ROOT/path),path
  e['historical'].append(dict(path=path,symlink_target=data.decode(),sha256=hashlib.sha256(data).hexdigest()))
  continue
 assert hashlib.sha256(data).hexdigest()==sha(ROOT/path),path
 e['historical'].append(bound(ROOT/path))
for old,new in [('pa17/plan.md','pa17/plan-loop61.md'),('pa17/audit.md','pa17/audit-loop60.md')]:
 data=subprocess.check_output(['git','show',e['entry']+':'+old],cwd=ROOT)
 assert hashlib.sha256(data).hexdigest()==sha(ROOT/new)
save('final-evidence.json',e)
print('Bound final audit artifacts, completed checks and unchanged historical evidence.')
