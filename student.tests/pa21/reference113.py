#!/usr/bin/env python3
"""Reconstruct failed-new cleanup from original reference bytes, never student IR."""
from pathlib import Path
import hashlib, json, subprocess, sys
ROOT = Path(__file__).resolve().parents[2]
BASE = 'f3af4630'
PATH = 'pa12/tests/general/300-class-new-expression-default-constructor.ref'
original = subprocess.check_output(['git', 'show', BASE+':'+PATH], cwd=ROOT).decode()
changes = [
    ('\nfunction @main()', '\ndeclare function @operatordelete(%arg0 : ptr) -> void [unwind=no, role=free_memory, binding=strong, object=cppgm_builtin_operator_delete]\n\nfunction @main()'),
    ('  slot $p : ptr\n', '  slot $p : ptr\n  slot $allocation : ptr\n'),
    ('    call void @S__S(%t2)\n', '''    store ptr %t2, $allocation
    eh_try ^release
    call void @S__S(%t2)
    eh_end
    jump ^constructed
  block ^resume:
    resume
  block ^release:
    %allocation = load ptr $allocation
    call void @operatordelete(%allocation)
    jump ^resume
  block ^constructed:
'''),
]
revised = original
for before, after in changes:
    assert revised.count(before) == 1
    revised = revised.replace(before, after)
sha = lambda data: hashlib.sha256(data.encode()).hexdigest()
manifest = dict(revision='pa21-failed-new-113', base=BASE, path=PATH,
    bundle_source='c2f713cd70d06170632bfde3e75dd6fe1aa44d98',
    bundle_sha256='c532a109ae800825da24f60ae28ea894aa4896f56efb6ebb14728cdccf25a7d7',
    original_sha256=sha(original), revised_sha256=sha(revised), replacements=changes)
if __name__ == '__main__':
    output = ROOT/'student.tests/pa21/reference113-revision.json'
    if '--check' in sys.argv:
        assert (ROOT/PATH).read_text() == revised
        assert json.loads(output.read_text()) == json.loads(json.dumps(manifest))
    else:
        (ROOT/PATH).write_text(revised)
        output.write_text(json.dumps(manifest, indent=2)+'\n')
    print(json.dumps(manifest, indent=2))
