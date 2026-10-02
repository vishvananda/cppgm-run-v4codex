#!/usr/bin/env python3
"""Bind final PA32 216 evidence to implementation, tests and retained artifacts."""
import hashlib,json,pathlib,shutil,subprocess
root=pathlib.Path(__file__).resolve().parents[2];art=pathlib.Path('/tmp/pa32-216');out=root/'student.tests/pa32/evidence216'
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def git(*args):return subprocess.check_output(['git',*args],cwd=root,text=True).strip()
for name,path in [('affected','closed-performance/performance.json'),('common-o0','closed-common-o0/performance.json'),('common-o1','closed-common-o1/performance.json'),('common-o3','closed-common-o3/performance.json'),('selfhost','closed-selfhost/performance.json'),('checks','closed-checks/checks.json'),('bounds','closed-checks/bounds.json')]:shutil.copyfile(art/path,out/(name+'.json'))
entry='335618379eb20ac285cc1df5fb498f498afccccf'
paths=git('diff','--name-only',entry,'--','dev').splitlines()
paths+=git('ls-files','student.tests/pa32/context*.py','student.tests/pa32/floating_facts.py').splitlines()
paths+=['AGENTS.md','TESTING_AND_REFERENCES.md','spec.md','pa32/README.md','pa32/plan.md']
files=[p for p in art.rglob('*') if p.is_file()]
b=dict(entry=entry,implementation_commit='2bace8a0',stage_base='e82bf4152fe8d6d68b9cd966655db0d8142cf81b',last_reviewed='401519044a2c28130d4085b3bc7c3d0411b5dc8f',
 implementation={p:sha(root/p) for p in sorted(set(paths))},
 evidence={p.name:sha(p) for p in sorted(out.glob('*')) if p.is_file() and p.name!='binding.json'},
 artifacts={str(p):sha(p) for p in sorted(files)},entry_log=str(art/'entry-stage.log'),observations=924,historical_observations=924,
 fixture_tree_sha256=hashlib.sha256(git('ls-tree','-r',entry,'pa32/tests').encode()).hexdigest())
assert git('ls-tree','-r',entry,'pa32/tests')==git('ls-tree','-r','HEAD','pa32/tests')
(out/'binding.json').write_text(json.dumps(b,indent=2)+'\n')
print('Bound final evidence, complete retained artifacts and unchanged fixture tree')
