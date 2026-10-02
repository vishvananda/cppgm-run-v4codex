#!/usr/bin/env python3
"""Verify the PA30 implementation201 handoff against current files and evidence."""
import hashlib,json,pathlib,re,subprocess
root=pathlib.Path(__file__).resolve().parents[2];e=root/'student.tests/pa30/evidence201'
def read(name):return json.loads((e/(name+'.json')).read_text())
def sha(data):return hashlib.sha256(data).hexdigest()
def git(*args):return subprocess.check_output(['git',*args],cwd=root,text=True).strip()
checks=[]
def check(name,value):
 assert value,name
 checks.append(name)
b=read('source-binding');entry=read('entry');v=read('validation');d=read('stage-delta')
check('code binding',git('rev-parse',b['implementation_commit']+':dev')==b['dev_tree'] and not git('diff',b['implementation_commit'],'--','dev'))
check('compiler binding',sha((root/'dev/cppgm++').read_bytes())==b['compiler_sha256'])
check('entry binding',entry['head']==b['entry_commit'] and entry['compiler_sha256']==b['entry_compiler_sha256'])
for p,h in b['files'].items():check('source '+p,sha((root/p).read_bytes())==h)
correction='pa30/tests/compile/700-hosted-replaceable-operator-new-dynamic-exception-spec.ref.exit_status'
protected=[p for p in git('diff',b['entry_commit'],'--name-only').splitlines() if re.match(r'pa\d+/(tests/|scripts/|Makefile$)|scripts/|Makefile$',p)]
check('only documented reference correction',protected==[correction])
for p,h in entry['fixtures'].items():
 check('fixture '+p,sha((root/p).read_bytes())==(sha(b'EXIT_FAILURE\n') if p==correction else h))
for name,row in v.items():check('report binding '+name,sha(row['output'].encode())==row['sha256'])
check('earlier PAs',v['priorThroughTests']['status']==0 and '4941 / 4941' in v['priorThroughTests']['output'])
check('file audit',v['fileAudit']['status']==0 and 'File audit passed' in v['fileAudit']['output'])
check('stage report',v['stageTests']['status']==2 and '151 / 153' in v['stageTests']['output'])
check('through report',v['through30']['status']==2 and '5092 / 5094' in v['through30']['output'])
check('existing progress',len(d['entry_failures'])==5 and len(d['final_failures'])==2 and len(d['fixed'])==3 and len(d['implementation_fixed'])==2 and not d['regressions'])
check('same coverage',len(d['cases'])==153 and sorted(r['path'] for r in entry['cases'])==sorted(r['path'] for r in d['cases'])==sorted(str(p.relative_to(root)) for p in (root/'pa30/tests/compile').glob('*.t')))
old={r['path']:r['expected'] for r in entry['cases']};new={r['path']:r['expected'] for r in d['cases']}
check('same outcomes except proven mismatch',[p for p in old if old[p]!=new[p]]==[correction.replace('.ref.exit_status','.t')])
for name,count in [('controls',111),('controls200',50),('controls199',88),('controls198',107),('trace',101)]:
 r=read(name);check(name+' binding',r['compiler_sha256']==b['compiler_sha256']);check(name+' count',len(r['commands'])==count)
 for i,row in enumerate(r['commands']):check(name+' command '+str(i),(row['status']!=0)==row.get('expected_rejection',False))
t=read('trace');check('telemetry invariant',all(r['telemetry_identical'] for r in t['images'].values()))
for name,item in t['images'].items():check('trace source '+name,sha((root/f'student.tests/pa30/source201/{name}.cpp').read_bytes())==item['source_sha256'])
for kind,count in [('common',224),('owner',168)]:
 r=read(kind+'-performance')
 check(kind+' binaries',r['binaries']['A']['sha256']==b['entry_compiler_sha256'] and r['binaries']['B']['sha256']==b['compiler_sha256'])
 check(kind+' observations',len(r['runs'])==count and all(v['status']==0 for v in r['runs']))
 check(kind+' flags',r['flags']==['-O0','-c','--stats'])
 for name,images in r['images'].items():
  check(kind+' same executable work '+name,images['A']==images['B'])
  for mode in ['compile','runtime']:
   rows=[v for v in r['runs'] if v['workload']==name and v['mode']==mode]
   check(kind+' AAAA + six ABBA '+name+mode,[''.join(v['label'] for v in rows if v['block']==i) for i in range(7)]==['AAAA']+['ABBA']*6)
 check(kind+' 45 second limit',all(v['wall_s']<45 for v in r['runs'] if v['mode']=='compile'))
o=read('owner-performance')
check('launch calibration',len(o['launchers'])==16 and all(r['status']==0 for r in o['launchers']))
for name,item in o['inputs'].items():check('owner source '+name,sha(item['source'].encode())==item['sha256'])
for counter in ['fallthrough_functions','fallthrough_work','fallthrough_edges']:
 points=[]
 for name,item in o['inputs'].items():
  rows=[r for r in o['runs'] if r['workload']==name and r['mode']=='compile' and r['label']=='B']
  values={next(v[counter] for v in r['phase_counters'] if counter in v) for r in rows}
  check('stable owner '+name+counter,len(values)==1);points.append((item['N'],values.pop()))
 points.sort();(x,y),(xx,yy),(xxx,yyy)=points
 check('affine owner '+counter,y>0 and (yy-y)*(xxx-xx)==(yyy-yy)*(xx-x))
h=read('hosted-performance');check('hosted binary',h['sha256']==b['compiler_sha256'])
check('hosted observations',len(h['runs'])==28 and h['flags']==['-O0','-c','--stats'])
for path,item in h['inputs'].items():
 check('hosted input '+path,sha((root/path).read_bytes())==item['sha256'])
 rows=[r for r in h['runs'] if r['path']==path]
 check('hosted outcomes '+path,len(rows)==4 and all((v['status']!=0)==item['expected_rejection'] and v['wall_s']<45 for v in rows))
 if not item['expected_rejection']:check('hosted deterministic object '+path,len({v['object_sha256'] for v in rows})==1)
plan=(root/'pa30/plan.md').read_text()
check('stage marker','Stage base commit: `27029f978e65b78331233123922d342033d5d1f7`' in plan)
check('review marker','Last reviewed commit: `4a081cb05b25638be7a759882f67d4d8ae97eb6a`' in plan)
(e/'verification.json').write_text(json.dumps(dict(passed=len(checks),checks=checks),indent=2)+'\n')
print(len(checks),'implementation evidence checks passed')
