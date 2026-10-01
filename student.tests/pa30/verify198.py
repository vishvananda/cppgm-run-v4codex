#!/usr/bin/env python3
"""Check the audit's current bindings, historical provenance and acceptance gates."""
import hashlib, json, pathlib, posixpath, re, subprocess
root = pathlib.Path(__file__).resolve().parents[2]
e = root/'student.tests/pa30/evidence198'
def read(name): return json.loads((e/(name+'.json')).read_text())
def sha(data): return hashlib.sha256(data).hexdigest()
def git(*args): return subprocess.check_output(['git',*args],cwd=root)
checks = []
def check(name, condition):
    assert condition, name
    checks.append(name)
entry, binding, validation, delta = [read(n) for n in ['entry','source-binding','validation','stage-delta']]
tip = binding['reviewed_code_tip']
check('no code beyond reviewed tip', not git('diff',tip,'--','dev').strip())
check('reviewed dev tree', git('rev-parse',tip+':dev').decode().strip()==binding['dev_tree'])
check('live compiler binding', sha((root/'dev/cppgm++').read_bytes())==binding['compiler_sha256'])
for p,h in binding['files'].items(): check('source '+p,sha((root/p).read_bytes())==h)
allowed = ['pa30/tests/compile/400-base-same-type-alias-ambiguous-bad.ref'+s for s in ['.exit_status','.stdout']]
changed = []
for p,h in entry['protected'].items():
    if sha((root/p).read_bytes()) != h: changed.append(p)
check('only proven audit reference correction', sorted(changed)==sorted(allowed))
history_allowed = allowed + ['pa6/tests/general/300-ambiguous-using-directive-type-bad'+s for s in ['.ref','.ref.exit_status','.ref.stdout']]
protected_diff = [p for p in git('diff',entry['stage_base'],'--name-only').decode().splitlines()
                  if re.match(r'pa\d+/(tests/|scripts/|Makefile$)|scripts/|Makefile$',p)]
check('complete-range fixture/harness protection', sorted(protected_diff)==sorted(history_allowed))
for name,v in validation.items(): check('retained log '+name,sha(v['output'].encode())==v['sha256'])
check('earlier PAs',validation['priorThroughTests']['status']==0 and '(4941 / 4941)' in validation['priorThroughTests']['output'])
check('file audit',validation['fileAudit']['status']==0 and 'File audit passed' in validation['fileAudit']['output'])
check('stage command',validation['stageTests']['status']==2 and '132 / 153' in validation['stageTests']['output'])
check('through30 command',validation['through30']['status']==2 and '5073 / 5094' in validation['through30']['output'])
check('same failures',sorted(delta['entry_failures'])==sorted(delta['final_failures']) and len(delta['final_failures'])==21 and not delta['regressions'])
check('same coverage',sorted(r['path'] for r in entry['cases'])==sorted(r['path'] for r in delta['cases'])==sorted(str(p.relative_to(root)) for p in (root/'pa30/tests/compile').glob('*.t')))
check('fixture count',len(delta['cases'])==153)
check('primary baseline', '132 / 153' in entry['primary_log'])
for name,count in [('controls',107),('trace',34)]:
    r=read(name)
    check(name+' compiler',r['compiler_sha256']==binding['compiler_sha256'])
    check(name+' count',len(r['commands'])==count)
    check(name+' results',all((v['status']!=0)==v['expected_rejection'] for v in r['commands']))
trace=read('trace')
check('telemetry is observational',all(v['telemetry_identical'] for v in trace['images'].values()))
check('old adapter defect reproduced','duplicate singleton role' in trace['commands'][1]['stderr'])
for name in trace['views']:
    check('one explicit entry '+name,trace['views'][name]['lowir'].count('role=entry')==1)
check('dormant template not emitted','dormant' not in trace['views']['optimization']['lowir'])
check('volatile effect retained','volatile' in trace['views']['optimization']['lowir'] and '@observations' in trace['views']['optimization']['mir'])
for name in ['common-performance','common-repeat-performance']:
    r=read(name)
    check(name+' final binary',r['binaries']['B']['sha256']==binding['compiler_sha256'])
    check(name+' stage-base binary',r['binaries']['A']['sha256']==binding['base_compiler_sha256'])
    check(name+' observations',len(r['runs'])==224 and all(v['status']==0 for v in r['runs']))
    check(name+' flags',r['flags']==['-O0','-c','--stats'])
    for workload,images in r['images'].items():
        check(name+' equivalent images '+workload,images['A']==images['B'])
        for mode in ['compile','runtime']:
            rows=[v for v in r['runs'] if v['workload']==workload and v['mode']==mode]
            check(name+' A/A + six ABBA '+workload+mode,[''.join(v['label'] for v in rows if v['block']==b) for b in range(7)]==['AAAA']+['ABBA']*6)
owner=read('owner-performance')
check('owner observations',len(owner['runs'])==48 and len(owner['launchers'])==16)
check('checked owner executions',all(v['status']==0 for v in owner['runs']))
check('owner final binary',owner['binaries']['B']['sha256']==binding['compiler_sha256'])
for k,v in owner['inputs'].items():
    check('frozen owner input '+k,sha(v['source'].encode())==v['sha256'])
    check('entry rejection not a timing baseline '+k,v['entry_rejection']['status']!=0)
    for row in [r for r in owner['runs'] if r['workload']==k and r['mode']=='compile']:
        counters=row['phase_counters'][0]; n=v['N']
        check('bounded owner work '+k+str(row['trial']),counters['semantic_lookup_work']==73*n+60 and counters['template_class_completions']==3*n and counters['max_pending']==10)
        check('identical owner output '+k+str(row['trial']),row['object_sha256']==owner['images'][k]['object_sha256'])
check('current measured compile limit',all(v['status']==0 and v['wall_s']<45 for n in ['common-performance','common-repeat-performance','owner-performance','hosted-performance'] for v in read(n)['runs'] if v.get('mode','compile')=='compile'))
check('all repaired hosted fixtures remeasured',len(read('hosted-performance')['inputs'])==27 and len(read('hosted-performance')['runs'])==108)
hosted=read('hosted-performance')
check('hosted binary binding',hosted['sha256']==binding['compiler_sha256'])
for p,h in hosted['inputs'].items():
    check('hosted input '+p,sha((root/p).read_bytes())==h)
    rows=[v for v in hosted['runs'] if v['path']==p]
    check('hosted repeated object '+p,len(rows)==4 and len({v['object_sha256'] for v in rows})==1)
# Historical bindings are checked at their own immutable code commits, not
# against the current tree. Resolve tracked wrapper symlinks before hashing.
for n in [195,196,197]:
    directory=root/f'student.tests/pa30/evidence{n}'
    b=json.loads((directory/'source-binding.json').read_text())
    rev=b.get('implementation_commit',b.get('head'))
    tree={}
    for line in git('ls-tree','-r',rev).decode().splitlines():
        meta,p=line.split('\t'); tree[p]=meta.split()
    for p,h in b.get('sources',b.get('files')).items():
        if not p.startswith('dev/'): continue
        path=p
        while tree[path][0]=='120000':
            path=posixpath.normpath(posixpath.join(posixpath.dirname(path),git('show',rev+':'+path).decode()))
        check('historical source '+str(n)+' '+p,sha(git('show',rev+':'+path))==h)
    common=json.loads((directory/'common-performance.json').read_text())
    check('historical images '+str(n),all(v['A']==v['B'] for v in common['images'].values()))
    check('historical ABBA observations '+str(n),len(common['runs'])==224 and all(v['status']==0 for v in common['runs']))
for path in ['pa30/plan.md','pa30/audit.md']:
    check('review boundary '+path,'Last reviewed commit: `'+tip+'`' in (root/path).read_text())
(e/'verification.json').write_text(json.dumps(dict(passed=len(checks),checks=checks),indent=2)+'\n')
print(len(checks),'audit evidence checks passed')
