#!/usr/bin/env python3
"""Verify implementation197 evidence bindings without rerunning benchmarks."""
import hashlib,json,pathlib,re,subprocess
root=pathlib.Path(__file__).resolve().parents[2]
e=root/'student.tests/pa30/evidence197'
def read(name):return json.loads((e/(name+'.json')).read_text())
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
checks=[]
def check(name,condition):
 assert condition,name
 checks.append(name)
entry=read('entry');delta=read('stage-delta');binding=read('source-binding');validation=read('validation');controls=read('controls')
check('entry commit',entry['commit']=='407fcdc0fceea44331958d1f0dd312432773fd35')
check('compiler controls binding',controls['sha256']==binding['compiler_sha256'])
for p,h in entry['protected'].items():check('protected '+p,sha(root/p)==h)
for p,h in binding['files'].items():check('source '+p,sha(root/p)==h)
check('coverage cardinality',len(entry['cases'])==len(delta['cases'])==len(list((root/'pa30/tests/compile').glob('*.t')))==153)
old={r['path']:r for r in entry['cases']};new={r['path']:r for r in delta['cases']}
check('coverage identities',old.keys()==new.keys())
check('no changed PA30 expectations',all(new[p]['expected']==r['expected'] for p,r in old.items()))
check('entry pass count',sum(r['actual']==r['expected'] for r in old.values())==129)
check('final pass count',sum(r['actual']==r['expected'] for r in new.values())==132)
check('fixed existing failures',sorted(delta['fixed'])==sorted(p for p in old if old[p]['actual']!=old[p]['expected'] and new[p]['actual']==new[p]['expected']))
check('no regressions',not delta['regressions'] and all(new[p]['actual']==new[p]['expected'] for p in old if old[p]['actual']==old[p]['expected']))
check('required earlier PAs',validation['priorThroughTests']['status']==0 and '(4941 / 4941)' in validation['priorThroughTests']['output'])
check('required stage progress',validation['stageTests']['status']==2 and '132 / 153' in validation['stageTests']['output'] and len(delta['fixed'])==3)
check('root through report',validation['through30']['status']==2 and '5073 / 5094' in validation['through30']['output'])
check('file audit',validation['fileAudit']['status']==0 and 'File audit passed' in validation['fileAudit']['output'])
for k,r in validation.items():check('retained check log '+k,hashlib.sha256(r['output'].encode()).hexdigest()==r['sha256'])
check('controls',len(controls['commands'])==23 and all((r['status']!=0)==r['expected_rejection'] for r in controls['commands']))
common=read('common-performance');owner=read('owner-performance');hosted=read('hosted-performance')
for k,r in [('common',common),('owner',owner)]:
 check(k+' flags',r['flags']==['-O0','-c','--stats'])
 check(k+' entry binary',r['binaries']['A']['sha256']==entry['compiler_sha256'])
 check(k+' final binary',r['binaries']['B']['sha256']==binding['compiler_sha256'])
 check(k+' observations',len(r['runs'])==(224 if k=='common' else 96))
 check(k+' success',all(v['status']==0 for v in r['runs']))
 check(k+' compiler timeout',all(v['wall_s']<45 for v in r['runs'] if v['mode']=='compile'))
for k,images in common['images'].items():
 check('equivalent images '+k,images['A']==images['B'])
 for mode in ['compile','runtime']:
  rows=[r for r in common['runs'] if r['workload']==k and r['mode']==mode]
  check('A/A + six ABBA '+k+mode,[''.join(r['label'] for r in rows if r['block']==b) for b in range(7)]==['AAAA']+['ABBA']*6)
check('owner sizes',sorted(v['N'] for v in owner['inputs'].values())==[128,128,512,512,2048,2048])
for k,v in owner['inputs'].items():
 check('frozen owner input '+k,hashlib.sha256(v['source'].encode()).hexdigest()==v['sha256'])
 check('entry cannot compile corrected input '+k,v['entry_rejection']['status']!=0)
check('launch calibration',len(owner['launchers'])==16)
check('hosted binary',hosted['sha256']==binding['compiler_sha256'])
check('hosted coverage',sorted(hosted['inputs'])==sorted(delta['fixed']))
check('hosted observations',len(hosted['runs'])==12 and all(v['status']==0 and v['wall_s']<45 for v in hosted['runs']))
check('hosted inputs',all(sha(root/p)==h for p,h in hosted['inputs'].items()))
equivalent=read('equivalent-performance');followup=read('followup-performance')
check('large equivalent images',equivalent['images']['A']==equivalent['images']['B'])
check('large equivalent input',hashlib.sha256(equivalent['source'].encode()).hexdigest()==equivalent['input_sha256'])
check('large equivalent binary',equivalent['binaries']['A']['sha256']==entry['compiler_sha256'] and equivalent['binaries']['B']['sha256']==binding['compiler_sha256'])
check('large equivalent observations',len(equivalent['runs'])==56 and all(v['status']==0 and v['wall_s']<45 for v in equivalent['runs']))
for mode in ['compile','runtime']:
 rows=[r for r in equivalent['runs'] if r['mode']==mode]
 check('large equivalent ABBA '+mode,[''.join(r['label'] for r in rows if r['block']==b) for b in range(7)]==['AAAA']+['ABBA']*6)
check('follow-up binary',followup['compiler_sha256']==binding['compiler_sha256'])
check('follow-up observations',len(followup['runs'])==16 and all(v['status']==0 and v['wall_s']<45 for v in followup['runs']))
check('follow-up inputs',all(r['source_sha256']==owner['inputs'][k]['sha256'] and r['executable_sha256']==owner['images'][k]['executable_sha256'] for k,r in followup['inputs'].items()))
check('follow-up objects',all(r['object_sha256']==owner['images'][r['workload']]['object_sha256'] for r in followup['runs'] if r['mode']=='compile'))
changed=subprocess.check_output(['git','diff',entry['commit'],'--name-only'],cwd=root,text=True).splitlines()
changed=[p for p in changed if re.match(r'pa\d+/(tests/|scripts/|Makefile$)|scripts/|Makefile$',p)]
expected=['pa6/tests/general/300-ambiguous-using-directive-type-bad'+s for s in ['.ref','.ref.exit_status','.ref.stdout']]
check('reference correction whitelist',sorted(changed)==sorted(expected))
plan=(root/'pa30/plan.md').read_text()
for marker in ['Stage base commit','Last reviewed commit']:
 check('preserved '+marker,marker+': `27029f978e65b78331233123922d342033d5d1f7`' in plan)
check('live implementation binding',sha(root/'dev/cppgm++')==binding['compiler_sha256'])
(e/'evidence-verification.json').write_text(json.dumps(dict(passed=len(checks),checks=checks),indent=2)+'\n')
print(len(checks),'evidence checks passed')
