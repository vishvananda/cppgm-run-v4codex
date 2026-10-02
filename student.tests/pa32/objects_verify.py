#!/usr/bin/env python3
"""Verify the implementation-211 evidence without treating PA32 as complete."""
import hashlib,json,pathlib,re,statistics,subprocess
ROOT=pathlib.Path(__file__).resolve().parents[2]
E=ROOT/'student.tests/pa32/evidence211'
def sha(p): return hashlib.sha256(pathlib.Path(p).read_bytes()).hexdigest()
def git(*args): return subprocess.check_output(['git',*args],cwd=ROOT,text=True).strip()
def read(name): return json.loads((E/name).read_text())
b=read('binding.json')
for name,digest in b['sources'].items(): assert sha(ROOT/name)==digest,name
for path,digest in b['artifacts'].items(): assert sha(path)==digest,path
assert git('rev-parse',b['entry_commit']+':pa32/tests')==b['contract_tree']
assert git('rev-parse','HEAD:pa32/tests')==b['contract_tree']
assert not git('diff',b['entry_commit'],'--','pa32/README.md','spec.md','TESTING_AND_REFERENCES.md')
for name in git('diff','--name-only',b['entry_commit'],'--','pa32','scripts','reference-binaries').splitlines():
    assert name=='pa32/plan.md',name
for name,digest in b['binaries'].items(): assert sha(name)==digest,name
assert sha(ROOT/'dev/cppgm++')==b['binaries'][b['artifact_root']+'/after-cppgm']
assert sha(ROOT/'dev/lowiropt')==b['binaries'][b['artifact_root']+'/after-lowiropt']
plan=(ROOT/'pa32/plan.md').read_text()
for marker in ['Stage base commit: '+b['stage_base'],'Last reviewed commit: '+b['last_reviewed']]: assert marker in plan
checks=read('checks.json'); c={v['name']:v for v in checks}
for row in checks: assert sha(row['log'])==row['sha256']
for name in ['prior','file','debug-objects','objects','local','dataflow','calls','audit','trace']: assert c[name]['exit_code']==0,name
for name in ['stage','through','debug','debug-driver-o1','debug-driver-o2','debug-driver-o3']: assert c[name]['exit_code']==2,name
prior=pathlib.Path(c['prior']['log']).read_text()
assert 'ALL TESTS PASSED SUCCESSFULLY! (5178 / 5178)' in prior
assert 'File audit passed for pa32 with 4 warning(s)' in pathlib.Path(c['file']['log']).read_text()
assert 'PASS (25/25)' in pathlib.Path(c['debug-objects']['log']).read_text()
current=pathlib.Path(c['stage']['log']).read_text()
baseline=pathlib.Path(b['artifact_root']+'/baseline.log').read_text()
def failures(text): return set(re.findall(r'^(pa32/[^:]+): ERROR:',text,re.M))
old,new=failures(baseline),failures(current)
assert len(old)==41 and len(new)==29 and new < old
assert sorted(old-new)==b['resolved_failures']
assert sorted(new)==b['remaining_failures']
assert '178 / 219 TESTS PASSED' in baseline and '190 / 219 TESTS PASSED' in current
assert '5368 / 5397 TESTS PASSED' in pathlib.Path(c['through']['log']).read_text()
# Recompute every summary from all observations, including outliers and A/A.
def summary(rows,s):
    aa=[v['wall_s'] for v in rows if v['block']==0]
    assert s['AA_range_s']==[min(aa),max(aa)] and len(aa)==4
    ratios=[]
    for block in range(1,7):
        vals={k:[v for v in rows if v['block']==block and v['label']==k] for k in 'AB'}
        assert all(len(v)==2 for v in vals.values())
        ratios.append(statistics.mean(v['wall_s'] for v in vals['B'])/statistics.mean(v['wall_s'] for v in vals['A']))
    assert ratios==s['paired_ratios']
    assert statistics.median(ratios)==s['paired_ratio_median']
    assert [min(ratios),max(ratios)]==s['paired_ratio_range']
    for label in 'AB':
        vals=[v for v in rows if v['block'] and v['label']==label]
        assert statistics.median(v['wall_s'] for v in vals)==s[label]['median_s']
        assert [min(v['wall_s'] for v in vals),max(v['wall_s'] for v in vals)]==s[label]['range_s']
        assert max(v['peak_rss_kib'] for v in vals)==s[label]['peak_rss_kib']
    assert all(v['status']==0 for v in rows)
count=0
for lane in ['affected','common-o0','common-o1','selfhost']:
    d=read(lane+'.json');count+=len(d['runs'])
    for bin in d['binaries'].values(): assert sha(bin['path'])==bin['sha256']
    if lane=='selfhost':
        summary(d['runs'],d['summary'])
        for path,digest in d['inputs'].items(): assert sha(path)==digest
        assert d['images']['A']==d['images']['B']
        continue
    for name,s in d['summary'].items():
        for mode in ['compile','runtime']:
            summary([v for v in d['runs'] if v['workload']==name and v['mode']==mode],s[mode])
        images=d['images'][name]
        if lane=='affected':
            assert all(v<1 for v in s['runtime']['paired_ratios'])
            assert s['compile']['paired_ratio_median']<=d['diagnostic_targets']['compiler_ratio']
            assert s['compile']['B']['peak_rss_kib']<=1.5*s['compile']['A']['peak_rss_kib']
            assert images['B']['object_text_bytes']<=images['A']['object_text_bytes']
            for ext,key in [('.lowir','lowir_sha256'),('.cpp','main_sha256')]:
                assert sha(pathlib.Path(b['artifact_root'])/lane/(name+ext))==d['inputs'][name][key]
        else:
            assert images['A']['object_sha256']==images['B']['object_sha256']
            assert sha(pathlib.Path(b['artifact_root'])/lane/(name+'.cpp'))==d['inputs'][name]
assert count==588
assert not git('diff',b['implementation_commit'],'--','dev'), 'unmeasured implementation changes'
assert not git('status','--porcelain'), 'handoff must be committed and clean'
print('PA32 implementation 211 evidence: PASS (588 samples; 41 -> 29 unchanged-coverage failures; prior/file gates; clean handoff)')
