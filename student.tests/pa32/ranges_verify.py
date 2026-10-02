#!/usr/bin/env python3
"""Verify implementation-215 source, check, coverage and performance bindings."""
import hashlib,json,pathlib,re,subprocess
root=pathlib.Path(__file__).resolve().parents[2];e=root/'student.tests/pa32/evidence215'
b=json.loads((e/'binding.json').read_text())
def sha(p):return hashlib.sha256(pathlib.Path(p).read_bytes()).hexdigest()
def git(*a):return subprocess.check_output(['git',*a],cwd=root,text=True).strip()
assert git('diff','--name-only',b['code_commit'],'--','dev')==''
assert git('rev-parse','HEAD:pa32/tests')==b['contract_tree']==git('rev-parse',b['entry_commit']+':pa32/tests')
for p,h in b['sources'].items():assert sha(root/p)==h,p
for p,h in b['evidence'].items():assert sha(e/p)==h,p
for p,h in b['artifacts'].items():assert sha(p)==h,p
for p,h in b['binaries'].items():assert sha(p)==h,p
plan=(root/'pa32/plan.md').read_text()
assert 'Stage base commit: '+b['stage_base'] in plan
assert 'Last reviewed commit: '+b['last_reviewed'] in plan
checks=json.loads((e/'checks.json').read_text());by={r['name']:r for r in checks}
expected_failures={'stage','through','debug','debug-driver-o1','debug-driver-o2','debug-driver-o3'}
for r in checks:
 assert sha(r['log'])==r['sha256'],r['name']
 assert (r['exit_code']!=0)==(r['name'] in expected_failures),r['name']
assert '5178 / 5178' in pathlib.Path(by['prior']['log']).read_text()
assert '208 / 219' in pathlib.Path(by['stage']['log']).read_text()
assert '5386 / 5397' in pathlib.Path(by['through']['log']).read_text()
assert 'File audit passed' in pathlib.Path(by['file']['log']).read_text()
assert all('PASS (25/25)' in pathlib.Path(by[n]['log']).read_text() for n in ['normal-objects','debug-objects'])
old=set(b['entry_failures']);new=set(re.findall(r'^(pa32/[^:]+): ERROR:',pathlib.Path(by['stage']['log']).read_text(),re.M))
assert new==set(b['remaining_failures']) and new<old and len(old-new)==6
assert len(old)==17 and len(new)==11
runtime=json.loads((e/'affected-runtime.json').read_text());compile=json.loads((e/'affected-compile.json').read_text())
assert sha(root/'dev/cppgm++')==compile['binaries']['B']['sha256']
assert set(runtime['inputs'])==set(compile['inputs'])
for name in runtime['inputs']:
 assert runtime['inputs'][name]==compile['inputs'][name]
 for label in 'AB':
  for key in ['object_sha256','executable_sha256','object_text_bytes','text_bytes']:
   assert runtime['images'][name][label][key]==compile['images'][name][label][key],(name,label,key)
for name in ['affected-runtime','affected-compile','common-o0','common-o1','common-o3','selfhost']:
 r=json.loads((e/(name+'.json')).read_text())
 groups={}
 for sample in r['runs']:
  assert sample['status']==0
  groups.setdefault((sample.get('workload','selfhost'),sample.get('mode','compile')),[]).append(sample)
 for key,rows in groups.items():
  assert len(rows)==28,(name,key,len(rows))
  assert ''.join(v['label'] for v in rows if v['block']==0)=='AAAA'
  for block in range(1,7):assert ''.join(v['label'] for v in rows if v['block']==block)=='ABBA'
 for label,info in r['binaries'].items():assert sha(info['path'])==info['sha256']
for level in [0,1,3]:
 r=json.loads((e/f'common-o{level}.json').read_text())
 assert all(v['A']['object_sha256']==v['B']['object_sha256'] for v in r['images'].values())
r=json.loads((e/'selfhost.json').read_text());assert r['images']['A']['sha256']==r['images']['B']['sha256']
assert b['bound_observations']==sum(len(json.loads((e/(n+'.json')).read_text())['runs']) for n in ['affected-runtime','affected-compile','common-o0','common-o1','common-o3','selfhost'])
assert b['current_acceptance_observations']==b['bound_observations']-sum(r['mode']=='compile' for r in runtime['runs'])
print('implementation 215 verified: 17 -> 11 existing failures; unchanged coverage; 5178 prior tests; file audit; source/binary/evidence bindings; 1288 bound observations (1092 support current acceptance)')
