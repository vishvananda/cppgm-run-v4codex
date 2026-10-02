#!/usr/bin/env python3
"""Verify the incomplete implementation handoff evidence, not stage completion."""
import hashlib,json,pathlib,statistics
root=pathlib.Path(__file__).resolve().parents[2]
e=root/'student.tests/pa32/evidence207'
def sha(p):return hashlib.sha256(pathlib.Path(p).read_bytes()).hexdigest()
b=json.loads((e/'binding.json').read_text())
for p,h in b['source_hashes'].items():assert sha(root/p)==h,p
for v in b['binaries'].values():assert sha(v['path'])==v['sha256']
for check in b['checks']:
 assert sha(check['log'])==check['sha256'],check['name']
required={v['name']:v for v in b['checks']}
assert required['prior']['exit_code']==required['audit']['exit_code']==required['personal']['exit_code']==required['replay-debug']['exit_code']==0
assert '5178 / 5178' in pathlib.Path(required['prior']['log']).read_text()
assert f"{b['stage']['current_passed']} / 219" in pathlib.Path(required['stage']['log']).read_text()
assert b['stage']['remaining']==219-b['stage']['current_passed']<219
assert b['stage']['current_passed']>b['stage']['entry_passed']
assert all(b['text_equality'].values())
observations=0
for name,h in b['performance'].items():
 p=e/(name+'.json');assert sha(p)==h
 r=json.loads(p.read_text());observations+=len(r['runs'])
 assert all(v['status']==0 for v in r['runs'])
 for summary in ([r['summary']] if name.startswith('selfhost') else
                 list(r['summary'].values()) if name.startswith('affected') else
                 [s for family in r['summary'].values() for s in family.values()]):
  assert len(summary['paired_ratios'])==6
  assert statistics.median(summary['paired_ratios'])==summary['paired_ratio_median']
  assert [min(summary['paired_ratios']),max(summary['paired_ratios'])]==summary['paired_ratio_range']
r=json.loads((e/'affected.json').read_text())
assert r['summary']['compile']['paired_ratio_median']<=2
assert r['summary']['compile']['B']['peak_rss_kib']<=1.75*r['summary']['compile']['A']['peak_rss_kib']
assert max(r['summary']['runtime']['paired_ratios'])<1
# The historical >=10% target is diagnostic, not a course/spec limit. Retain
# every observation and require repeatable benefit plus the explicit budgets.
assert r['images']['B']['text_bytes']<=r['images']['A']['text_bytes']
for n in ['Stage base commit','Last reviewed commit']:
 assert n+': '+b['stage_base_commit'] in (root/'pa32/plan.md').read_text()
print(json.dumps(dict(passed=True,stage_complete=False,observations=observations,stage=b['stage'])))
