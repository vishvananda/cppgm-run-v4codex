#!/usr/bin/env python3
"""Validate and retain implementation187 evidence, without generated tool logs.
Usage: analyze187.py SCRATCH
"""
import collections,hashlib,json,pathlib,re,subprocess,sys
root=pathlib.Path(__file__).resolve().parents[2]
scratch=pathlib.Path(sys.argv[1]).resolve()
out=root/'student.tests/pa29/evidence187';out.mkdir(exist_ok=True)
entry='5ef7f89a3bd6be203c6a845432b417bcf518eb07'
implementation=subprocess.check_output(['git','rev-parse','f714397c'],cwd=root,text=True).strip()
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def digest(s):return hashlib.sha256(s.encode()).hexdigest()
def save(name,value):(out/(name+'.json')).write_text(json.dumps(value,indent=2)+'\n')
common=json.loads((scratch/'common-final/performance.json').read_text())
owner=json.loads((scratch/'owner-final/performance.json').read_text())
assert len(common['runs'])==224 and len(owner['runs'])==48 and len(owner['launchers'])==8
assert sha(root/'dev/cppgm++')==common['binaries']['B']['sha256']==owner['binaries']['B']['sha256']
for name,d in [('common',common),('owner',owner)]:
 assert all(v['status']==0 for v in d['runs'])
 for v in d['binaries'].values():assert sha(pathlib.Path(v['path']))==v['sha256']
 save(name+'-performance',d)
 preliminary=json.loads((out/('preliminary-'+name+'-performance.json')).read_text())
 assert preliminary['images']==d['images']
 assert len(preliminary['runs'])==len(d['runs'])
 for v in preliminary['binaries'].values():assert sha(pathlib.Path(v['path']))==v['sha256']
for name,images in common['images'].items():
 assert images['A']==images['B'],name
 for mode in ['compile','runtime']:
  rows=[v for v in common['runs'] if v['workload']==name and v['mode']==mode]
  assert ''.join(v['label'] for v in rows)=='AAAA'+'ABBA'*6
rows=[]
for name,v in owner['inputs'].items():
 assert digest(v['source'])==v['sha256'] and v['entry_rejection']['status']!=0
 n=v['N']
 for x in owner['runs']:
  if x['workload']!=name or x['mode']!='compile':continue
  c={k:v for r in x['phase_counters'] for k,v in r.items()}
  expected=dict(parsed_nodes=24*n+272,nodes=56*n+272,semantic_specializations=n,
   template_body_transitions=n,semantic_constant_bodies=n,semantic_constant_execution_steps=9*n,
   semantic_constant_execution_hits=n,semantic_type_query_work=n+9,semantic_value_query_work=n+1,
   semantic_floating_constants=2*n-2,semantic_signature_work=14,
   instructions=155,operands=238,prepared_instructions=155,prepared_operands=238,
   native_instructions=154,native_functions=2,text_bytes=933)
  assert all(c[k]==v for k,v in expected.items()),(name,expected)
  rows.append(dict(workload=name,trial=x['trial'],asserted_counters=expected))
assert len(rows)==24
assert len({json.dumps(v,sort_keys=True) for v in owner['images'].values()})==1
save('scaling',dict(observations=272,launchers=8,preliminary_observations=272,preliminary_launchers=8,final_images_identical_to_preliminary=True,equivalent_pairs=4,corrected_only_images_identical=True,checks=rows))
checks=json.loads((scratch/'checks/checks.json').read_text());assert len(checks)==99
assert all(x['status']==0 for x in checks[:-36]) and all(x['status']!=0 for x in checks[-36:])
save('controls',dict(commands=[dict(command=x['command'],status=x['status'],expected_success=i<len(checks)-36,
 stdout_sha256=digest(x['stdout']),stderr_sha256=digest(x['stderr'])) for i,x in enumerate(checks)],
 raw_inspection_path=str(scratch/'checks/checks.json'),raw_inspection_sha256=sha(scratch/'checks/checks.json'),
 files={str(p.relative_to(root)):sha(p) for p in sorted((root/'student.tests/pa29/controls187').iterdir())}))
artifacts={str(p.relative_to(scratch)):dict(bytes=p.stat().st_size,sha256=sha(p)) for p in sorted((scratch/'checks').iterdir())
 if p.suffix in ('.lowir','.mir','.canonical','.again')}
mir=(scratch/'checks/adapter.lowir.mir').read_text()
functions=[]
for name,body in re.findall(r'^function (\S+)\n(.*?)(?=^function |\Z)',mir,re.M|re.S):
 functions.append(dict(name=name,abi=body.split('  frame\n')[0].strip().splitlines(),
  stack_bytes=int(re.search(r'stack_size (\d+)',body).group(1)),
  scratch_bytes=int(re.search(r'scratch_bytes (\d+)',body).group(1))))
save('inspection',dict(artifacts=artifacts,adapter_functions=functions,
 complex_symbols=[line.split()[-1] for x in checks if x['command'][0]=='readelf' and x['command'][-1].endswith('/complex.o')
 for line in x['stdout'].splitlines() if '_Z' in line]))
prior=(scratch/'prior-final.log').read_text();stage=(scratch/'stage-final.log').read_text();through=(scratch/'through-final.log').read_text();audit=(scratch/'file-audit-final.log').read_text()
assert 'ALL TESTS PASSED SUCCESSFULLY! (4538 / 4538)' in prior
assert 'TEST SUMMARY: 394 / 403 TESTS PASSED' in stage
assert 'TEST SUMMARY: 4932 / 4941 TESTS PASSED' in through
assert 'File audit passed for pa29 with 4 warning(s).' in audit
failures=re.findall(r'^(pa29/tests/[^:]+): ERROR: (.*)$',stage,re.M)
assert len(failures)==9
assert failures==re.findall(r'^(pa29/tests/[^:]+): ERROR: (.*)$',through,re.M)
old=json.loads((root/'student.tests/pa29/evidence186/remaining.json').read_text())
by_name={v['test']:v for v in old['failures']}
fixed=sorted(set(by_name)-{v[0] for v in failures})
assert fixed==['pa29/tests/compile/600-builtin-complex-return.t','pa29/tests/compile/600-gnu-complex-template-constructor.t']
remaining=[]
for name,diagnostic in failures:
 v=by_name[name];v['diagnostic']=[diagnostic];v['diagnostic_provenance']='Implementation187 final make test-pa29 and root-through report.'
 p=(root/name).with_suffix('.my.stdout')
 if name.startswith('pa29/tests/compile/') and p.exists():v['compiler_diagnostic']=p.read_text()
 remaining.append(v)
save('remaining',dict(total=403,passed=394,failed=9,entry_failures=11,fixed=fixed,owner_counts=dict(collections.Counter(v['owner'] for v in remaining)),failures=remaining))
coverage_path=root/'student.tests/pa29/evidence186/coverage.json';coverage=json.loads(coverage_path.read_text())
assert len(coverage['files'])==1707
for path,d in coverage['files'].items():assert sha(root/path)==d,path
inputs=subprocess.check_output(['git','ls-files','pa29/tests'],cwd=root,text=True).splitlines()
assert sum(x.endswith('.t') for x in inputs)==403
assert not subprocess.check_output(['git','diff',entry,'--','pa29/tests','pa29/scripts','pa29/Makefile','TESTING_AND_REFERENCES.md'],cwd=root)
save('coverage',dict(entry=entry,unchanged=True,total_stage_inputs=403,contract_harness_files=1707,
 predecessor_manifest=str(coverage_path.relative_to(root)),predecessor_manifest_sha256=sha(coverage_path),compared_all_hashes=True))
source_paths=subprocess.check_output(['git','diff','--name-only',entry,implementation,'--','dev'],cwd=root,text=True).splitlines()
source={}
for path in source_paths:
 data=subprocess.check_output(['git','show',implementation+':'+path],cwd=root)
 assert hashlib.sha256(data).hexdigest()==sha(root/path),path
 source[path]=sha(root/path)
assert not subprocess.check_output(['git','diff',implementation,'--','dev'],cwd=root)
save('source-binding',dict(entry=entry,implementation=implementation,binaries=common['binaries'],files=source))
validation=[dict(command=cmd,status=status,passed=passed,total=total,path=str(scratch/path),sha256=sha(scratch/path))
 for cmd,status,passed,total,path in [('make test-report-through-pa28',0,4538,4538,'prior-final.log'),
 ('make test-pa29',2,394,403,'stage-final.log'),('make test-report-through-pa29',2,4932,4941,'through-final.log'),
 ('perl scripts/cppgm_file_audit.pl --stage pa29 --paths dev/src',0,None,None,'file-audit-final.log')]]
save('validation',dict(checks=validation,stageProgress='pass: two original failures fixed; all previous passes preserved; coverage unchanged',file_audit_warnings=audit.splitlines()[1:]))
records=[root/p for p in ['pa29/plan.md','pa29/handoff187.md','pa29/performance187.md','student.tests/pa29/analyze187.py','student.tests/pa29/check187.py','student.tests/pa29/performance187.py']]
save('manifest',dict(entry=entry,implementation=implementation,stage_base='2734e5c67eaa7c0cf4bbbd510dba8d60f36d6543',last_reviewed='2df00585bd10d4e2e068934394dffc8adb0a47ed',
 checks=dict(priorThroughTests='pass',fileAudit='pass',stageProgress='pass',stageTests='394/403; exit 2; nine original failures remain'),
 observations=544,launchers=16,files={str(p.relative_to(root)):sha(p) for p in [*sorted(out.glob('*.json')),*records] if p.name!='manifest.json'}))
print('544 measurements, 24 final scaling checks, 99 control commands, 1707 unchanged contract paths; original failures 11 -> 9')
