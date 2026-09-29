#!/usr/bin/env python3
"""Rebuild the member-copy oracle from source semantics, never student output.
Run --apply, or WORK CC to verify the revision and execute the reducer.
"""
from pathlib import Path
import hashlib, json, subprocess, sys
ROOT = Path(__file__).resolve().parents[2]
ENTRY = '16ac49da1339edaa4a5f96bf4efab731a30c65e1'
PATH = 'pa20/tests/general/200-aggregate-braced-return-copies-class-members.ref'
def sha(data): return hashlib.sha256(data).hexdigest()
def run(args): return subprocess.run([str(x) for x in args],cwd=ROOT,capture_output=True,text=True,timeout=90)
before = subprocess.check_output(['git','show',ENTRY+':'+PATH],cwd=ROOT)
old = before.decode()
# Preserve the source copy constructor and entry point. Construct each member
# at its specified destination, directly from the corresponding lvalue.
after = '''function @make_aggregate(%ret : ptr [pass=indirect_result, object_bytes=40], %first : ptr [pass=by_address, object_bytes=16], %second : ptr [pass=by_address, object_bytes=16]) -> void [binding=strong, object=_Z14make_aggregate12member_valueS_] {
  slot $first : obj<16x8>
  slot $second : obj<16x8>
  block ^entry:
    %prefix = index i8 [projection=field] %ret, 0
    store i32 1, %prefix
    %a = index i8 [projection=field] %ret, 8
    call void @member_value__member_value__ov2(%a, %first)
    %b = index i8 [projection=field] %ret, 24
    call void @member_value__member_value__ov2(%b, %second)
    return void
}
'''
after += old[old.index('function @main('):old.index('function @aggregate_value__')]
# Ordinary visible constructor definition root. Its body follows the two
# source member initializers; it is never called by make_aggregate.
after += '''function @member_value__member_value(%this : ptr [object_bytes=16], %input : i32) -> void [unwind=no, binding=weak, object=_ZN12member_valueC1Ei, inline_hint=yes] {
  slot $this : ptr
  slot $input : i32
  block ^entry:
    store ptr %this, $this
    store i32 %input, $input
    %a = load ptr $this
    %local = index i8 [projection=field] %a, 8
    %b = load ptr $this
    %pointer = index i8 [projection=field] %b, 0
    store ptr %local, %pointer
    %value = load i32 $input
    %c = load ptr $this
    %target = index i8 [projection=field] %c, 8
    store i32 %value, %target
    return void
}
alias object _ZN12member_valueC2Ei = @member_value__member_value
alias object _ZN12member_valueC2ERKS_ = @member_value__member_value__ov2
'''
after = after.encode()
manifest = dict(entry=ENTRY,bundle_revision='c2f713cd70d06170632bfde3e75dd6fe1aa44d98',
    local_revision='pa20-aggregate-member-copies-99',proof='pa20/reference-corrections99.md',
    revisions=[dict(path=PATH,before_sha256=sha(before),after_sha256=sha(after),
        source_sha256=sha((ROOT/PATH.replace('.ref','.t')).read_bytes()))])
manifest_path = ROOT/'student.tests/pa20/reference99-revisions.json'
if '--apply' in sys.argv:
    (ROOT/PATH).write_bytes(after)
    manifest_path.write_text(json.dumps(manifest,indent=2)+'\n')
else:
    assert (ROOT/PATH).read_bytes() == after
    assert json.loads(manifest_path.read_text()) == manifest
    work, cc = [Path(p).resolve() for p in sys.argv[1:]]
    work.mkdir(parents=True,exist_ok=True)
    checks = []
    for label,compiler,expected in [('bundle',ROOT/'dev/cppgm++-ref',1),('student',cc,0)]:
        src = ROOT/'student.tests/pa20/member_copies99.cpp'
        ir, exe = work/(label+'.lowir'), work/label
        compile = run([compiler,'--emit-lowir','-O0','-o',ir,src]); assert compile.returncode==0,compile.stderr
        backend = run([ROOT/'dev/lowir2native-ref','-O0','-o',exe,ir]); assert backend.returncode==0,backend.stderr
        execute = run([exe]); assert execute.returncode==expected,(label,execute.returncode)
        checks.append(dict(label=label,source_sha256=sha(src.read_bytes()),lowir_sha256=sha(ir.read_bytes()),native_exit=execute.returncode))
    for label,content in [('original',before),('corrected',after)]:
        ir,exe = work/(label+'.lowir'),work/label
        ir.write_bytes(content)
        backend = run([ROOT/'dev/lowir2native-ref','-O0','-o',exe,ir])
        # The old oracle spuriously references the undefined move constructor.
        if label=='original':
            assert backend.returncode and 'undefined native symbol:' in backend.stderr
        else:
            assert backend.returncode==0,backend.stderr
            assert run([exe]).returncode==0
        checks.append(dict(label=label,backend_exit=backend.returncode,diagnostic=backend.stderr,lowir_sha256=sha(content)))
    (work/'results.json').write_text(json.dumps(dict(manifest=manifest,checks=checks),indent=2)+'\n')
print('Verified aggregate member-copy oracle revision.')
