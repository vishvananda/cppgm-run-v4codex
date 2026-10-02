#!/usr/bin/env python3
"""Verify implementation-217 handoff without treating it as independent audit."""
import hashlib,json,pathlib,subprocess
root=pathlib.Path(__file__).resolve().parents[2];e=root/'student.tests/pa32/evidence217'
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def git(*args):return subprocess.check_output(['git',*args],cwd=root,text=True).strip()
b=json.loads((e/'binding.json').read_text())
for path,digest in b['implementation'].items():assert sha(root/path)==digest,path
for path,digest in b['evidence'].items():assert sha(e/path)==digest,path
for path,digest in b['artifacts'].items():assert sha(pathlib.Path(path))==digest,path
plan=(root/'pa32/plan.md').read_text()
assert 'Stage base commit: '+b['stage_base'] in plan and 'Last reviewed commit: '+b['last_reviewed'] in plan
cs={x['name']:x for x in json.loads((e/'checks.json').read_text())}
for name,count in [('prior','5178 / 5178'),('stage','219 / 219'),('through','5397 / 5397')]:
    assert cs[name]['exit_code']==0 and count in pathlib.Path(cs[name]['log']).read_text(),name
assert cs['file']['exit_code']==0
def failures(text):return {s.split(': ERROR:',1)[0] for s in text.splitlines() if ': ERROR:' in s}
entry=failures(pathlib.Path(b['entry_log']).read_text())
current=failures(pathlib.Path(cs['stage']['log']).read_text())
assert len(entry)==6 and not current and current<entry
debug={'debug','debug-driver-o1','debug-driver-o2','debug-driver-o3'}
for name,c in cs.items():
    assert sha(pathlib.Path(c['log']))==c['sha256'],name
    assert (c['exit_code']!=0)==(name in debug),name
assert len(b['unfinished_implementation'])==3
subprocess.run(['git','merge-base','--is-ancestor',b['implementation_commit'],'HEAD'],cwd=root,check=True)
assert git('diff','--name-only',b['entry'],'HEAD','--','pa32/tests').splitlines()==b['fixture_delta']
assert hashlib.sha256(git('ls-tree','-r','HEAD','pa32/tests').encode()).hexdigest()==b['fixture_tree_sha256']
observations=0
for name in ['affected','common-o0','common-o1','common-o3','selfhost']:
    p=json.loads((e/(name+'.json')).read_text());observations+=len(p['runs'])
    assert all(row['status']==0 for row in p['runs'])
    for v in p['binaries'].values():assert sha(pathlib.Path(v['path']))==v['sha256']
    for row in p['runs']:
        if row.get('mode')=='compile' and 'object_sha256' in row:
            assert row['object_sha256']==p['images'][row['workload']][row['label']]['object_sha256']
    if name.startswith('common-'):
        assert all(v['A']['object_sha256']==v['B']['object_sha256'] for v in p['images'].values())
assert observations==b['observations']
assert sum(len(json.loads((e/(n+'.json')).read_text())['runs']) for n in ['historical-initial','historical-concurrent'])==b['historical_observations']
assert sha(root/'dev/cppgm++')==sha(pathlib.Path('/tmp/pa32-217/final-cppgm++'))
assert sha(root/'dev/lowiropt')==sha(pathlib.Path('/tmp/pa32-217/final-lowiropt'))
assert not git('status','--short')
print('PA32 implementation 217 verified: 5397/5397 through tests, file audit, 868 final/280 historical observations, exact oracle correction, clean handoff; three source-debug checks and independent review remain')
