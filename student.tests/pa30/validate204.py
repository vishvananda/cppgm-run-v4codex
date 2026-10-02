#!/usr/bin/env python3
"""Retain exact final required-check output, status and binary bindings."""
import hashlib
import json
import pathlib
import subprocess
import time

root = pathlib.Path(__file__).resolve().parents[2]
out = root / 'student.tests/pa30/evidence204/validation.json'
commands = [
    ['perl', 'scripts/cppgm_file_audit.pl', '--stage', 'pa30', '--paths', 'dev/src'],
    ['make', 'test-pa30'],
    ['make', 'test-report-through-pa30'],
]
record = dict(binaries={n: hashlib.sha256((root / 'dev' / n).read_bytes()).hexdigest()
                        for n in ['cppgm++', 'lowir', 'lowir2native']}, commands=[])
for args in commands:
    start = time.monotonic()
    p = subprocess.run(args, cwd=root, text=True, stdout=subprocess.PIPE,
                       stderr=subprocess.STDOUT)
    record['commands'].append(dict(args=args, status=p.returncode, output=p.stdout,
                                    wall_s=time.monotonic() - start))
    out.write_text(json.dumps(record, indent=2) + '\n')
    print(' '.join(args), p.returncode, p.stdout.splitlines()[-1], flush=True)
    assert not p.returncode
