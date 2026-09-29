#!/usr/bin/env python3
"""Reconstruct the active-handler lifetime correction without student output."""
from pathlib import Path
import hashlib,json,subprocess,sys
ROOT=Path(__file__).resolve().parents[2]
BASE='d9528e585fd2823d25f9f3a3abaa8f0755f5832a'
PATH='pa21/tests/general/400-handler-context-cleanup-continuation.ref'
original=subprocess.check_output(['git','show',BASE+':'+PATH],cwd=ROOT,text=True)
old='''  block ^cleanup_action_22:
    call void @Value___Value(%t12)
    jump ^cleanup_resume_19

  block ^call_unwind_dispatch_23:
    eh_end
    call void @__external_runtime____cxa_end_catch()
    eh_end
    jump ^cleanup_action_22'''
new='''  block ^cleanup_action_22:
    call void @Value___Value(%t12)
    jump ^call_unwind_dispatch_20

  block ^call_unwind_dispatch_23:
    jump ^cleanup_action_22'''
assert original.count(old)==1
revised=original.replace(old,new)
sha=lambda s:hashlib.sha256(s.encode()).hexdigest()
manifest=dict(revision='pa21-active-handler-lifetime-110',base=BASE,
 bundle_source='c2f713cd70d06170632bfde3e75dd6fe1aa44d98',
 bundle_sha256='c532a109ae800825da24f60ae28ea894aa4896f56efb6ebb14728cdccf25a7d7',
 path=PATH,original_sha256=sha(original),revised_sha256=sha(revised),replacements=[[old,new]],
 proof='pa21/reference-corrections110.md')
if '--write' in sys.argv:
 (ROOT/PATH).write_text(revised)
 (ROOT/'student.tests/pa21/reference110-revision.json').write_text(json.dumps(manifest,indent=2)+'\n')
elif '--check' in sys.argv:
 assert (ROOT/PATH).read_text()==revised
 assert json.loads((ROOT/'student.tests/pa21/reference110-revision.json').read_text())==manifest
 print('reference 110 reconstruction passes')
elif __name__=='__main__':
 print(json.dumps(manifest,indent=2))
