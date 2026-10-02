#!/usr/bin/env python3
"""Verify the implementation200 handoff against bound sources and raw evidence."""
import hashlib,json,pathlib,re,subprocess
root=pathlib.Path(__file__).resolve().parents[2];e=root/'student.tests/pa30/evidence200'
def read(name):return json.loads((e/(name+'.json')).read_text())
def sha(data):return hashlib.sha256(data).hexdigest()
def git(*args):return subprocess.check_output(['git',*args],cwd=root,text=True).strip()
checks=[]
def check(name,value):
 assert value,name
 checks.append(name)
b=read('source-binding');entry=read('entry');d=read('stage-delta');v=read('validation')
check('code binding',git('rev-parse',b['implementation_commit']+':dev')==b['dev_tree'] and not git('diff',b['implementation_commit'],'--','dev'))
check('compiler binding',sha((root/'dev/cppgm++').read_bytes())==b['compiler_sha256'])
check('entry binding',entry['head']==b['entry_commit'] and entry['compiler_sha256']==b['entry_compiler_sha256'])
for p,h in b['files'].items():check('source '+p,sha((root/p).read_bytes())==h)
protected=[p for p in git('diff',b['entry_commit'],'--name-only').splitlines() if re.match(r'pa\d+/(tests/|scripts/|Makefile$)|scripts/|Makefile$',p)]
check('no fixture/harness/reference changes',not protected)
for p,h in entry['fixtures'].items():check('entry fixture '+p,sha((root/p).read_bytes())==h)
for name,row in v.items():check('report binding '+name,sha(row['output'].encode())==row['sha256'])
check('earlier PAs',v['priorThroughTests']['status']==0 and '4941 / 4941' in v['priorThroughTests']['output'])
check('file audit',v['fileAudit']['status']==0 and 'File audit passed' in v['fileAudit']['output'])
check('stage report',v['stageTests']['status']==2 and '148 / 153' in v['stageTests']['output'])
check('through report',v['through30']['status']==2 and '5089 / 5094' in v['through30']['output'])
check('existing progress',len(d['entry_failures'])==15 and len(d['final_failures'])==5 and len(d['fixed'])==10 and not d['regressions'])
check('same coverage',len(d['cases'])==153 and sorted(r['path'] for r in entry['cases'])==sorted(r['path'] for r in d['cases'])==sorted(str(p.relative_to(root)) for p in (root/'pa30/tests/compile').glob('*.t')))
check('same expected outcomes',{r['path']:r['expected'] for r in entry['cases']}=={r['path']:r['expected'] for r in d['cases']})
for name,count in [('controls',50),('controls199',88),('controls198',107),('trace',81)]:
 r=read(name);check(name+' binding',r['compiler_sha256']==b['compiler_sha256']);check(name+' count',len(r['commands'])==count)
 for i,row in enumerate(r['commands']):check(name+' command '+str(i),(row['status']!=0)==row.get('expected_rejection',False))
t=read('trace');check('telemetry invariant',all(r['telemetry_identical'] for r in t['images'].values()))
for name,image in t['images'].items():check('trace input '+name,sha((root/f'student.tests/pa30/source200/{name}.cpp').read_bytes())==image['source_sha256'])
c=read('common-performance')
check('common binaries',c['binaries']['A']['sha256']==b['entry_compiler_sha256'] and c['binaries']['B']['sha256']==b['compiler_sha256'])
check('common observations',len(c['runs'])==224 and all(r['status']==0 for r in c['runs']))
for name,images in c['images'].items():
 check('equivalent common images '+name,images['A']==images['B'])
 for mode in ['compile','runtime']:
  rows=[r for r in c['runs'] if r['workload']==name and r['mode']==mode]
  check('AAAA + six ABBA '+name+mode,[''.join(r['label'] for r in rows if r['block']==i) for i in range(7)]==['AAAA']+['ABBA']*6)
o=read('owner-performance')
check('owner binaries',o['binaries']['A']['sha256']==b['entry_compiler_sha256'] and o['binaries']['B']['sha256']==b['compiler_sha256'])
check('owner observations',len(o['runs'])==48 and len(o['launchers'])==16 and all(r['status']==0 for r in o['runs']+o['launchers']))
for name,item in o['inputs'].items():
 check('owner input '+name,sha(item['source'].encode())==item['sha256'])
 check('entry rejection not a timing baseline '+name,item['entry_rejection']['status']!=0)
 for row in [r for r in o['runs'] if r['workload']==name and r['mode']=='compile']:
  check('owner deterministic object '+name+str(row['trial']),row['object_sha256']==o['images'][name]['object_sha256'])
# Work counters should grow affinely with independent source families, rather
# than retrying earlier specializations when another family is declared.
for counter in ['semantic_lookup_work','semantic_type_query_work','template_class_completions','delimiter_work']:
 points=[]
 for name,item in o['inputs'].items():
  rows=[r for r in o['runs'] if r['workload']==name and r['mode']=='compile']
  values={r['phase_counters'][0][counter] for r in rows};check('stable counter '+name+counter,len(values)==1)
  points.append((item['N'],values.pop()))
 points.sort();(x,y),(xx,yy),(xxx,yyy)=points
 check('affine owner work '+counter,(yy-y)*(xxx-xx)==(yyy-yy)*(xx-x))
h=read('hosted-performance');check('hosted binary',h['sha256']==b['compiler_sha256'])
check('all repaired fixtures',sorted(h['inputs'])==sorted(d['fixed']) and len(h['runs'])==40)
for path,digest in h['inputs'].items():
 check('hosted input '+path,sha((root/path).read_bytes())==digest)
 rows=[r for r in h['runs'] if r['path']==path]
 check('hosted repeated output '+path,len(rows)==4 and len({r['object_sha256'] for r in rows})==1 and all(r['status']==0 for r in rows))
for name in ['common','owner','hosted']:
 r=read(name+'-performance');check('flags '+name,r['flags']==['-O0','-c','--stats'])
 check('45-second limit '+name,all(v['wall_s']<45 and v['status']==0 for v in r['runs'] if v.get('mode','compile')=='compile'))
plan=(root/'pa30/plan.md').read_text()
check('stage marker','Stage base commit: `27029f978e65b78331233123922d342033d5d1f7`' in plan)
check('review marker','Last reviewed commit: `4a081cb05b25638be7a759882f67d4d8ae97eb6a`' in plan)
(e/'verification.json').write_text(json.dumps(dict(passed=len(checks),checks=checks),indent=2)+'\n')
print(len(checks),'implementation evidence checks passed')
