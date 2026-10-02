#!/usr/bin/env python3
"""Bind frozen 213 evidence after checks; logs/objects remain outside git."""
import hashlib,json,pathlib,re,shutil,subprocess,sys
root=pathlib.Path(__file__).resolve().parents[2];artifact=pathlib.Path(sys.argv[1]).resolve()
out=root/'student.tests/pa32/evidence213';out.mkdir(exist_ok=True)
def sha(p):return hashlib.sha256(pathlib.Path(p).read_bytes()).hexdigest()
def git(*args):return subprocess.check_output(['git',*args],cwd=root,text=True).strip()
lanes={'affected':'performance-handoff','common-o0':'common-handoff-o0','common-o1':'common-handoff-o1','common-o3':'common-handoff-o3','selfhost':'selfhost-handoff',
 'exploratory':'performance','intermediate':'performance-final','policy':'performance-release','pre-cost':'performance-complete',
 'pre-cost-common-o0':'common-o0','pre-cost-common-o1':'common-o1','pre-cost-common-o3':'common-o3','pre-cost-selfhost':'selfhost',
 'private-long':'private-long.json','copies-long':'copies-long.json','private-handoff-long':'private-handoff-long.json','copies-handoff-long':'copies-handoff-long.json'}
samples=0
for lane,directory in lanes.items():
 path=artifact/directory if directory.endswith('.json') else artifact/directory/'performance.json'
 d=json.loads(path.read_text());samples+=len(d['runs']);shutil.copyfile(path,out/(lane+'.json'))
for name,source in [('checks.json','checks-handoff/checks.json'),('bounds.json','checks-handoff/bounds.json'),('cost-diagnosis.json','cost-diagnosis.json')]:shutil.copyfile(artifact/source,out/name)
checks=json.loads((out/'checks.json').read_text())
def locate(path):return artifact/pathlib.Path(path).relative_to('/tmp/pa32-213')
base=(artifact/'baseline.log').read_text();now=locate(next(c['log'] for c in checks if c['name']=='stage')).read_text()
def failed(s):return set(re.findall(r'^(pa32/[^:]+): ERROR:',s,re.M))
paths=git('ls-files','dev','student.tests/pa32').splitlines()
paths=[p for p in paths if '/evidence' not in p]
paths += ['student.tests/pa32/memory_bind.py','student.tests/pa32/memory_verify.py','AGENTS.md','TESTING_AND_REFERENCES.md','PROJECT_LAYOUT.md','spec.md','pa32/README.md','pa32/plan.md','pa32/audit.md','pa8/lowir.md','pa24/README.md']
b=dict(entry_commit='cec5b40ae0de687d217cc787e1cca342591a7c56',implementation_commit=git('rev-parse','5b5512b4'),dev_tree=git('rev-parse','HEAD:dev'),
 stage_base='e82bf4152fe8d6d68b9cd966655db0d8142cf81b',last_reviewed='76d3fcb24059d1557b6dd0e398cf3896931fcca9',artifact_root=str(artifact),
 contract_tree=git('rev-parse','HEAD:pa32/tests'),sources={p:sha(root/p) for p in sorted(set(paths)) if (root/p).is_file()},
 artifacts={str(p):sha(p) for p in sorted(artifact.rglob('*')) if p.is_file()},
 binaries={str(artifact/p):sha(artifact/p) for p in ['A','lowiropt-A','B','lowiropt-B','B-final','lowiropt-B-final','B-release','lowiropt-B-release','B-complete','lowiropt-B-complete','B-cost-diagnostic','B-handoff','lowiropt-B-handoff']},
 performance_lanes=lanes,sample_count=samples,diagnostic_sample_count=len(json.loads((artifact/'cost-diagnosis.json').read_text())),
 resolved_failures=sorted(failed(base)-failed(now)),remaining_failures=sorted(failed(now)))
(out/'binding.json').write_text(json.dumps(b,indent=2)+'\n')
print('bound',len(b['sources']),'sources,',len(b['artifacts']),'artifacts,',samples,'A/A + ABBA samples,',b['diagnostic_sample_count'],'diagnostic samples')
