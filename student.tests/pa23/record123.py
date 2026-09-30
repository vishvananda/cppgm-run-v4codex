#!/usr/bin/env python3
"""Seal PA23 handoff evidence, preserving failures and every contract fixture."""
from pathlib import Path
import hashlib,json,re,subprocess,sys
ROOT=Path(__file__).resolve().parents[2];ART=Path(sys.argv[1]).resolve()
ENTRY='9688f9e54a2b206b9add41ce78acd9b5f0452c62';BASE='f33dd0775073bf5db6665fb2f4f504783159df76'
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def read(name):return json.loads((ROOT/'student.tests/pa23'/name).read_text())
def fails(s):return sorted(set(re.findall(r'(pa23/tests/[^ :]+\.t): ERROR:',s)))
old=fails((ART/'stage-entry.log').read_text());new=fails((ART/'stage-final.log').read_text())
assert len(old)==25 and len(new)==21 and not set(new)-set(old)
for name,text in [('stage','24 / 45 TESTS PASSED'),('prior','ALL TESTS PASSED SUCCESSFULLY! (3811 / 3811)'),('through','3835 / 3856 TESTS PASSED'),('audit','File audit passed for pa23 with 3 warning(s).')]:assert text in (ART/(name+'-final.log')).read_text()
fixtures=sorted((ROOT/'pa23/tests/general').glob('*.t'));assert len(fixtures)==45
changed=subprocess.check_output(['git','diff','--name-only',ENTRY,'--',':(glob)pa*/tests/**',':(glob)pa*/scripts/**','scripts','reference-binaries'],cwd=ROOT,text=True).splitlines();assert changed==[],changed
for src in fixtures:assert subprocess.check_output(['git','show',ENTRY+':'+str(src.relative_to(ROOT))],cwd=ROOT)==src.read_bytes()
compiler=sha(ROOT/'dev/cppgm++');assert compiler==sha(ART/'sealed-final')
semantic=read('controls123-semantic.json');layout=read('controls123-layout.json');lifecycle=read('controls123-lifecycle.json');inherited=read('controls123-inherited.json');roundtrips=read('roundtrips123.json');performance=read('performance123.json');reference=read('reference-observation123.json')
assert len(semantic['cases'])==26 and all(r['passed'] for r in semantic['cases'])
assert len(layout['cases'])==19 and len([r for r in layout['cases'] if not r['unfinished']])==17
assert all(r['passed'] for r in layout['cases'] if not r['unfinished'])
assert [r['name'] for r in layout['cases'] if not r['passed']]==['construct-shared-once','construct-hidden-vptr-target']
assert len(lifecycle['cases'])==20 and all(r['passed'] for r in lifecycle['cases'])
assert len(inherited['cases'])==33 and [r['name'] for r in inherited['cases'] if not r['passed']]==['virtual-member-pointer']
native=read('native123.json');limit=read('backend-limit123.json')
assert len(native['cases'])==19
assert [r['name'] for r in native['cases'] if not r['passed'] and not r['unfinished']]==['shared-rtti']
assert all(r['hosted_run']['exit']==0 for r in limit['cases'])
assert limit['cases'][0]['private_run']['exit']==1 and limit['cases'][1]['standalone_run']['exit']==1
assert len(roundtrips['cases'])==44 and all(r['stable'] for r in roundtrips['cases'])
assert all(x['compiler_sha256']==compiler for x in (semantic,layout,lifecycle,inherited))
assert performance['binaries'][1]['sha256']==compiler and performance['binaries'][0]['sha256']==sha(ART/'entry')
assert len(performance['workloads'])==11 and all(o['checked_exit']==0 for w in performance['workloads'].values() for o in w['outputs'])
repeat=read('performance123-repeat.json')
assert repeat['binaries']==performance['binaries']
assert repeat['workloads']['member-functions-2048']['source_sha256']==performance['workloads']['member-functions-2048']['source_sha256']
assert all(w['identical_text'] for n,w in performance['workloads'].items() if not w['final_only'] and n not in ('runtime-virtual-single','runtime-nonpoly-single'))
assert reference['reference_exit']==1 and reference['expected_source_exit']==0 and reference['oracle_changes']==[]
plan=(ROOT/'pa23/plan.md').read_text()
for marker in ('Stage base commit','Last reviewed commit'):assert marker+': `'+BASE+'`' in plan
result=dict(stage_base=BASE,last_reviewed_commit=BASE,entry_commit=ENTRY,implementation_commit='6a762f25',compiler_sha256=compiler,entry_binary_sha256=sha(ART/'entry'),baseline_log_sha256=sha(ART/'stage-entry.log'),baseline_passed=20,baseline_failures=old,stage_passed=24,stage_total=45,stage_failures=new,resolved_failures=sorted(set(old)-set(new)),new_failures=[],stage_progress='pass: four original failures resolved without coverage reduction',prior_passed=3811,prior_total=3811,prior_stages=22,through_stage_passed=3835,through_stage_total=3856,file_audit='pass; three inherited header organization warnings',semantic_passed=26,semantic_total=26,completed_layout_passed=17,completed_layout_total=17,layout_probes_total=19,unfinished_layout=['construct-shared-once','construct-hidden-vptr-target'],lifecycle_passed=20,lifecycle_total=20,inherited_passed=32,inherited_total=33,unfinished_inherited=['virtual-member-pointer'],stable_roundtrips=44,standalone_completed_passed=16,standalone_completed_total=17,standalone_limit='student.tests/pa23/backend-limit123.json',independent_review='pending; handoff is incomplete implementation, not stage certification',contract_changes=changed,reference_observation='pa23/reference-observation123.md',compiler_build_config=(ROOT/'obj/dev/.compile_config').read_text(),checks=[])
for label,command,code in [('stage','make test-pa23',2),('prior',"n=23; if [ \"$n\" -le 1 ]; then echo '===== ALL TESTS PASSED SUCCESSFULLY! (0/0) ====='; else make test-report-through-pa$((n - 1)); fi",0),('through','make test-report-through-pa23',2),('audit','perl scripts/cppgm_file_audit.pl --stage pa23 --paths dev/src',0)]:
 log=ART/(label+'-final.log');result['checks'].append(dict(command=command,observed_exit=code,log=str(log),log_sha256=sha(log)))
result['fixture_manifest']=[dict(path=str(p.relative_to(ROOT)),source_sha256=sha(p),reference_sha256=sha(p.with_suffix('.ref')),expected_status=p.with_suffix('.ref.exit_status').read_text().strip(),observed_status=p.with_suffix('.my.exit_status').read_text().strip()) for p in fixtures]
result['evidence_files']={str(p.relative_to(ROOT)):sha(p) for p in sorted((ROOT/'student.tests/pa23').glob('*123*.json')) if p.name!='validation123.json'}
(ROOT/'student.tests/pa23/validation123.json').write_text(json.dumps(result,indent=2)+'\n')
print('PA23: 20 -> 24 / 45, no new failures; earlier 3811/3811; audit pass; semantic 26/26; layout 17/17 plus 2 retained lifecycle failures; prior lifecycle 20/20; inherited 32/33; 44 stable roundtrips')
