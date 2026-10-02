#!/usr/bin/env python3
"""Bind the complete checkpoint audit to code, contract, observations and records."""
import hashlib,json,pathlib,posixpath,re,subprocess,sys
root=pathlib.Path(__file__).resolve().parents[2];e=root/'student.tests/pa30/evidence202'
def read(n):return json.loads((e/(n+'.json')).read_text())
def sha(data):return hashlib.sha256(data).hexdigest()
def git(*args):return subprocess.check_output(['git',*args],cwd=root)
checks=[]
def check(name,value):
 assert value,name
 checks.append(name)
b=read('source-binding');entry=read('entry');v=read('validation');d=read('stage-delta');tip=b['reviewed_code_tip']
check('review starts at previous code tip',b['review_start']==entry['review']=='4a081cb05b25638be7a759882f67d4d8ae97eb6a')
check('all handoffs reviewed',b['reviewed_commits']==git('rev-list','--reverse',b['review_start']+'..'+entry['head']).decode().splitlines())
check('reviewed implementation tree',git('rev-parse',tip+':dev').decode().strip()==b['dev_tree'] and not git('diff',tip,'--','dev').strip())
check('compiler',sha((root/'dev/cppgm++').read_bytes())==b['compiler_sha256'])
for label,row in b['frozen_binaries'].items():check('frozen '+label,sha(pathlib.Path(row['path']).read_bytes())==row['sha256'])
for path,h in b['files'].items():check('current source '+path,sha((root/path).read_bytes())==h)
for path,h in entry['fixtures'].items():check('unchanged entry fixture '+path,sha((root/path).read_bytes())==h)
correction='pa30/tests/compile/700-hosted-replaceable-operator-new-dynamic-exception-spec.ref.exit_status'
protected=[p for p in git('diff','--name-only',b['review_start'],'--').decode().splitlines() if re.match(r'pa\d+/(tests/|scripts/|Makefile$)|scripts/|Makefile$',p)]
check('only proved reference correction across review',protected==[correction])
check('no additional reference changes',not git('diff',entry['head'],'--','pa30/tests').strip())
for name,row in v.items():check('report '+name,sha(row['output'].encode())==row['sha256'])
check('prior full report',v['priorThroughTests']['status']==0 and '4941 / 4941' in v['priorThroughTests']['output'])
check('file audit',v['fileAudit']['status']==0 and 'File audit passed' in v['fileAudit']['output'])
check('stage required command',v['stageTests']['command']=='make test-pa30' and v['stageTests']['status']==2 and '151 / 153' in v['stageTests']['output'])
check('through30',v['through30']['status']==2 and '5092 / 5094' in v['through30']['output'])
check('same failures',d['entry_failures']==d['final_failures'] and len(d['final_failures'])==2 and not d['regressions'])
check('same inventory',len(d['cases'])==153 and sorted(r['path'] for r in entry['cases'])==sorted(r['path'] for r in d['cases'])==sorted(str(p.relative_to(root)) for p in (root/'pa30/tests/compile').glob('*.t')))
for name,count in [('controls',35),('controls198',107),('controls199',88),('controls200',50),('controls201',111),('trace',51)]:
 r=read(name);check(name+' binary',r['compiler_sha256']==b['compiler_sha256']);check(name+' count',len(r['commands'])==count)
 for i,row in enumerate(r['commands']):check(name+' outcome '+str(i),(row['status']!=0)==row.get('expected_rejection',False))
t=read('trace')
for name,row in t['images'].items():
 check('trace source '+name,sha((root/f'student.tests/pa30/source202/{name}.cpp').read_bytes())==row['source_sha256'])
 check('telemetry invariant '+name,row['telemetry_identical'])
for kind,count in [('common',224),('owner',168)]:
 r=read(kind+'-performance')
 check(kind+' binaries',r['binaries']['A']['sha256']==b['frozen_binaries']['review']['sha256'] and r['binaries']['B']['sha256']==b['compiler_sha256'])
 check(kind+' flags',r['flags']==['-O0','-c','--stats'])
 check(kind+' observations',len(r['runs'])==count and all(row['status']==0 for row in r['runs']))
 for name,images in r['images'].items():
  check(kind+' equivalent emitted bytes '+name,images['A']==images['B'])
  for mode in ['compile','runtime']:
   rows=[row for row in r['runs'] if row['workload']==name and row['mode']==mode]
   check(kind+' AAAA/six ABBA '+name+mode,[''.join(row['label'] for row in rows if row['block']==block) for block in range(7)]==['AAAA']+['ABBA']*6)
 check(kind+' mandated compile limit',all(row['wall_s']<45 for row in r['runs'] if row['mode']=='compile'))
owner=read('owner-performance')
for counter in ['fallthrough_functions','fallthrough_work','fallthrough_edges']:
 points=[]
 for name,row in owner['inputs'].items():
  check('owner source '+name,sha(row['source'].encode())==row['sha256'])
  samples=[r for r in owner['runs'] if r['workload']==name and r['mode']=='compile' and r['label']=='B']
  values={next(v[counter] for v in r['phase_counters'] if counter in v) for r in samples}
  check('stable '+name+counter,len(values)==1);points.append((row['N'],values.pop()))
 points.sort();(x,y),(xx,yy),(xxx,yyy)=points
 check('linear flow work '+counter,y>0 and (yy-y)*(xxx-xx)==(yyy-yy)*(xx-x))
a=read('allocation-performance')
check('allocation binding',a['binaries']['A']['sha256']==entry['compiler_sha256'] and a['binaries']['B']['sha256']==b['compiler_sha256'])
check('allocation measurements',len(a['runs'])==48 and all(r['status']==0 and (r['mode']!='compile' or r['wall_s']<45) for r in a['runs']))
for name,row in a['inputs'].items():
 check('allocation source '+name,sha(row['source'].encode())==row['sha256'])
 check('allocation non-equivalent entry '+name,row['entry_rejection']['status']!=0)
for r in [owner,a]:check('launcher calibration '+r['binaries']['A']['sha256'],len(r['launchers'])==16 and all(x['status']==0 for x in r['launchers']))
h=read('hosted-performance');check('hosted binding',h['sha256']==b['compiler_sha256'])
check('hosted observations',len(h['runs'])==80 and h['flags']==['-O0','-c','--stats'])
for path,row in h['inputs'].items():
 check('hosted source '+path,sha((root/path).read_bytes())==row['sha256'])
 rows=[r for r in h['runs'] if r['path']==path]
 check('hosted outcome '+path,len(rows)==4 and all((r['status']!=0)==row['expected_rejection'] and r['wall_s']<45 for r in rows))
 if not row['expected_rejection']:check('hosted deterministic '+path,len({r['object_sha256'] for r in rows})==1)
ab=read('hosted-ab-performance')
check('hosted AB binaries',ab['binaries']['A']['sha256']==entry['compiler_sha256'] and ab['binaries']['B']['sha256']==b['compiler_sha256'])
check('hosted AB samples',len(ab['runs'])==56 and all(r['status']==0 and r['wall_s']<45 for r in ab['runs']))
check('hosted AB flags',ab['flags']==['-O0','-c','--stats'])
for name,row in ab['inputs'].items():
 check('hosted AB source '+name,sha((root/row['path']).read_bytes())==row['sha256'])
 check('hosted AB same image '+name,ab['images'][name]['A']==ab['images'][name]['B'])
 samples=[r for r in ab['runs'] if r['workload']==name]
 check('hosted AB protocol '+name,[''.join(r['label'] for r in samples if r['block']==i) for i in range(7)]==['AAAA']+['ABBA']*6)
# Every historical source binding is checked at its own handoff, including
# controls/scripts added after its code commit. Wrappers are real symlinks.
for n,rev in [(199,'377d92a0'),(200,'b0790726'),(201,'56ecc31c')]:
 directory=root/f'student.tests/pa30/evidence{n}';old=json.loads((directory/'source-binding.json').read_text())
 check('historical code tree '+str(n),git('rev-parse',old['implementation_commit']+':dev').decode().strip()==old['dev_tree'])
 tree={}
 for line in git('ls-tree','-r',rev).decode().splitlines():
  meta,path=line.split('\t');tree[path]=meta.split()
 for path,h in old['files'].items():
  resolved=path
  while tree[resolved][0]=='120000':resolved=posixpath.normpath(posixpath.join(posixpath.dirname(resolved),git('show',rev+':'+resolved).decode()))
  check('historical source '+str(n)+' '+path,sha(git('show',rev+':'+resolved))==h)
 for p in directory.glob('*-performance.json'):
  r=json.loads(p.read_text());check('historical observations '+str(p),all(x['status']==0 or x.get('path') in r.get('inputs',{}) and r['inputs'][x['path']].get('expected_rejection') for x in r['runs']))
images=read('historical-images')
check('all historical traced images',len(images)==27)
artifacts=pathlib.Path(b['frozen_binaries']['entry']['path']).parent
for row in images:
 n=row['checkpoint'];name=row['name'];path=artifacts/f'controls-final{n}'/((str(n)+'-' if n==199 else '')+name+'.o')
 old=json.loads((root/f'student.tests/pa30/evidence{n}/trace.json').read_text())['images'][name]
 check('historical generated bytes '+str(n)+name,sha(path.read_bytes())==row['current_sha256']==row['old_sha256']==old.get('direct_object_sha256',old.get('object_sha256')))
check('optimization trace binding',read('optimization-trace')['compiler_sha256']==b['compiler_sha256'])
for i,r in enumerate(read('optimization-trace')['commands']):check('optimization command '+str(i),r['status']==0)
if '--pre-records' not in sys.argv:
 for path in ['pa30/plan.md','pa30/audit.md']:
  s=(root/path).read_text();check('recorded tip '+path,'Last reviewed commit: `'+tip+'`' in s)
  check('stage base '+path,'Stage base commit: `27029f978e65b78331233123922d342033d5d1f7`' in s)
(e/'verification.json').write_text(json.dumps(dict(passed=len(checks),checks=checks),indent=2)+'\n')
print(len(checks),'audit evidence checks passed')
