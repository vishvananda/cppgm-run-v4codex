#!/usr/bin/env python3
"""Reconstruct the one defaulted-pack oracle correction from the stage base."""
from pathlib import Path
import hashlib, json, subprocess, sys
ROOT=Path(__file__).resolve().parents[2]
BASE='e5f4c3ed78972c8d161671d145bf525cb99033f4'
PATH='pa19/tests/spec/200-defaulted-class-template-argument-pack-prefix-deduction.ref'
old=subprocess.check_output(['git','show',BASE+':'+PATH],cwd=ROOT).decode()
before='_Z4takeIiJiiEEiRKN6tuples5tupleIT_T0_NS0_9null_typeES4_S4_S4_S4_S4_S4_S4_EE'
after='_Z4takeIiJiiN6tuples9null_typeES1_S1_S1_S1_S1_S1_EEiRKNS0_5tupleIT_DpT0_EE'
assert old.count('    %t1 = const i64 2\n')==1 and old.count(before)==1
new=old.replace('    %t1 = const i64 2\n','    %t1 = const i64 9\n').replace(before,after)
sha=lambda s:hashlib.sha256(s).hexdigest()
record=dict(base_commit=BASE,bundle_revision='c2f713cd70d06170632bfde3e75dd6fe1aa44d98',
    local_revision='pa19-defaulted-pack-91',path=PATH,before_sha256=sha(old.encode()),after_sha256=sha(new.encode()),
    source_sha256=sha((ROOT/PATH).with_suffix('.t').read_bytes()),
    changes=['pack cardinality 2 -> 9','ABI template argument pack and dependent expansion encoding'],
    proof='pa19/reference-correction91.md')
manifest=ROOT/'student.tests/pa19/reference91-revisions.json'
if '--write' in sys.argv:
    (ROOT/PATH).write_text(new);manifest.write_text(json.dumps(record,indent=2)+'\n')
else:
    assert (ROOT/PATH).read_text()==new
    assert json.loads(manifest.read_text())==record
    print('PA19 defaulted-pack oracle reconstruction: PASS')
