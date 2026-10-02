#!/usr/bin/env python3
"""Bind implementation 217, final evidence and all retained observations."""
import hashlib,json,pathlib,shutil,subprocess
root=pathlib.Path(__file__).resolve().parents[2];art=pathlib.Path('/tmp/pa32-217');out=root/'student.tests/pa32/evidence217'
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def git(*args):return subprocess.check_output(['git',*args],cwd=root,text=True).strip()
entry='7e04081b611b89cd46e5b294a963aba891c36073'
for name,path in [('affected','closed-affected/performance.json'),('historical-initial','affected/performance.json'),
                  ('historical-concurrent','final-affected/performance.json'),('common-o0','common-o0/performance.json'),
                  ('common-o1','common-o1/performance.json'),('common-o3','common-o3/performance.json'),
                  ('selfhost','selfhost/performance.json'),('checks','final-checks/checks.json'),
                  ('bounds','final-checks/bounds.json'),('native-inspection','native-inspection.json')]:
    shutil.copyfile(art/path,out/(name+'.json'))
paths=git('diff','--name-only',entry,'--','dev').splitlines()
paths+=git('ls-files','student.tests/pa32/source*.py','student.tests/pa32/pointer_congruence.py').splitlines()
paths+=['student.tests/pa32/source_bind.py','student.tests/pa32/source_verify.py',
        'student.tests/pa32/source_checks.py','student.tests/pa32/source_performance.py',
        'student.tests/pa32/common_levels.py','student.tests/pa32/selfhost_performance.py',
        'AGENTS.md','TESTING_AND_REFERENCES.md','spec.md','pa32/README.md','pa32/plan.md',
        'pa32/reference-corrections.md','pa8/lowir.md']
fixtures=git('diff','--name-only',entry,'HEAD','--','pa32/tests').splitlines()
expected=['pa32/tests/o1/500-effect-free-loop-deleted-twin-backward.ref',
          'pa32/tests/o1/500-effect-free-loop-deleted-twin-backward.ref.expect']
assert fixtures==expected,fixtures
paths+=fixtures
files=[p for p in art.rglob('*') if p.is_file()]
b=dict(entry=entry,implementation_commit='c81197b04c7ddadfcdaa3a2be596e3d04c7d0e44',
       stage_base='e82bf4152fe8d6d68b9cd966655db0d8142cf81b',last_reviewed='401519044a2c28130d4085b3bc7c3d0411b5dc8f',
       implementation={p:sha(root/p) for p in sorted(set(paths))},
       evidence={p.name:sha(p) for p in sorted(out.glob('*')) if p.is_file() and p.name!='binding.json'},
       artifacts={str(p):sha(p) for p in sorted(files)},
       fixture_delta=fixtures,fixture_tree_sha256=hashlib.sha256(git('ls-tree','-r','HEAD','pa32/tests').encode()).hexdigest(),
       entry_log=str(art/'entry-stage.log'),observations=868,historical_observations=280,
       unfinished_implementation=['source-debug O1','source-debug O2','source-debug O3'],
       independent_review='Accumulated changes since '+git('rev-parse','40151904'))
assert sha(root/'dev/cppgm++')==sha(art/'final-cppgm++')
assert sha(root/'dev/lowiropt')==sha(art/'final-lowiropt')
(out/'binding.json').write_text(json.dumps(b,indent=2)+'\n')
print('Bound 217 checks, 868 final/280 historical observations and exact reference correction')
