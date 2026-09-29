#!/usr/bin/env python3
"""Trace PA21 cross-owner facts through LowIR to the supplied ELF backend."""
from pathlib import Path
import json, struct, sys
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'pa10'))
from benchmark import run, sha
ROOT=Path(__file__).resolve().parents[2]
CC,WORK,OUT=[Path(p).resolve() for p in sys.argv[1:]]
WORK.mkdir(parents=True,exist_ok=True)
assert not OUT.exists()
src=ROOT/'student.tests/pa21/audit105_trace.cpp'
ir,plain,exe=WORK/'trace.lowir',WORK/'plain.lowir',WORK/'trace.exe'
cmd=[CC,'--emit-lowir','-O0','-o',ir,src]
stats=[json.loads(s) for s in run([*cmd,'--stats','--validate-lowir']).stderr.splitlines()]
run([CC,'--emit-lowir','-O0','-o',plain,src])
assert ir.read_bytes()==plain.read_bytes()
run([ROOT/'dev/lowir2native-ref','-O0','-o',exe,ir]);run([exe])
data=exe.read_bytes();entry,phoff=struct.unpack_from('<QQ',data,24)
kind,flags,offset,address,_,length,_,_=struct.unpack_from('<IIQQQQQQ',data,phoff)
assert kind==1 and flags&1 and offset==0
payload=WORK/'trace.payload';payload.write_bytes(data[entry-address:length])
disassembly=run(['objdump','-D','-b','binary','-m','i386:x86-64','--adjust-vma='+str(entry),payload]).stdout
syscalls=WORK/'compiler.syscalls'
run(['strace','-f','-e','trace=execve,openat','-o',syscalls,*cmd])
trace=syscalls.read_text()
assert trace.count('execve(')==1 and 'reference-binaries' not in trace and '.ref"' not in trace
OUT.write_text(json.dumps(dict(source=src.read_text(),source_sha256=sha(src),compiler_sha256=sha(CC),
    backend_sha256=sha(ROOT/'reference-binaries/lowir2native'),stats=stats,
    lowir=ir.read_text(),lowir_sha256=sha(ir),native_sha256=sha(exe),native_exit=0,
    payload_bytes=payload.stat().st_size,disassembly=disassembly,compiler_syscalls=trace,
    telemetry_output_identical=True),indent=2)+'\n')
print('PA21 source-to-ELF trace PASS')
