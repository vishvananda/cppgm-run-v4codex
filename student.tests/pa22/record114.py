#!/usr/bin/env python3
"""Record the unchanged PA22 contract and the implementation handoff gates."""
from pathlib import Path
import hashlib,json,re,subprocess
ROOT=Path(__file__).resolve().parents[2];WORK=Path('/tmp/pa22-114')
BASE='a8482d768bd2dcede42ea63ef39e39cf3245c380'
initial=Path('/home/vishvananda/work/.ralph/v4codex-gpt-6-astra-xhigh/last-test.log').read_text()
stage=(WORK/'stage-final.log').read_text();prior=(WORK/'prior-final.log').read_text();through=(WORK/'through22.log').read_text()
def failures(log):return set(re.findall(r'pa22/(tests/[^:]+\.t): ERROR:',log))
old,new=failures(initial),failures(stage)
assert len(old)==77 and len(new)==52 and new<=old
assert '47 / 99 TESTS PASSED' in stage and '3712 / 3712' in prior and '3759 / 3811' in through
paths=subprocess.check_output(['git','ls-files','pa22/tests'],cwd=ROOT,text=True).splitlines();manifest=[]
for path in paths:
 data=(ROOT/path).read_bytes();before=subprocess.check_output(['git','show',BASE+':'+path],cwd=ROOT)
 assert data==before,path
 manifest.append(dict(path=path,sha256=hashlib.sha256(data).hexdigest()))
assert sum(p['path'].endswith('.t') for p in manifest)==99
assert not subprocess.check_output(['git','diff','--name-only',BASE,'--','scripts','shared','pa22/Makefile','pa22/scripts'],cwd=ROOT,text=True).strip()
audit=subprocess.run(['perl','scripts/cppgm_file_audit.pl','--stage','pa22','--paths','dev/src'],cwd=ROOT,capture_output=True,text=True);assert not audit.returncode
controls=json.loads((ROOT/'student.tests/pa22/controls114.json').read_text());entry=json.loads((ROOT/'student.tests/pa22/controls114-entry.json').read_text())
assert len(controls)==len(entry)==16 and all(r['passed'] for r in controls)
assert (ROOT/'dev/cppgm++').read_bytes()==(WORK/'compiler-final').read_bytes()
r=dict(stage_base=BASE,implementation_commit=subprocess.check_output(['git','rev-parse','HEAD'],text=True).strip(),
 entry_continuation_audit='No PA22 implementation progress or live process was found from the interrupted entry; baseline revalidated and work resumed',
 priorThroughTests=dict(command='make test-report-through-pa21',exit=0,passed=3712,total=3712,log_sha256=hashlib.sha256(prior.encode()).hexdigest()),
 stageTests=dict(command='make test-pa22',exit=2,passed=47,total=99,log_sha256=hashlib.sha256(stage.encode()).hexdigest()),
 through22=dict(command='make test-report-through-pa22',exit=2,passed=3759,total=3811,log_sha256=hashlib.sha256(through.encode()).hexdigest()),
 stageProgress=dict(entry_passed=22,entry_failed=77,final_passed=47,final_failed=52,unchanged_cases=99,improved=sorted(old-new),regressed=sorted(new-old),criterion_met=True),
 fileAudit=dict(command='perl scripts/cppgm_file_audit.pl --stage pa22 --paths dev/src',exit=audit.returncode,output=audit.stdout+audit.stderr),
 personal=dict(entry_passed=sum(r['passed'] for r in entry),final_passed=16,total=16),remaining_failures=sorted(new),contract_manifest=manifest)
(ROOT/'student.tests/pa22/validation114.json').write_text(json.dumps(r,indent=2)+'\n')
print('47/99; 25 existing failures repaired; no regressions; prior 3712/3712; file audit passed; unchanged 99-case contract')
