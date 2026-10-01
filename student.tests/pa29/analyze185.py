#!/usr/bin/env python3
"""Retain observations and assert exact demand/emission scaling and coverage."""
import hashlib,json,pathlib,shutil,subprocess
root=pathlib.Path(__file__).resolve().parents[2];out=root/'student.tests/pa29/evidence185';scratch=pathlib.Path('/tmp/pa29-185')
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def save(n,v):(out/(n+'.json')).write_text(json.dumps(v,indent=2)+'\n')
for src,name in [('entry-controls/controls.json','entry-controls'),('controls/controls.json','preliminary-controls'),('controls-boundaries/controls.json','boundary-controls'),('controls-deduction/controls.json','deduction-controls'),('controls-extended/controls.json','preliminary-storage-controls'),('controls-storage/controls.json','controls'),('inspection-final/inspection.json','inspection'),('common/performance.json','common-performance'),('owner/performance.json','owner-performance')]:shutil.copyfile(scratch/src,out/(name+'.json'))
c=json.loads((out/'common-performance.json').read_text());o=json.loads((out/'owner-performance.json').read_text())
assert len(c['runs'])==224 and len(o['runs'])==48 and len(o['launchers'])==8
assert all(v['A']['object_sha256']==v['B']['object_sha256'] for v in c['images'].values())
assert len({v['object_sha256'] for v in o['images'].values()})==1
rows=[]
for row in o['runs']:
 if row['mode']!='compile':continue
 n=o['inputs'][row['workload']]['N'];facts={k:v for p in row['phase_counters'] for k,v in p.items()}
 expected=dict(semantic_specializations=n,template_body_transitions=n,semantic_constant_execution_hits=n,semantic_constant_execution_steps=8*n,semantic_type_substitution_work=2*n+2,semantic_type_substitution_records=2*n,semantic_substitution_frames=n,semantic_signature_work=266,parsed_nodes=21*n+246,instructions=83,operands=139,prepared_instructions=83,prepared_operands=139,native_instructions=272)
 assert all(facts[k]==v for k,v in expected.items()),(n,{k:(v,facts[k]) for k,v in expected.items() if facts[k]!=v})
 rows.append(dict(N=n,trial=row['trial'],facts=expected))
save('scaling',dict(observations=len(rows),checks=rows,object_identity=True,text_bytes=1698))
binary=sha(root/'dev/cppgm++');assert binary==c['binaries']['B']['sha256']==o['binaries']['B']['sha256']
for name in ['controls','inspection','validation']:
 data=json.loads((out/(name+'.json')).read_text());assert data['compiler_sha256']==binary
controls=json.loads((out/'controls.json').read_text());assert len(controls['cases'])==31 and not any('failure' in r for r in controls['cases'])
inspection=json.loads((out/'inspection.json').read_text());assert len(inspection['rows'])==169 and not any(r['status'] for r in inspection['rows'])
validated=json.loads((out/'validated-source.json').read_text());assert all(sha(root/p)==v for p,v in validated['files'].items())
entry=json.loads((out/'entry.json').read_text());assert all(sha(root/p)==v for p,v in entry['coverage'].items())
files={str(p.relative_to(root)):sha(p) for p in sorted((root/'student.tests/pa29').glob('*185*')) if p.is_file()}
for directory in ['source185','support185']:
 files.update({str(p.relative_to(root)):sha(p) for p in sorted((root/'student.tests/pa29'/directory).rglob('*')) if p.is_file()})
files.update({str(p.relative_to(root)):sha(p) for p in sorted(out.glob('*.json')) if p.name!='manifest.json'})
save('manifest',dict(entry=entry['head'],implementation_commits=['8ee20450','27012cf5'],binaries=c['binaries'],files=files,performance_observations=272,launchers=8,controls=31,inspection_commands=169))
print('31 controls; 169 inspections; 272 performance observations; 24 scaling checks; source/coverage identities pass')
