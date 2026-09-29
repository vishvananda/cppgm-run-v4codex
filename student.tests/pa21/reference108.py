#!/usr/bin/env python3
"""Reconstruct the aggregate-prefix correction independently of student output."""
from pathlib import Path
import hashlib,json,subprocess,sys
ROOT=Path(__file__).resolve().parents[2]
BASE='ac988ea33d4997b44e82baaca5a86623fff3127a'
PATH='pa21/tests/general/200-indirect-param-prologue-copy.ref'
original=subprocess.check_output(['git','show',BASE+':'+PATH],cwd=ROOT,text=True)
replacements=[
('  slot $e : obj<3x1>\n','  slot $e : obj<3x1>\n  slot $constructed_a : ptr\n  slot $constructed_b : ptr\n'),
('    eh_try ^cleanup_action_6\n    %t7 = addr $e\n    %t8 = index i8 [projection=field] %t7, 0\n    %t9 = addr $x\n    call void @S__S__ov2(%t8, %t9)',
 '    %t7 = addr $e\n    %t8 = index i8 [projection=field] %t7, 0\n    %t9 = addr $x\n    eh_try ^cleanup_action_6\n    call void @S__S__ov2(%t8, %t9)'),
('''  block ^call_unwind_end_7:
    eh_try ^cleanup_action_6
    %t11 = addr $e
    %t12 = index i8 [projection=field] %t11, 1
    %t13 = addr $y
    call void @S__S__ov2(%t12, %t13)
    eh_end
    eh_try ^cleanup_action_6
    %t14 = addr $e
    %t15 = index i8 [projection=field] %t14, 2
    %t16 = addr $z
    call void @S__S__ov2(%t15, %t16)
    eh_end
    %t17 = addr $e''',
 '''  block ^call_unwind_end_7:
    store ptr %t8, $constructed_a
    %t11 = addr $e
    %t12 = index i8 [projection=field] %t11, 1
    %t13 = addr $y
    eh_try ^constructed_a_cleanup
    call void @S__S__ov2(%t12, %t13)
    eh_end
    jump ^constructed_b_ready

  block ^constructed_a_cleanup:
    %partial_a = load ptr $constructed_a
    call void @S___S(%partial_a)
    jump ^cleanup_action_6

  block ^constructed_b_ready:
    store ptr %t12, $constructed_b
    %t14 = addr $e
    %t15 = index i8 [projection=field] %t14, 2
    %t16 = addr $z
    eh_try ^constructed_b_cleanup
    call void @S__S__ov2(%t15, %t16)
    eh_end
    jump ^constructed_c_ready

  block ^constructed_b_cleanup:
    %partial_b = load ptr $constructed_b
    call void @S___S(%partial_b)
    jump ^constructed_a_cleanup

  block ^constructed_c_ready:
    %t17 = addr $e''')]
revised=original
for old,new in replacements:
 assert revised.count(old)==1
 revised=revised.replace(old,new)
sha=lambda s:hashlib.sha256(s.encode()).hexdigest()
manifest=dict(revision='pa21-aggregate-prefix-108',base=BASE,
 bundle_source='c2f713cd70d06170632bfde3e75dd6fe1aa44d98',
 bundle_sha256='c532a109ae800825da24f60ae28ea894aa4896f56efb6ebb14728cdccf25a7d7',
 path=PATH,original_sha256=sha(original),revised_sha256=sha(revised),replacements=replacements,
 proof='pa21/reference-corrections108.md')
if '--write' in sys.argv:
 (ROOT/PATH).write_text(revised)
 (ROOT/'student.tests/pa21/reference108-revision.json').write_text(json.dumps(manifest,indent=2)+'\n')
else:
 assert (ROOT/PATH).read_text()==revised
 assert json.loads((ROOT/'student.tests/pa21/reference108-revision.json').read_text())==json.loads(json.dumps(manifest))
 print(json.dumps(manifest,indent=2))
