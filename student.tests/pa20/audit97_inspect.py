#!/usr/bin/env python3
"""Inspect current compiler containment and encoding of safe/ordered initializer paths.
Run CC WORK CONTROLS OUT. CONTROLS is the audit97_controls output directory.
"""
from pathlib import Path
import hashlib, json, struct, subprocess, sys
ROOT = Path(__file__).resolve().parents[2]
CC, WORK, CONTROLS, OUT = [Path(x).resolve() for x in sys.argv[1:]]
WORK.mkdir(parents=True, exist_ok=True)
assert not OUT.exists()
def sha(p): return hashlib.sha256(Path(p).read_bytes()).hexdigest()
def run(args):
    r = subprocess.run([str(x) for x in args],cwd=ROOT,capture_output=True,text=True,timeout=60)
    assert r.returncode == 0, (args,r.returncode,r.stderr)
    return r
result = dict(compiler_sha256=sha(CC), backend_sha256=sha(ROOT/'reference-binaries/lowir2native'), traces={})
for name, source in [('safe',ROOT/'student.tests/pa20/trace96.cpp'),
                     ('ordered',CONTROLS/'aggregate_template_order.cpp')]:
    ir, plain, native = (WORK/(name+ext) for ext in ('.lowir','.plain','.exe'))
    command = [CC,'--emit-lowir','-O0','-o',ir,source]
    r = run([*command,'--stats','--validate-lowir'])
    run([CC,'--emit-lowir','-O0','-o',plain,source])
    assert ir.read_bytes() == plain.read_bytes()
    text = ir.read_text()
    assert ('function @__aggregate_' in text) == (name == 'safe')
    run([ROOT/'dev/lowir2native-ref','-O0','-o',native,ir]); run([native])
    data = native.read_bytes()
    entry, phoff = struct.unpack_from('<QQ',data,24)
    kind, flags, offset, address, _, length, _, _ = struct.unpack_from('<IIQQQQQQ',data,phoff)
    assert kind == 1 and flags&1 and offset == 0
    payload = WORK/(name+'.payload'); payload.write_bytes(data[entry-address:length])
    disassembly = run(['objdump','-D','-b','binary','-m','i386:x86-64','--adjust-vma='+str(entry),payload]).stdout
    result['traces'][name] = dict(source=source.read_text(),source_sha256=sha(source),
        lowir=text,lowir_sha256=sha(ir),native_sha256=sha(native),native_exit=0,
        payload_bytes=payload.stat().st_size,disassembly=disassembly,
        stats=[json.loads(line) for line in r.stderr.splitlines()],instrumentation_parity=True)
    syscalls = WORK/(name+'.syscalls')
    run(['strace','-f','-e','trace=execve,openat','-o',syscalls,*command])
    trace = syscalls.read_text()
    assert trace.count('execve(') == 1 and 'reference-binaries' not in trace and '.ref"' not in trace
    result['traces'][name]['compiler_syscalls'] = trace
OUT.write_text(json.dumps(result,indent=2)+'\n')
