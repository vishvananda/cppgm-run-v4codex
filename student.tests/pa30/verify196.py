#!/usr/bin/env python3
"""Verify handoff evidence binding, coverage, measurements and required checks."""
import hashlib,json,pathlib,statistics,subprocess
root=pathlib.Path(__file__).resolve().parents[2]
out=root/'student.tests/pa30/evidence196';checks=[]
def check(name,condition):
 assert condition,name
 checks.append(name)
def read(name):return json.loads((out/name).read_text())
def sha(p):return hashlib.sha256(pathlib.Path(p).read_bytes()).hexdigest()
entry=read('entry.json');delta=read('stage-delta.json');binding=read('source-binding.json')
check('all course fixture/reference/harness bytes preserved',all(sha(root/p)==h for p,h in entry['coverage'].items()))
check('all tested implementation bytes preserved',all(sha(root/p)==h for p,h in binding['sources'].items()))
check('all control and benchmark inputs preserved',all(sha(root/p)==h for p,h in binding['personal'].items()))
check('entry and final compilers preserved',all(sha(p)==h for p,h in binding['binaries'].items()))
prior={r['path']:r for r in entry['cases']};after={r['path']:r for r in delta['cases']}
check('all 153 original cases retained',len(prior)==153 and set(prior)==set(after))
check('all expected statuses retained',all(r['expected']==prior[p]['expected'] for p,r in after.items()))
fixed=[p for p,r in after.items() if r['actual']==r['expected'] and prior[p]['actual']!=prior[p]['expected']]
regressed=[p for p,r in after.items() if r['actual']!=r['expected'] and prior[p]['actual']==prior[p]['expected']]
check('15 existing failures fixed without regressions',fixed==delta['fixed'] and len(fixed)==15 and not regressed and not delta['regressed'])
check('entry 114 and final 129 successes',sum(r['actual']==r['expected'] for r in prior.values())==114 and sum(r['actual']==r['expected'] for r in after.values())==129)
validation=read('validation.json')
check('earlier stages pass',validation['prior']['status']==0 and '4941 / 4941' in validation['prior']['tail'])
check('file audit passes',validation['audit']['status']==0 and 'File audit passed' in validation['audit']['tail'])
check('required PA30 test preserves incomplete result',validation['stage']['status']==2 and '129 / 153' in validation['stage']['tail'])
check('combined through30 agrees',validation['through30']['status']==2 and '5070 / 5094' in validation['through30']['tail'])
check('stage progress gate satisfied',validation['stageProgress']['pass'] and validation['stageProgress']['failures_before']==39 and validation['stageProgress']['failures_after']==24)
controls=read('controls.json');common=read('common-performance.json');owner=read('owner-performance.json');hosted=read('hosted-performance.json')
check('controls use measured compiler',controls['sha256']==common['binaries']['B']['sha256'])
check('all 28 controls meet expected outcomes',len(controls['commands'])==28 and all((r['status']!=0) if r['expected_rejection'] else (r['status']==0) for r in controls['commands']))
for group,data in [('common',common),('owner',owner)]:
 check(group+' binary identity',all(sha(v['path'])==v['sha256'] for v in data['binaries'].values()))
 check(group+' all measurement results correct',all(r['status']==0 for r in data['runs']))
 for name,img in data['images'].items():
  if group=='common':
   check(name+' equivalent object and executable bytes',img['A']==img['B'])
   for mode in ['compile','runtime']:
    rows=[r for r in data['runs'] if r['workload']==name and r['mode']==mode]
    check(name+' '+mode+' AA/ABBA orders',[''.join(r['label'] for r in rows if r['block']==b) for b in range(7)]==['AAAA']+['ABBA']*6)
    ratios=[statistics.mean(r['wall_s'] for r in rows if r['block']==b and r['label']=='B')/statistics.mean(r['wall_s'] for r in rows if r['block']==b and r['label']=='A') for b in range(1,7)]
    check(name+' '+mode+' recomputed paired ratios',ratios==data['summary'][name][mode]['paired_ratios'])
  else:
   inp=data['inputs'][name]
   check(name+' rejecting entry is not a speed baseline',inp['entry_rejection']['status']!=0)
   check(name+' frozen input',hashlib.sha256(inp['source'].encode()).hexdigest()==inp['sha256'])
   rows=[r for r in data['runs'] if r['workload']==name]
   check(name+' eight bounded compiles and checked runtimes',all(len([r for r in rows if r['mode']==m])==8 for m in ['compile','runtime']) and max(r['wall_s'] for r in rows if r['mode']=='compile')<45)
   for r in rows:
    if r['mode']=='compile':
     counters=r['phase_counters'][0];n=inp['N']
     check(name+' query work linear trial '+str(r['trial']),counters['semantic_type_query_work']==29*n+4 and counters['semantic_type_queries']==29*n+4 and counters['template_class_completions']==2*n)
check('hosted compiler identity',sha(hosted['compiler'])==hosted['sha256']==common['binaries']['B']['sha256'])
check('all 15 repaired fixtures measured',set(hosted['inputs'])==set(fixed))
check('hosted inputs unchanged',all(sha(root/p)==h for p,h in hosted['inputs'].items()))
for p in fixed:
 rows=[r for r in hosted['runs'] if r['path']==p]
 check(p+' four bounded successful observations',len(rows)==4 and all(r['status']==0 and r['wall_s']<45 for r in rows))
 check(p+' stable objects',len({r['object_sha256'] for r in rows})==1)
(out/'evidence-verification.json').write_text(json.dumps(dict(checks=checks,observations=len(common['runs'])+len(owner['runs'])+len(hosted['runs']),launchers=len(owner['launchers'])),indent=2)+'\n')
print(len(checks),'evidence checks passed')
