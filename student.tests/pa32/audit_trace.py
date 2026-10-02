#!/usr/bin/env python3
"""Run the source/template-to-ELF trace and retain the actual consumed views."""
import hashlib
import json
import pathlib
import subprocess
import sys

ROOT = pathlib.Path(__file__).resolve().parents[2]
OUT = pathlib.Path(sys.argv[1]).resolve()
OUT.mkdir(parents=True, exist_ok=True)
SRC = ROOT / 'student.tests/pa32/audit-trace.cpp'
records = []


def run(args):
    command = list(map(str, args))
    result = subprocess.run(command, capture_output=True, text=True, timeout=60, cwd=ROOT)
    records.append(dict(command=command, exit_code=result.returncode,
                        stdout=result.stdout, stderr=result.stderr))
    (OUT / 'trace.json').write_text(json.dumps(records, indent=2) + '\n')
    assert result.returncode == 0, (command, result.stderr[-1000:])
    return result


for level in range(4):
    ir, direct, replay = (OUT / f'trace{level}{suffix}' for suffix in ['.lowir', '.o', '-replay.o'])
    run(['dev/cppgm++', '--emit-lowir', '--validate-lowir', '-gline-tables-only', f'-O{level}',
         '--stats', '-o', ir, SRC])
    run(['dev/cppgm++', '-c', '-gline-tables-only', f'-O{level}', '-o', direct, SRC])
    run(['dev/cppgm++', '-c', f'-O{level}', '-o', replay, OUT / 'trace0.lowir'])
    assert direct.read_bytes() == replay.read_bytes(), (level, 'object replay mismatch')
    run(['g++', direct, '-o', OUT / 'trace-exe'])
    run([OUT / 'trace-exe'])
    run([OUT / 'trace-exe', 'argument'])
    run(['readelf', '-SW', direct])
    run(['readelf', '-Ws', direct])
    # Record the inherited ELF surface as well: this backend carries locations
    # in MIR but does not yet emit DWARF line sections. PA32 checks LowIR/MIR
    # locations and direct/replayed object equivalence, not DWARF generation.
    run(['readelf', '--debug-dump=decodedline', direct])
    # The MIR view comes from the same selector consumed by native encoding;
    # it is inspection, never production phase transport.
    run(['dev/lowir2native', '--dump-machine-ir', OUT / f'trace{level}.mir', '--stats', ir])
    assert 'audit-trace.cpp' in (OUT / f'trace{level}.mir').read_text()
    run(['objdump', '-d', direct])
(OUT / 'source.sha256').write_text(hashlib.sha256(SRC.read_bytes()).hexdigest() + '\n')
print('PA32 architecture trace: PASS (4 levels; validated typed LowIR, direct/replay ELF equality, '
      'two checked runtime inputs, MIR debug locations and ELF inspection)')
