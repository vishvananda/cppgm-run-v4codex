#!/usr/bin/env python3
"""Freeze initialization validation, live source identities and unchanged coverage."""
from pathlib import Path
import hashlib,json,os,subprocess,sys
ROOT=Path(__file__).resolve().parents[2]
A,B,SAN,WORK,OUT=map(lambda p:Path(p).resolve(),sys.argv[1:6]);WORK.mkdir(parents=True,exist_ok=True)
def sha(p):return hashlib.sha256(Path(p).read_bytes()).hexdigest()
def binary(p):return dict(path=str(p),sha256=sha(p))
result=dict(entry_commit='2ab55111',harness_sha256=sha(__file__),binaries=[binary(p) for p in (A,B,SAN)],checks=[],sources=[],coverage=[])
env=dict(os.environ,ASAN_OPTIONS='detect_leaks=1:halt_on_error=1',UBSAN_OPTIONS='halt_on_error=1:print_stacktrace=1')
def check(name,command):
 p=subprocess.run(list(map(str,command)),cwd=ROOT,env=env,capture_output=True,text=True,timeout=600)
 log=WORK/(name+'.log');log.write_text(p.stdout+p.stderr)
 result['checks'].append(dict(name=name,command=list(map(str,command)),exit_code=p.returncode,log=str(log),log_sha256=sha(log)))
 OUT.write_text(json.dumps(result,indent=2)+'\n');assert p.returncode==0,(name,p.returncode,p.stderr)
 print(name,'PASS',flush=True)
assert sha(ROOT/'dev/cppgm++')==sha(B)
check('through',['make','test-report-through-pa14'])
check('file-audit',['perl','scripts/cppgm_file_audit.pl','--stage','pa14','--paths','dev/src'])
check('sanitizer-parity',[sys.executable,ROOT/'student.tests/pa14/check_sanitizers.py',B,SAN])
check('entry-parity',[sys.executable,ROOT/'student.tests/pa14/check_audit_parity.py',A,B,WORK/'entry-parity'])
check('all-native',[sys.executable,ROOT/'student.tests/pa14/check_functions.py'])
for label,compiler in [('release',B),('sanitized',SAN)]:
 objects=ROOT/'obj/dev' if label=='release' else WORK.parent.parent/'pa14-asan-obj/dev'
 for name in ['check_default_facts','check_default_demand_states']:
  check(label+'-'+name,[sys.executable,ROOT/('student.tests/pa14/'+name+'.py'),WORK/(name+'-'+label),objects,'current' if label=='release' else 'sanitized'])
 check(label+'-default-demands',[sys.executable,ROOT/'student.tests/pa14/check_default_demands.py',compiler,WORK/('defaults-'+label)])
 check(label+'-lifecycle-properties',[sys.executable,ROOT/'student.tests/pa14/check_lifecycle_properties.py',compiler,WORK/('properties-'+label),objects,label])
 check(label+'-lifecycle-actions',[sys.executable,ROOT/'student.tests/pa14/check_lifecycle_actions.py',WORK/('actions-'+label),objects,label])
 check(label+'-lifecycle-definitions',[sys.executable,ROOT/'student.tests/pa14/check_lifecycle_definitions.py',compiler,WORK/('definitions-'+label)])
 check(label+'-virtual-demands',[sys.executable,ROOT/'student.tests/pa14/check_virtual_demands.py',WORK/('virtual-'+label),objects,'current' if label=='release' else 'sanitized'])
 check(label+'-initializers',[sys.executable,ROOT/'student.tests/pa14/check_initialization_facts.py',compiler,WORK/('initializers-'+label)])
 check(label+'-statements',[sys.executable,ROOT/'student.tests/pa14/check_statement_facts.py',compiler,WORK/('statements-'+label)])
 check(label+'-body-publication',[sys.executable,ROOT/'student.tests/pa14/check_body_publication.py',WORK/('bodies-'+label),objects,'current' if label=='release' else 'sanitized'])
 check(label+'-prior-demands',[sys.executable,ROOT/'student.tests/pa14/check_demand_failures.py',WORK/('demands-'+label),objects,'current' if label=='release' else 'sanitized'])
 for name in ['bindings','queries','fixed_expressions','fixed_calls','fixed_objects','dependent_objects','declaration_types','demand_regions','body_values','value_queries','object_reducers','definition_demands','special_signatures','signature_publications']:
  check(label+'-'+name,[sys.executable,ROOT/f'student.tests/pa14/check_{name}.py',compiler])
 for name in ['check_native','check_virtual_semantics','check_ir','check_linkage','audit_check','check_literal_storage']:
  check(label+'-pa13-'+name,[sys.executable,ROOT/f'student.tests/pa13/{name}.py',compiler])
for p in sorted((ROOT/'dev/src').rglob('*')):
 if p.suffix in ('.cpp','.h'):result['sources'].append(binary(p))
for p in [ROOT/'dev/frontend_source_sets.mk',ROOT/'dev/cppgm++.cpp',ROOT/'spec.md',ROOT/'pa14/README.md',ROOT/'scripts/compare_results_common.pl']:
 result['sources'].append(binary(p))
for p in [ROOT/'student.tests/pa14/check_initialization_facts.py',ROOT/'student.tests/pa14/body_compare.pl',ROOT/'student.tests/pa14/body_compare.py']:
 result['sources'].append(binary(p))
prior=json.loads((ROOT/'student.tests/pa14/default-proofs.json').read_text())
for row in prior['coverage']:
 p=ROOT/row['path'];assert sha(p)==row['sha256'];result['coverage'].append(binary(p))
assert sha(ROOT/'dev/cppgm++')==sha(B)
OUT.write_text(json.dumps(result,indent=2)+'\n')
