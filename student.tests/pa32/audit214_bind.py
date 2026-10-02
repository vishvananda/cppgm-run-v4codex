#!/usr/bin/env python3
"""Freeze audit inputs/results before committing the code and evidence boundary."""
import hashlib,json,os,pathlib,re,shutil,subprocess
ROOT=pathlib.Path(__file__).resolve().parents[2];E=ROOT/'student.tests/pa32/evidence214'
art=pathlib.Path(os.environ['RALPH_ARTIFACT_DIR'])/'pa32-214'
def sha(p):return hashlib.sha256(pathlib.Path(p).read_bytes()).hexdigest()
def git(*args):return subprocess.check_output(['git',*args],cwd=ROOT,text=True).strip()
E.mkdir(exist_ok=True)
lanes=['objects','loops','memory','common-o0','common-o1','common-o3','selfhost']
for lane in lanes:shutil.copyfile(art/lane/'performance.json',E/(lane+'.json'))
for target,source in [('checks.json','checks/checks.json'),('history.json','history-checks.json'),('bounds.json','checks/bounds.json'),('loop-bounds.json','checks/loop-bounds/bounds.json'),('commands.json','performance-commands.json')]:shutil.copyfile(art/source,E/target)
sources=set(git('ls-files','dev').splitlines())
sources.update(['spec.md','AGENTS.md','TESTING_AND_REFERENCES.md','PROJECT_LAYOUT.md','pa32/README.md','pa8/lowir.md'])
sources.update(str(p.relative_to(ROOT)) for p in (ROOT/'student.tests/pa32').glob('*.py'))
sources.update(str(p.relative_to(ROOT)) for p in (ROOT/'student.tests/pa32').glob('*.cpp'))
failures=sorted(set(re.findall(r'^(pa32/[^:]+): ERROR:',(art/'checks/stage.log').read_text(),re.M)))
b=dict(entry_commit='86d08255ca50fec798f6b0fa209eb8d2bd21a04b',last_reviewed='76d3fcb24059d1557b6dd0e398cf3896931fcca9',stage_base='e82bf4152fe8d6d68b9cd966655db0d8142cf81b',
 contract_tree=git('rev-parse','HEAD:pa32/tests'),artifact_root=str(art),remaining_failures=failures,
 sources={p:sha(ROOT/p) for p in sorted(sources)},
 evidence={p.name:sha(p) for p in sorted(E.glob('*.json')) if p.name!='binding.json'},
 artifacts={str(p):sha(p) for p in sorted(art.rglob('*')) if p.is_file()},
 binaries={str(p):sha(p) for p in [art/'entry-cppgm++',art/'entry-lowiropt',art/'final-cppgm++',art/'final-lowiropt',ROOT/'dev/lowir2native']},
 performance_lanes=lanes,sample_count=sum(len(json.loads((E/(s+'.json')).read_text())['runs']) for s in lanes),
 historical_samples=4536,reviewed_implementation_paths=git('diff','--name-only','76d3fcb2','--','dev').splitlines(),
 validation=dict(prior=[5178,5178],stage=[202,219],through=[5380,5397],file='pass; four inherited warnings',direct_debug=[5,5],source_debug=[0,3],objects=[25,25],debug_objects=[25,25],new_reducers=48,unexported_reducer=1),
 interpretation='Performance ratios are diagnostics, not extra gates. Mandatory work/growth bounds, correctness, course predicates and coverage are preserved. No oracle change.')
(E/'binding.json').write_text(json.dumps(b,indent=2)+'\n')
print('bound',len(b['sources']),'sources,',len(b['artifacts']),'artifacts,',b['sample_count'],'current samples')
