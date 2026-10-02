#!/usr/bin/env python3
"""Verify this implementation handoff's evidence, never whole-stage completion."""
import hashlib,json,pathlib,re,statistics,subprocess
root=pathlib.Path(__file__).resolve().parents[2]
e=root/'student.tests/pa32/evidence208'
def sha(p): return hashlib.sha256(pathlib.Path(p).read_bytes()).hexdigest()
b=json.loads((e/'binding.json').read_text())
for p,h in b['source_hashes'].items(): assert sha(root/p)==h,p
for v in b['binaries'].values(): assert sha(v['path'])==v['sha256'],v['path']
assert sha(root/'dev/cppgm++')==b['binaries']['selected-cppgm']['sha256']
assert sha(root/'dev/lowiropt')==b['binaries']['selected-opt']['sha256']
checks={c['name']:c for c in b['checks']}
for check in checks.values(): assert sha(check['log'])==check['sha256'],check['name']
for n in ['prior','audit','dataflow','local','replay-debug','replay-nodebug']:
    assert checks[n]['exit_code']==0,n
assert '5178 / 5178' in pathlib.Path(checks['prior']['log']).read_text()
assert '134 execution cases' in pathlib.Path(checks['dataflow']['log']).read_text()
assert '506 execution cases' in pathlib.Path(checks['local']['log']).read_text()
for n in ['replay-debug','replay-nodebug']: assert 'PASS (25/25)' in pathlib.Path(checks[n]['log']).read_text()
assert checks['stage']['exit_code']==2
stage=pathlib.Path(checks['stage']['log']).read_text()
assert '127 / 219' in stage
failures=set(re.findall(r'^(pa32/[^:]+): ERROR',stage,re.M))
entry=pathlib.Path(checks['entry-stage']['log']).read_text()
old=set(re.findall(r'^(pa32/[^:]+): ERROR',entry,re.M))
assert len(old)==109 and len(failures)==92 and failures < old
assert sorted(old-failures)==b['stage']['fixed'] and not failures-old
# Compare actual tracked contract contents to the entry commit, not just counts.
assert not subprocess.check_output(['git','diff',b['entry_commit'],'--','pa32/tests','pa32/scripts','pa32/Makefile','scripts'],cwd=root)
for n in ['Stage base commit','Last reviewed commit']:
    assert n+': '+b['stage_base_commit'] in (root/'pa32/plan.md').read_text()
observations=0
for name,h in b['performance'].items():
    path=e/name; assert sha(path)==h,name
    r=json.loads(path.read_text());observations+=len(r['runs'])
    assert all(v.get('status',0)==0 for v in r['runs']),name
    frozen=r.get('binaries',{'compiler':r.get('binary')})
    for binary in frozen.values():
        if binary: assert sha(binary['path'])==binary['sha256'],binary['path']
    def inspect(node,axes=()):
        if 'paired_ratios' not in node:
            for k,v in node.items():
                if isinstance(v,dict): inspect(v,axes+(k,))
            return
        rows=r['runs']
        for axis in axes:
            field='mode' if axis in ('compile','runtime') else 'workload'
            rows=[v for v in rows if v.get(field)==axis]
        assert len(rows)==28,(name,axes,len(rows))
        aa=[v['wall_s'] for v in rows if not v['block']]
        assert len(aa)==4 and node['AA_range_s']==[min(aa),max(aa)]
        ratios=[]
        for block in range(1,7):
            a=[v['wall_s'] for v in rows if v['block']==block and v['label']=='A']
            c=[v['wall_s'] for v in rows if v['block']==block and v['label']=='B']
            assert len(a)==len(c)==2
            ratios.append(statistics.mean(c)/statistics.mean(a))
        assert ratios==node['paired_ratios']
        assert statistics.median(ratios)==node['paired_ratio_median']
        assert [min(ratios),max(ratios)]==node['paired_ratio_range']
    inspect(r['summary'])
for name in ['affected-selected.json','scalar-selected.json']:
    r=json.loads((e/name).read_text())
    families=r['summary'] if name.startswith('affected') else {'scalar':r['summary']}
    images=r['images'] if name.startswith('affected') else {'scalar':r['images']}
    for workload,metrics in families.items():
        assert metrics['compile']['paired_ratio_median']<=2.0
        assert metrics['compile']['B']['peak_rss_kib']<=1.75*metrics['compile']['A']['peak_rss_kib']
        assert max(metrics['runtime']['paired_ratios'])<1.0
        assert images[workload]['B']['text_bytes']<=images[workload]['A']['text_bytes']
for level in ['o0','o1']:
    r=json.loads((e/('common-selected-'+level+'.json')).read_text())
    for name,images in r['images'].items():
        if level=='o0' or name!='exceptions': assert images['A']['object_sha256']==images['B']['object_sha256']
        assert images['B']['executable_text_bytes']<=images['A']['executable_text_bytes']
        metrics=r['summary'][name]['compile']
        assert metrics['paired_ratio_median']<=2.0
        assert metrics['B']['peak_rss_kib']<=1.75*metrics['A']['peak_rss_kib']
selfhost=json.loads((e/'selfhost-selected.json').read_text())
assert selfhost['images']['A']==selfhost['images']['B']
assert checks['debug-required']['exit_code']==checks['debug-driver']['exit_code']==2
assert 'PASS (3/3)' in pathlib.Path(checks['debug-required']['log']).read_text()
assert 'PASS (1/1)' in pathlib.Path(checks['debug-required']['log']).read_text()
assert 'FAIL (0/3)' in pathlib.Path(checks['debug-driver']['log']).read_text()
print(json.dumps(dict(handoff_checks_pass=True,stage_complete=False,observations=observations,
                     fixed_failures=len(old-failures),remaining_failures=len(failures))))
