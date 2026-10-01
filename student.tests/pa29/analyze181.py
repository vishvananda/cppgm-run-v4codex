#!/usr/bin/env python3
"""Bind final checks, frozen performance, personal controls and source evidence."""
import hashlib,json,pathlib,shutil,subprocess,sys,time
root=pathlib.Path(__file__).resolve().parents[2];scratch=pathlib.Path(sys.argv[1]).resolve();out=pathlib.Path(sys.argv[2]).resolve()
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def save(name,data):(out/(name+'.json')).write_text(json.dumps(data,indent=2)+'\n')
for source,target in [('controls-final/controls.json','controls.json'),('inspection-final/inspection.json','inspection.json'),('common/performance.json','common-performance.json'),('owner/performance.json','owner-performance.json')]:shutil.copyfile(scratch/source,out/target)
controls=json.loads((out/'controls.json').read_text());assert len(controls['cases'])==43 and not any('failure' in c for c in controls['cases'])
inspection=json.loads((out/'inspection.json').read_text());assert len(inspection['rows'])==166
entry=json.loads((out/'entry.json').read_text());validation=json.loads((out/'validation.json').read_text())
source=json.loads((out/'validated-source.json').read_text());assert source['compiler_sha256']==sha(root/'dev/cppgm++')==sha(scratch/'cppgm-final')
assert controls['compiler_sha256']==inspection['compiler_sha256']==source['compiler_sha256']
for p,h in source['files'].items():assert sha(root/p)==h,p
performance={n:json.loads((out/(n+'-performance.json')).read_text()) for n in ['common','owner']}
for data in performance.values():
 assert data['binaries']['A']['sha256']==entry['compiler_sha256']==sha(scratch/'cppgm-entry')
 assert data['binaries']['B']['sha256']==source['compiler_sha256']
 assert all(not r['status'] for r in data['runs'])
summary={n:data['summary'] for n,data in performance.items()};save('performance-summary',summary)
identity={}
for suite,data in performance.items():
 for name,images in data['images'].items():
  if 'A' in images:identity[suite+'/'+name]={key:images['A'][key]==images['B'][key] for key in ['object_sha256','executable_sha256']}
save('image-equivalence',identity)
if not (out/'launchers.json').exists():
 launch=[]
 for i in range(8):
  start=time.perf_counter();p=subprocess.run(['/usr/bin/time','-f','%M','-o',str(scratch/'launcher-rss'),'/bin/true']);wall=time.perf_counter()-start
  assert not p.returncode;launch.append(dict(wall_s=wall,peak_rss_kib=int((scratch/'launcher-rss').read_text()),status=p.returncode))
 save('launchers',launch)
# Retain final owner counters in a compact scaling table as well as raw runs.
scaling={}
for row in performance['owner']['runs']:
 if row['mode']!='compile' or row['label']!='B':continue
 inp=performance['owner']['inputs'][row['workload']];n=inp['N'];width=inp['width']
 semantic,lowering,native=row['phase_counters']
 assert semantic['parsed_nodes']==15*n+381 and semantic['template_occurrences']==95*n
 assert semantic['semantic_type_substitution_work']==n and semantic['semantic_type_substitution_hits']==3*n
 assert semantic['semantic_substitution_frames']==2*n and semantic['template_body_transitions']==n
 assert lowering['nodes']==110*n+381 and lowering['instructions']==((25*n+84) if width else (21*n+80))
 assert native['text_bytes']==((98*n+19) if width else (86*n+11))
for name in performance['owner']['inputs']:
 row=next(r for r in performance['owner']['runs'] if r['workload']==name and r['mode']=='compile' and r['label']=='B')
 scaling[name]=dict(input=performance['owner']['inputs'][name],counters=row['phase_counters'])
save('scaling',scaling)
manifest=dict(entry=entry['entry'],last_reviewed=entry['previous_review'],code_tip=validation['code_tip'],compiler_sha256=source['compiler_sha256'],controls=len(controls['cases']),inspection_commands=len(inspection['rows']),performance_observations={n:len(d['runs']) for n,d in performance.items()},preliminary_observations={n:len(json.loads((out/('preliminary-'+n+'-performance.json')).read_text())['runs']) for n in ['common','owner']},artifacts={p.name:sha(p) for p in sorted(out.glob('*.json')) if p.name!='manifest.json'},scripts={str(p.relative_to(root)):sha(p) for p in [root/'student.tests/pa29'/name for name in ['test181.py','inspect181.py','performance181.py','validate181.py','analyze181.py']]+[root/'student.tests/pa27/performance147_common.py']},reproduction=[['python3','student.tests/pa29/test181.py','OUT/controls-final','B'],['python3','student.tests/pa29/inspect181.py','OUT/inspection-final'],['python3','student.tests/pa29/validate181.py','student.tests/pa29/evidence181'],['python3','student.tests/pa27/performance147_common.py','OUT/common','A','B'],['python3','student.tests/pa29/performance181.py','OUT/owner','A','B'],['python3','student.tests/pa29/analyze181.py','OUT','student.tests/pa29/evidence181']])
save('manifest',manifest)
print('source, checks, 43 controls, 166 inspections and',sum(len(d['runs']) for d in performance.values()),'performance observations bound')
