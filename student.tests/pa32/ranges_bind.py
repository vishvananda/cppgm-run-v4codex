#!/usr/bin/env python3
"""Preserve measured evidence and bind the implementation-215 handoff."""
import hashlib,json,os,pathlib,re,shutil,subprocess
root=pathlib.Path(__file__).resolve().parents[2];e=root/'student.tests/pa32/evidence215';e.mkdir(exist_ok=True)
art=pathlib.Path(os.environ['RALPH_ARTIFACT_DIR'])/'pa32-215'
def sha(p):return hashlib.sha256(pathlib.Path(p).read_bytes()).hexdigest()
def git(*a):return subprocess.check_output(['git',*a],cwd=root,text=True).strip()
lanes={'diagnostic-initial':'perf/performance.json','affected-runtime':'perf-final/performance.json','affected-compile':'accepted/performance.json',**{f'common-o{n}':f'common-o{n}/performance.json' for n in [0,1,3]},'selfhost':'selfhost/performance.json','checks':'checks-final/checks.json','range-bounds':'checks-final/range-bounds.json','loop-bounds':'checks-final/loop-bounds/bounds.json','memory-bounds':'checks-final/bounds.json','common-commands':'common-commands.json'}
for name,path in lanes.items():shutil.copyfile(art/path,e/(name+'.json'))
sources=set(git('ls-files','dev').splitlines())
sources.update(['AGENTS.md','spec.md','TESTING_AND_REFERENCES.md','PROJECT_LAYOUT.md','pa32/README.md','pa8/lowir.md'])
sources.update(str(p.relative_to(root)) for suffix in ['*.py','*.cpp'] for p in (root/'student.tests/pa32').glob(suffix))
def failures(path):return sorted(set(re.findall(r'^(pa32/[^:]+): ERROR:',path.read_text(),re.M)))
accepted=['affected-runtime','affected-compile','common-o0','common-o1','common-o3','selfhost']
b=dict(entry_commit='f4f075b80af3805aa0ef3d1784d723d82585f933',code_commit=git('rev-parse','7dd38d40'),stage_base='e82bf4152fe8d6d68b9cd966655db0d8142cf81b',last_reviewed='401519044a2c28130d4085b3bc7c3d0411b5dc8f',contract_tree=git('rev-parse','HEAD:pa32/tests'),artifact_root=str(art),
 entry_failures=failures(art/'baseline-stage.log'),remaining_failures=failures(art/'checks-final/stage.log'),sources={p:sha(root/p) for p in sorted(sources)},evidence={p.name:sha(p) for p in sorted(e.glob('*')) if p.is_file() and p.name!='binding.json'},
 artifacts={str(p):sha(p) for p in sorted(art.rglob('*')) if p.is_file()},
 binaries={str(p):sha(p) for p in [*(art/('cppgm-'+label) for label in 'ABCDE'),art/'lowiropt-final',art/'native-final',root/'dev/cppgm++',root/'dev/lowiropt',root/'dev/lowir2native']},
 bound_observations=sum(len(json.loads((e/(n+'.json')).read_text())['runs']) for n in accepted),current_acceptance_observations=1092,diagnostic_observations=len(json.loads((e/'diagnostic-initial.json').read_text())['runs']),
 interpretation='Incomplete implementation handoff, not a stage audit. No fixture/reference/comparison changes. Final compiler measurements use A/E; runtime A/D observations are retained only after byte-identical final A/E objects and executables were checked for every workload. Partial promotion is a mandated IR outcome with unchanged runtime/text and an observed 8% compiler cost; its extra frame home is owned by later allocation. Rejected empty-range policy and the first contended check attempt are preserved. Performance ratios are diagnostics, not additional course gates.')
old=json.loads((root/'student.tests/pa32/evidence214/binding.json').read_text())
assert sha(art/'cppgm-A') in old['binaries'].values()
(e/'binding.json').write_text(json.dumps(b,indent=2)+'\n')
print('bound',len(b['sources']),'sources,',len(b['artifacts']),'artifacts,',b['bound_observations'],'bound observations')
