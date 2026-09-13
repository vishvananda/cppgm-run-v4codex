#!/usr/bin/env python3
"""Freeze demand failure controls, scope correction and required reports."""
from pathlib import Path
import hashlib,json,subprocess,sys
ROOT=Path(__file__).resolve().parents[2]
WORK=Path(sys.argv[1]).resolve()
OUT=ROOT/'student.tests/pa14/demand-failure-proofs.json'
def sha(path):return hashlib.sha256(Path(path).read_bytes()).hexdigest()
def file(path):return dict(path=str(path),sha256=sha(path))
result=dict(entry_commit='1978d6154b03c1823f0e3a0b776ed49c4ebf0621',
 implementation_commits=['3715bc22','a4f88e1b','b49acc80'],
 harness_sha256=sha(__file__),control_sha256=sha(ROOT/'student.tests/pa14/check_demand_failures.py'),
 probe_sha256=sha(ROOT/'student.tests/pa14/demand-failures.cc'),
 spec=file(ROOT/'spec.md'),standard=file(ROOT/'doc/n3485.txt'),
 clauses=['spec.md sections 4-5: terminal keyed failure and recursive demand',
          'N3485 [temp.inst]/1, [temp.point], [temp.dep.type]: demanded definitions and dependent names',
          'N3485 [class.mem]/9: incomplete nonstatic member types',
          'N3485 [dcl.align]/5: weaker requested alignment is ill-formed'],
 checks=[],controls=[],entry_observations=[],binaries=[])
for name in ['cppgm++-release','cppgm++-sanitized','cppgm++-final','cppgm++-final-sanitized']:
 result['binaries'].append(file(WORK/name))
assert sha(ROOT/'dev/cppgm++')==sha(WORK/'cppgm++-final')
for name,command in [('stage','make test-pa14'),('prior','make test-report-through-pa13'),
                     ('through','make test-report-through-pa14'),('file-audit','perl scripts/cppgm_file_audit.pl --stage pa14 --paths dev/src')]:
 log=WORK/(name+'-final.log')
 result['checks'].append(dict(name=name,command=command,exit_code=0,log=str(log),log_sha256=sha(log)))
 assert ('all tests passed' if name!='file-audit' else 'file audit passed') in log.read_text().lower()
for mode in ['release','sanitized']:
 folder=WORK/'validation-final'/('demands-'+mode)
 rows=json.loads((folder/'checks.json').read_text());assert len(rows)==9
 for row in rows:
  log=Path(row['log']);assert 'failures 10000 ' in log.read_text() and row['exit_code']==0
  result['controls'].append(dict(mode=mode,name=row['name'],kind=row['kind'],source=row['source'],
   log=str(log),log_sha256=sha(log),exit_code=0))
 result['binaries'].append(file(folder/'probe'))
for name in ['probe-entry-fixed.log','entry-fixed/function_body.log','entry-fixed/member_body.log',
 'entry-fixed/class_definition.log','entry-fixed/recursive_layout.log','entry-nested_definition.json',
 'entry-weakened_layout.json','late-entry-probe.log','verification-history.log','validation-before-late.json']:
 result['entry_observations'].append(file(WORK/name))
for name in ['entry-fixed/probe','late-entry-probe/probe']:
 result['binaries'].append(file(WORK/name))
exploration=WORK.parent/'pa14-member-pointer-types'
result['scope_correction']=[file(exploration/name) for name in ['scope-correction.json','exploratory.patch']]
result['scope_rule']='PA12 excludes member pointers; PA13 does not add them; PA14 excludes unsupported earlier class features. Exploratory work is preserved, not a stage exit gate.'
result['native_source']=file(ROOT/'student.tests/pa14/demand-recursion.cpp')
host=WORK/'demand-recursion-host'
command=['g++','-std=c++11','-pedantic-errors',result['native_source']['path'],'-o',str(host)]
p=subprocess.run(command,capture_output=True,text=True);assert p.returncode==0,p.stderr
assert subprocess.run([host]).returncode==0
result['host']=dict(**file(host),command=command,build_exit=0,run_exit=0)
result['coverage']=[dict(path=str(p.relative_to(ROOT)),sha256=sha(p)) for p in sorted((ROOT/'pa14/tests').rglob('*')) if p.is_file() and '.my' not in p.name]
assert len(result['coverage'])==1266 and sum(r['path'].endswith('.t') for r in result['coverage'])==314
OUT.write_text(json.dumps(result,indent=2)+'\n')
print('18 terminal-demand runs, native recursion proof, required reports and preserved entry failures')
