#!/usr/bin/env python3
"""Check frozen evidence consistency; no compiler or executable measurements."""
import hashlib,json,pathlib,statistics,sys
root=pathlib.Path(__file__).resolve().parents[2]
out=root/'student.tests/pa30/evidence195'
checks=[]
def check(name,condition):
 assert condition,name
 checks.append(name)
def read(name):return json.loads((out/name).read_text())
def sha(p):return hashlib.sha256(pathlib.Path(p).read_bytes()).hexdigest()
entry=read('entry.json');binding=read('source-binding.json');delta=read('stage-delta.json')
check('all course fixture/reference/harness bytes preserved',all(sha(root/p)==h for p,h in entry['coverage'].items()))
check('all tested implementation bytes preserved',all(sha(root/p)==h for p,h in binding['sources'].items()))
check('frozen compilers preserved',all(sha(p)==h for p,h in binding['binaries'].items()))
check('existing failures reduced without losing a passing case',delta['total']==153 and delta['passed']>delta['baseline_passed'] and not delta['regressed'])
check('root earlier stages and file audit pass',read('validation.json')[0]['status']==0 and read('validation.json')[2]['status']==0)
common=read('common-performance.json');owner=read('owner-performance.json')
controls=read('controls.json')
check('controls use measured final compiler',controls['sha256']==common['binaries']['B']['sha256'])
check('all 28 explicit controls meet expected outcomes',len(controls['commands'])==28 and all((v['status']!=0) if v['expected_rejection'] else (v['status']==0) for v in controls['commands']))
check('combined through30 matches current stage delta',read('validation.json')[3]['status']==2 and '5055 / 5094' in read('validation.json')[3]['tail'])
for group,data in [('common',common),('owner',owner)]:
 check(group+' binary identity',all(sha(v['path'])==v['sha256'] for v in data['binaries'].values()))
 check(group+' measured successes',all(row['status']==0 for row in data['runs']))
 for name,img in data['images'].items():
  if group=='common':
   check(name+' equivalent object and executable bytes',img['A']==img['B'])
   for mode in ['compile','runtime']:
    rows=[v for v in data['runs'] if v['workload']==name and v['mode']==mode]
    check(name+' '+mode+' AA/ABBA orders',[''.join(v['label'] for v in rows if v['block']==b) for b in range(7)]==['AAAA']+['ABBA']*6)
    ratios=[statistics.mean(v['wall_s'] for v in rows if v['block']==b and v['label']=='B')/statistics.mean(v['wall_s'] for v in rows if v['block']==b and v['label']=='A') for b in range(1,7)]
    check(name+' '+mode+' recomputed paired ratios',ratios==data['summary'][name][mode]['paired_ratios'])
  else:
   check(name+' entry invalid is not a speed baseline',data['inputs'][name]['entry_rejection']['status']!=0)
   check(name+' input frozen',hashlib.sha256(data['inputs'][name]['source'].encode()).hexdigest()==data['inputs'][name]['sha256'])
   check(name+' compile budget',max(v['wall_s'] for v in data['runs'] if v['workload']==name and v['mode']=='compile')<45)
   check(name+' runtime checked',len([v for v in data['runs'] if v['workload']==name and v['mode']=='runtime'])==8)
hosted=read('hosted-performance.json')
check('hosted final compiler identity',sha(hosted['compiler'])==hosted['sha256']==common['binaries']['B']['sha256'])
check('hosted fixture bytes preserved',all(sha(root/p)==h for p,h in hosted['inputs'].items()))
check('all nine repaired hosted cases measured',set(hosted['inputs'])==set(delta['fixed']))
for path in hosted['inputs']:
 rows=[v for v in hosted['runs'] if v['path']==path]
 check(path+' four successful bounded observations',len(rows)==4 and all(v['status']==0 and v['wall_s']<45 for v in rows))
 check(path+' stable measured objects',len({v['object_sha256'] for v in rows})==1)
(out/'evidence-verification.json').write_text(json.dumps(dict(checks=checks,observations=len(common['runs'])+len(owner['runs'])+len(hosted['runs']),launchers=len(owner['launchers'])),indent=2)+'\n')
print(len(checks),'evidence checks passed')
