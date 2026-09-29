#!/usr/bin/env python3
"""Run the PA20 source/fact/LowIR/native trace: COMPILER WORK OUTPUT.json."""
from pathlib import Path
import hashlib, json, subprocess, sys
ROOT = Path(__file__).resolve().parents[2]
CC, WORK, OUT = [Path(p).resolve() for p in sys.argv[1:]]
WORK.mkdir(parents=True, exist_ok=True)
SRC = ROOT/'student.tests/pa20/trace94.cpp'
def sha(p): return hashlib.sha256(Path(p).read_bytes()).hexdigest()
commands = []
def run(command):
    command = [str(x) for x in command]
    p = subprocess.run(command, cwd=ROOT, capture_output=True, text=True, timeout=60)
    commands.append(dict(command=command, exit=p.returncode, stdout=p.stdout, stderr=p.stderr))
    assert p.returncode == 0, commands[-1]
    return p
ir, plain, exe = WORK/'trace.lowir', WORK/'plain.lowir', WORK/'trace'
stats = run([CC, '--emit-lowir', '-O0', '--stats', '--validate-lowir', '-o', ir, SRC])
run([CC, '--emit-lowir', '-O0', '-o', plain, SRC])
assert ir.read_bytes() == plain.read_bytes(), 'Instrumentation changed LowIR'
run([ROOT/'dev/lowir2native-ref', '-O0', '-o', exe, ir])
run([exe])
OUT.write_text(json.dumps(dict(source_sha256=sha(SRC), compiler_sha256=sha(CC),
    backend_sha256=sha(ROOT/'reference-binaries/lowir2native'),
    ir_sha256=sha(ir), native_sha256=sha(exe), native_exit=0,
    instrumented_and_plain_ir_equal=True,
    telemetry=[json.loads(line) for line in stats.stderr.splitlines()],
    lowir=ir.read_text(), commands=commands), indent=2)+'\n')
print('PA20 source/fact/LowIR/native trace passed')
