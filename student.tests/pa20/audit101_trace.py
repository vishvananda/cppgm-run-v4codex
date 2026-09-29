#!/usr/bin/env python3
"""Trace captured ranges through typed facts, LowIR, native bytes and syscalls."""
from pathlib import Path
import hashlib, json, struct, subprocess, sys
ROOT = Path(__file__).resolve().parents[2]
CC, WORK, OUT = [Path(x).resolve() for x in sys.argv[1:]]
WORK.mkdir(parents=True, exist_ok=True)
assert not OUT.exists()
def sha(p): return hashlib.sha256(p.read_bytes()).hexdigest()
def run(args):
    r = subprocess.run([str(x) for x in args], cwd=ROOT, capture_output=True, text=True, timeout=60)
    assert r.returncode == 0, (args, r.returncode, r.stderr)
    return r
source = ROOT/'student.tests/pa20/audit101_trace.cpp'
ir, ordinary, exe = WORK/'trace.lowir', WORK/'ordinary.lowir', WORK/'trace.exe'
command = [CC, '--emit-lowir', '-O0', '-o', ir, source]
r = run([*command, '--stats', '--validate-lowir'])
run([CC, '--emit-lowir', '-O0', '-o', ordinary, source])
assert ir.read_bytes() == ordinary.read_bytes()
stats = [json.loads(l) for l in r.stderr.splitlines()]
semantic = next(s for s in stats if 'template_body_transitions' in s)
assert semantic['template_body_transitions'] == 2, semantic
run([ROOT/'dev/lowir2native-ref', '-O0', '-o', exe, ir]); run([exe])
data = exe.read_bytes()
entry, phoff = struct.unpack_from('<QQ', data, 24)
kind, flags, offset, address, _, length, _, _ = struct.unpack_from('<IIQQQQQQ', data, phoff)
assert kind == 1 and flags&1 and offset == 0
payload = WORK/'trace.payload'; payload.write_bytes(data[entry-address:length])
disassembly = run(['objdump', '-D', '-b', 'binary', '-m', 'i386:x86-64', '--adjust-vma='+str(entry), payload]).stdout
syscalls = WORK/'compiler.syscalls'
run(['strace', '-f', '-e', 'trace=execve,openat', '-o', syscalls, *command])
trace = syscalls.read_text()
assert trace.count('execve(') == 1 and 'reference-binaries' not in trace and '.ref"' not in trace
OUT.write_text(json.dumps(dict(source=source.read_text(), source_sha256=sha(source),
    compiler_sha256=sha(CC), backend_sha256=sha(ROOT/'reference-binaries/lowir2native'),
    stats=stats, lowir=ir.read_text(), lowir_sha256=sha(ir), native_sha256=sha(exe),
    native_exit=0, payload_bytes=payload.stat().st_size, disassembly=disassembly,
    compiler_syscalls=trace, telemetry_output_identical=True,
    checks='two specializations reused by three calls; nested capture forwarding, hidden range references, member endpoint return deduction, mutation and destruction'), indent=2)+'\n')
print('captured range source-to-native trace PASS')
