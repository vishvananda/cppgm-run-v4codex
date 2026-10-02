#!/usr/bin/env python3
"""Verify the checked-in implementation-216 handoff against frozen artifacts."""
import hashlib,json,pathlib,subprocess
root=pathlib.Path(__file__).resolve().parents[2];e=root/'student.tests/pa32/evidence216'
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
b=json.loads((e/'binding.json').read_text())
for path,digest in b['implementation'].items():assert sha(root/path)==digest,path
for path,digest in b['evidence'].items():assert sha(e/path)==digest,path
for path,digest in b['artifacts'].items():assert sha(pathlib.Path(path))==digest,path
plan=(root/'pa32/plan.md').read_text()
assert 'Stage base commit: '+b['stage_base'] in plan and 'Last reviewed commit: '+b['last_reviewed'] in plan
checks=json.loads((e/'checks.json').read_text());cs={x['name']:x for x in checks}
assert cs['prior']['exit_code']==0 and '5178 / 5178' in pathlib.Path(cs['prior']['log']).read_text()
assert cs['file']['exit_code']==0
stage=pathlib.Path(cs['stage']['log']).read_text();assert '213 / 219' in stage
entry=pathlib.Path(b['entry_log']).read_text()
def failures(text):return {s.split(': ERROR:',1)[0] for s in text.splitlines() if ': ERROR:' in s}
assert failures(stage)<failures(entry)
for c in checks:
 assert sha(pathlib.Path(c['log']))==c['sha256'],c['name']
 if c['name'] not in ['stage','through','debug','debug-driver-o1','debug-driver-o2','debug-driver-o3']:assert c['exit_code']==0,c['name']
subprocess.run(['git','merge-base','--is-ancestor',b['implementation_commit'],'HEAD'],cwd=root,check=True)
fixture_tree=subprocess.check_output(['git','ls-tree','-r','HEAD','pa32/tests'],cwd=root,text=True).strip()
assert hashlib.sha256(fixture_tree.encode()).hexdigest()==b['fixture_tree_sha256']
observations=0
for name in ['affected','common-o0','common-o1','common-o3','selfhost']:
 p=json.loads((e/(name+'.json')).read_text());observations+=len(p['runs'])
 assert all(row['status']==0 for row in p['runs'])
 for v in p['binaries'].values():assert sha(pathlib.Path(v['path']))==v['sha256']
 for row in p['runs']:
  if row.get('mode')=='compile':assert row['object_sha256']==p['images'][row['workload']][row['label']]['object_sha256'] if 'object_sha256' in row else True
assert observations==b['observations']
assert subprocess.check_output(['git','status','--short'],cwd=root,text=True)==''
print(f'PA32 implementation 216: verified {observations} observations, checks, failure reduction, source/artifact hashes and clean worktree')
