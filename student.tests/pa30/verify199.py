#!/usr/bin/env python3
"""Verify implementation handoff bindings, progress and stage-scoped evidence."""
import hashlib,json,pathlib,re,subprocess
root=pathlib.Path(__file__).resolve().parents[2];e=root/'student.tests/pa30/evidence199'
def read(name):return json.loads((e/(name+'.json')).read_text())
def sha(data):return hashlib.sha256(data).hexdigest()
def git(*args):return subprocess.check_output(['git',*args],cwd=root).decode().strip()
checks=[]
def check(name,value):
 assert value,name
 checks.append(name)
b=read('source-binding');entry=read('entry');delta=read('stage-delta');v=read('validation')
check('current code tree',git('rev-parse',b['implementation_commit']+':dev')==b['dev_tree'] and not git('diff',b['implementation_commit'],'--','dev'))
check('current compiler',sha((root/'dev/cppgm++').read_bytes())==b['compiler_sha256'])
for p,h in b['files'].items():check('source '+p,sha((root/p).read_bytes())==h)
protected=[p for p in git('diff',b['entry_commit'],'--name-only').splitlines() if re.match(r'pa\d+/(tests/|scripts/|Makefile$)|scripts/|Makefile$',p)]
check('no fixture/harness/reference changes',not protected)
for name,row in v.items():check('log hash '+name,sha(row['output'].encode())==row['sha256'])
check('earlier PAs',v['priorThroughTests']['status']==0 and '4941 / 4941' in v['priorThroughTests']['output'])
check('file audit',v['fileAudit']['status']==0 and 'File audit passed' in v['fileAudit']['output'])
check('PA30 partial report',v['stageTests']['status']==2 and '138 / 153' in v['stageTests']['output'])
check('through30 partial report',v['through30']['status']==2 and '5079 / 5094' in v['through30']['output'])
check('strict existing progress',len(delta['entry_failures'])==21 and len(delta['final_failures'])==15 and len(delta['fixed'])==6 and not delta['regressions'])
check('same coverage',len(delta['cases'])==153 and sorted(r['path'] for r in entry['cases'])==sorted(r['path'] for r in delta['cases'])==sorted(str(p.relative_to(root)) for p in (root/'pa30/tests/compile').glob('*.t')))
check('same expectations',{r['path']:r['expected'] for r in entry['cases']}=={r['path']:r['expected'] for r in delta['cases']})
for name,count in [('controls',88),('controls198',107),('trace',93)]:
 r=read(name);check(name+' compiler',r['compiler_sha256']==b['compiler_sha256']);check(name+' command count',len(r['commands'])==count)
 for i,row in enumerate(r['commands']):check(name+' command '+str(i),(row['status']!=0)==row.get('expected_rejection',False))
t=read('trace')
check('telemetry invariance',all(r['telemetry_identical'] for r in t['images'].values()))
check('volatile whole operands',t['views']['vector-volatile']['lowir'].count('load volatile i32')==4)
check('representation MIR trace',bool(t['views']['vector-representation']['mir']))
common=read('common-performance')
check('common binaries',common['binaries']['A']['sha256']==b['entry_compiler_sha256'] and common['binaries']['B']['sha256']==b['compiler_sha256'])
check('common observations',len(common['runs'])==224 and all(r['status']==0 for r in common['runs']))
for name,images in common['images'].items():
 check('equivalent common output '+name,images['A']==images['B'])
 for mode in ['compile','runtime']:
  rows=[r for r in common['runs'] if r['workload']==name and r['mode']==mode]
  check('AAAA + six ABBA '+name+mode,[''.join(r['label'] for r in rows if r['block']==i) for i in range(7)]==['AAAA']+['ABBA']*6)
owner=read('owner-performance')
check('owner binary',owner['binaries']['B']['sha256']==b['compiler_sha256'])
check('owner observations',len(owner['runs'])==48 and len(owner['launchers'])==16 and all(r['status']==0 for r in owner['runs']+owner['launchers']))
for name,item in owner['inputs'].items():
 check('frozen owner input '+name,sha(item['source'].encode())==item['sha256'])
 check('invalid entry not timing baseline '+name,item['entry_rejection']['status']!=0)
 n=item['N']
 for row in [r for r in owner['runs'] if r['workload']==name and r['mode']=='compile']:
  c=row['phase_counters'][0]
  check('linear owner counters '+name+str(row['trial']),c['semantic_lookup_work']==179*n+88 and c['semantic_type_query_work']==12*n+3 and c['template_class_completions']==n and c['delimiter_work']==311*n+170 and c['max_pending']==47)
  check('deterministic owner object '+name+str(row['trial']),row['object_sha256']==owner['images'][name]['object_sha256'])
vector=read('vector-performance')
check('vector binary',vector['binaries']['B']['sha256']==b['compiler_sha256'])
check('vector observations',len(vector['runs'])==16 and all(r['status']==0 for r in vector['runs']))
check('vector frozen checked input',sha(vector['input']['source'].encode())==vector['input']['sha256'] and vector['input']['entry_rejection']['status']!=0)
hosted=read('hosted-performance')
check('hosted binary',hosted['sha256']==b['compiler_sha256'])
check('all six repaired fixtures',sorted(hosted['inputs'])==sorted(delta['fixed']) and len(hosted['runs'])==24)
for path,h in hosted['inputs'].items():
 check('hosted input '+path,sha((root/path).read_bytes())==h)
 rows=[r for r in hosted['runs'] if r['path']==path]
 check('hosted repeated output '+path,len(rows)==4 and len({r['object_sha256'] for r in rows})==1 and all(r['status']==0 for r in rows))
for name in ['common','owner','vector','hosted']:
 r=read(name+'-performance')
 check('unchanged flags '+name,r['flags']==['-O0','-c','--stats'])
 check('45-second bound '+name,all(v['wall_s']<45 and v['status']==0 for v in r['runs'] if v.get('mode','compile')=='compile'))
plan=(root/'pa30/plan.md').read_text()
check('stage marker preserved','Stage base commit: `27029f978e65b78331233123922d342033d5d1f7`' in plan)
check('review marker preserved','Last reviewed commit: `4a081cb05b25638be7a759882f67d4d8ae97eb6a`' in plan)
(e/'verification.json').write_text(json.dumps(dict(passed=len(checks),checks=checks),indent=2)+'\n')
print(len(checks),'implementation evidence checks passed')
