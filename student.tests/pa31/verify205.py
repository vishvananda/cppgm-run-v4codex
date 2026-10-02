#!/usr/bin/env python3
"""Check final source/binary/coverage/evidence bindings without rerunning workloads."""
import hashlib,json,pathlib,subprocess
root=pathlib.Path(__file__).resolve().parents[2]
e=root/'student.tests/pa31/evidence205'
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
binding=json.loads((e/'binding.json').read_text());checks=0
for path,digest in binding['sources'].items():
 assert sha(root/path)==digest,path;checks+=1
assert sha(root/'dev/cppgm++')==binding['binaries']['B']['sha256'];checks+=1
for path,digest in binding['records'].items():
 assert sha(root/path)==digest,path;checks+=1
coverage=json.loads((e/'coverage.json').read_text())
for path,item in coverage['files'].items():
 p=root/path;assert sha(p)==item['current_sha256'],path
 original=subprocess.check_output(['git','show',binding['stage_base']+':'+path],cwd=root)
 assert hashlib.sha256(original).hexdigest()==item['base_sha256'],path
 expected=original.replace(b'_Z1g',b'g') if path in coverage['corrected_sidecars'] else original
 assert p.read_bytes()==expected,path;checks+=1
assert len(list((root/'pa31/tests/link').glob('*.t')))==84
for record in ['controls.json','trace.json']:
 r=json.loads((e/record).read_text());assert r['passed'] and r['compiler_sha256']==binding['binaries']['B']['sha256'];checks+=1
for record in ['common-performance.json','owner-performance.json','hosted-performance.json']:
 r=json.loads((e/record).read_text())
 assert all(r['binaries'][k]['sha256']==binding['binaries'][k]['sha256'] for k in 'AB')
 assert all(sha(pathlib.Path(r['binaries'][k]['path']))==r['binaries'][k]['sha256'] for k in 'AB')
 directory=pathlib.Path(binding['artifact_root'])/record.removesuffix('.json')
 for name,images in r['images'].items():
  for label,image in images.items():
   assert sha(directory/(name+label+'.o'))==image['object_sha256']
   assert sha(directory/(name+label))==image['executable_sha256']
   checks+=2
 assert all(x['status']==0 for x in r['runs']);checks+=1
for record in ['priorThroughTests','stageTests','fileAudit','through31']:
 assert binding['validation'][record]['exit_code']==0;checks+=1
assert binding['validation']['stageTests']['passed']==84
result=dict(passed=True,checks=checks,implementation_commit=binding['implementation_commit'])
(e/'verification.json').write_text(json.dumps(result,indent=2)+'\n')
print(checks,'source, fixture, binary and evidence bindings pass')
