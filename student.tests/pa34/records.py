#!/usr/bin/env python3
"""Bind completed canonical checks, exact objects, sources and retained evidence."""
import hashlib
import json
import pathlib
import subprocess
import sys

root = pathlib.Path(__file__).resolve().parents[2]
art = pathlib.Path(sys.argv[1]).resolve()
target = pathlib.Path(sys.argv[2]).resolve()
suffix = sys.argv[3] if len(sys.argv) > 3 else 'dispatch'
def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()
def run(args, cwd=root):
    return subprocess.check_output(args, cwd=cwd, text=True).strip()
checks = {
    'fileAudit': ('audit-order.log', 'File audit passed for pa34'),
    'hostThrough33': ('host-through33-order.log', 'ALL TESTS PASSED SUCCESSFULLY! (5454 / 5454)'),
    'selfThroughPa5': ('self-through5-order.log', 'pa5 tests: PASS (188/188)'),
    'selfThroughPa8': ('self-through8-order.log', 'pa8 tests/behavior: PASS (3/3)'),
    'selfThroughPa33': ('self-through33-order.log', 'PA24 native contract properties: PASS (5/5)'),
    'pptokenInception': ('pptoken-inception-order.log', 'MATCH pptoken:'),
    'inception': ('inception-order.log', 'MATCH cppgm++:'),
}
result = dict(implementation_commit=run(['git','rev-parse','HEAD']), checks={}, objects={}, binaries={}, sources={}, artifacts={})
for name,(file,marker) in checks.items():
    path = art/file.replace('-order.', '-'+suffix+'.')
    assert marker in path.read_text(), (name, marker)
    result['checks'][name] = dict(log=str(path), sha256=sha(path), success_marker=marker)
inputs = run(['make','-s','--no-print-directory','--eval',
              'print-inputs: ; @echo $(call inception_link_inputs,cppgm++)','print-inputs'], root/'pa34').split()
for file in inputs:
    a = (root/'pa34'/file).resolve()
    b = pathlib.Path(str(a).replace('/obj/pa34/selfhost/', '/obj/pa34/inception/'))
    assert a != b and a.read_bytes() == b.read_bytes(), (a,b)
    result['objects'][str(a.relative_to(root/'obj/pa34/selfhost'))] = sha(a)
for tool in ('cppgm++', 'pptoken'):
    a, b = root/'pa34'/(tool+'-self'), root/'pa34'/(tool+'-inception')
    assert a.read_bytes() == b.read_bytes(), tool
for file in ('dev/cppgm++','pa34/cppgm++-self','pa34/cppgm++-inception','pa34/pptoken-self','pa34/pptoken-inception'):
    result['binaries'][file] = dict(sha256=sha(root/file), bytes=(root/file).stat().st_size)
files = set(run(['git','ls-files','--','dev','pa34','student.tests/pa34','AGENTS.md','spec.md','TESTING_AND_REFERENCES.md']).splitlines())
files.update(run(['git','diff','--name-only','99ea7d9d','--','student.tests','pa29/tests/preproc/300-has-builtin.ref']).splitlines())
files.update(('student.tests/pa32/selfhost_performance.py', 'student.tests/pa32/common_levels.py',
              'student.tests/pa33/audit-trace.cpp', 'student.tests/pa26/evidence144/common-performance.json'))
for file in sorted(files):
    p = root/file
    if p.is_file() and p.resolve() != target:
        result['sources'][file] = sha(p)
for file in ('obj/dev/generated/builtin_host_config.h','obj/pa34/generated/builtin_host_config.h'):
    result['sources'][file] = sha(root/file)
assert result['sources']['obj/dev/generated/builtin_host_config.h'] == result['sources']['obj/pa34/generated/builtin_host_config.h']
for file in sorted(art.rglob('*')):
    if file.is_file() and file.resolve() != target:
        result['artifacts'][str(file.relative_to(art))] = dict(sha256=sha(file), bytes=file.stat().st_size)
result['host'] = run(['g++','--version'])
target.parent.mkdir(parents=True, exist_ok=True)
target.write_text(json.dumps(result, indent=2)+'\n')
print(f"canonical cppgm++/pptoken binaries and {len(inputs)} object pairs match; evidence bound")
