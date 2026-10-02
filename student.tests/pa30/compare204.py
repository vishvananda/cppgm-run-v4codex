#!/usr/bin/env python3
"""Check semantic ELF equality across the hosted LowIR adapter boundary."""
import json
import pathlib
import re
import subprocess
import sys

root = pathlib.Path(__file__).resolve().parents[2]
scratch = pathlib.Path(sys.argv[1])
out = root / 'student.tests/pa30/evidence204'
trace = json.loads((out / 'inspection.json').read_text())
record = dict(commands=[], images={})
for name, view in trace['views'].items():
    checks = dict(text_and_named_relocations=
                  view['direct']['disassembly'].splitlines()[3:] ==
                  view['rebuilt']['disassembly'].splitlines()[3:],
                  cfi=view['direct']['frames'] == view['rebuilt']['frames'])
    symbols = {}
    relocations = {}
    for label, suffix in [('direct', '.o'), ('rebuilt', '.rebuilt.o')]:
        symbols[label] = sorted(re.sub(r'^\s*\d+:\s*', '', s)
                                for s in view[label]['symbols'].splitlines()
                                if re.match(r'^\s*\d+:', s))
        args = ['readelf', '-Wr', str(scratch / (name + suffix))]
        p = subprocess.run(args, capture_output=True, text=True, check=True)
        record['commands'].append(dict(args=args, status=p.returncode,
                                        stdout=p.stdout, stderr=p.stderr))
        # Keep section, offset, relocation type, symbol value/name and addend.
        # ELF symbol-table ordinals are incidental presentation, not identity.
        relocations[label] = re.sub(r'(?m)^(\s*[0-9a-f]{16}\s+)[0-9a-f]{16}(\s+R_)',
                                    r'\1<symbol-index>\2', p.stdout)
    checks['symbol_facts'] = symbols['direct'] == symbols['rebuilt']
    checks['all_named_relocations'] = relocations['direct'] == relocations['rebuilt']
    assert all(checks.values()), (name, checks)
    record['images'][name] = checks
(out / 'elf-equivalence.json').write_text(json.dumps(record, indent=2) + '\n')
print('Hosted text, symbol facts, relocations and CFI match for both traces')
