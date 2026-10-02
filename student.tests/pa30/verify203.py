#!/usr/bin/env python3
"""Bind final PA30 checks and performance evidence to unchanged code/fixtures."""
import hashlib,json,pathlib,subprocess
root=pathlib.Path(__file__).resolve().parents[2];out=root/'student.tests/pa30/evidence203';checks=[]
def read(name):return json.loads((out/(name+'.json')).read_text())
def sha(path):return hashlib.sha256(path.read_bytes()).hexdigest()
def check(value,name):
 assert value,name
 checks.append(name)
binding=read('source-binding');entry=read('entry');final=binding['binaries']['cppgm++']
for kind in ['sources','objects','controls']:
 for path,digest in binding[kind].items():check(sha(root/path)==digest,kind+' '+path)
for name,digest in binding['binaries'].items():check(sha(root/'dev'/name)==digest,'binary '+name)
for name,item in binding['frozen'].items():check(sha(pathlib.Path(item['path']))==item['sha256'],'frozen '+name)
check(binding['frozen']['entry']['sha256']==entry['compiler_sha256'],'entry binary identity')
check(binding['frozen']['final']['sha256']==final,'final binary identity')
changes=subprocess.check_output(['git','diff','--name-only',entry['head'],'--'],cwd=root,text=True).splitlines()
check(all(p.startswith(('dev/','pa30/','student.tests/')) for p in changes),'change scope')
check(not any(p.startswith('pa30/') and p not in ['pa30/plan.md','pa30/design203.md','pa30/performance203.md'] for p in changes),'handouts fixtures and references unchanged')
check(not subprocess.check_output(['git','diff',binding['code_commit'],'--','dev'],cwd=root),'code matches frozen commit')
for row in entry['cases']:check(sha(root/row['path'])==row['source_sha256'],'fixture '+row['path'])
validation=read('validation')
for key,total in [('priorThroughTests',4941),('stageTests',153),('through30',5094)]:
 record=validation[key];check(record['status']==0 and f'({total} / {total})' in record['output'],'required '+key)
check(validation['fileAudit']['status']==0,'required fileAudit')
delta=read('stage-delta')
check(len(delta['cases'])==153 and not delta['final_failures'] and not delta['regressions'] and len(delta['fixed'])==2,'stageProgress fixes original failures')
check({r['path'] for r in delta['cases']}=={r['path'] for r in entry['cases']},'unchanged stage coverage')
for n,count in [(198,107),(199,88),(200,50),(201,111),(202,35),(203,48)]:
 data=read('controls'+str(n));check(data['compiler_sha256']==final,'control compiler '+str(n));check(len(data['commands'])==count,'control count '+str(n))
 for i,row in enumerate(data['commands']):check((row['status']!=0)==row['expected_rejection'],f'control {n}/{i}')
data=read('differential');check(data['compiler_sha256']==final and len(data['cases'])==286,'differential compiler/count')
for row in data['cases']:
 check(sha(root/row['source'])==row['sha256'],'differential source '+row['source'])
 check(row['equal'] and len(row['commands'])==8 and all(c['status']==0 for c in row['commands']),'differential output '+row['source'])
trace=read('trace');check(trace['compiler_sha256']==final and len(trace['commands'])==95,'trace compiler/count')
for i,row in enumerate(trace['commands']):check((row['status']!=0)==row['expected_rejection'],f'trace {i}')
check(len(trace['images'])==7 and all(r['telemetry_identical'] for r in trace['images'].values()),'trace coverage and telemetry')
inspection=read('inspection')
check(inspection['compiler_sha256']==final and len(inspection['commands'])==42,'inspection compiler/count')
check(all(r['status']==0 for r in inspection['commands']) and len(inspection['images'])==7 and all(all(v.values()) for v in inspection['images'].values()),'reparsed text symbols and CFI identical')
observations=0
for filename,count in [('common-performance',224),('hosted-ab-performance',56),('vector-performance',216),('hosted-new-performance',16)]:
 data=read(filename);check(data['binaries']['A']['sha256']==entry['compiler_sha256'] and data['binaries']['B']['sha256']==final,'performance frozen '+filename)
 check(len(data['runs'])==count and all(r['status']==0 for r in data['runs']),'performance observations '+filename);observations+=count
 check(data['affinity']==['taskset','-c','2'] and data['flags']==['-O0','-c','--stats'],'performance policy '+filename)
 for r in data['runs']:
  if r.get('mode','compile')=='compile':check(r['wall_s']<45 and r['peak_rss_kib']>0,'compile budget '+filename+' '+str(len(checks)))
 if filename in ['common-performance','hosted-ab-performance']:
  for name,images in data['images'].items():check(images['A']['object_sha256']==images['B']['object_sha256'],'unchanged image '+filename+' '+name)
 if filename=='vector-performance':
  check(len(data['launchers'])==16 and len(data['inputs'])==6,'vector calibration and sizes')
  for name,item in data['inputs'].items():
   check(hashlib.sha256(item['source'].encode()).hexdigest()==item['sha256'],'vector frozen input '+name)
   if name.startswith('simd'):check(item['entry_rejection']['status']!=0,'new SIMD has no valid baseline '+name)
  for r in data['runs']:
   if r['mode']!='compile' or r['label']!='B':continue
   name=r['workload'];n=data['inputs'][name]['N'];c={k:v for phase in r['phase_counters'] for k,v in phase.items()};simd=name.startswith('simd')
   check(c['semantic_specializations']==n and c['template_body_transitions']==n,'one specialization/body per family '+name+' '+str(len(checks)))
   check(c['instructions']==(284 if simd else 40)*n+53 and c['native_instructions']==(367 if simd else 41)*n+72,'linear consumed/emitted work '+name+' '+str(len(checks)))
plan=(root/'pa30/plan.md').read_text()
check('Stage base commit: `27029f978e65b78331233123922d342033d5d1f7`' in plan,'stage base preserved')
check('Last reviewed commit: `378d1bd83df4dd3f7a9c1693c18afa7e7c7bd61f`' in plan,'review marker preserved')
result=dict(passed=len(checks),performance_observations=observations,checks=checks)
(out/'verification.json').write_text(json.dumps(result,indent=2)+'\n')
print(len(checks),'evidence checks passed;',observations,'performance observations')
