#!/usr/bin/env python3
"""Reproduce the pinned bundle's constant-initialization defect (observation only)."""
from pathlib import Path
import hashlib
import json
import subprocess
import tempfile
ROOT = Path(__file__).resolve().parents[2]
results = []
with tempfile.TemporaryDirectory(prefix='pa10-reference-proof-') as work:
    work = Path(work)
    for name in ('constant-address-order', 'constant-reference-order'):
        source = ROOT/'student.tests/pa10'/f'{name}.cpp'
        ir, program = work/f'{name}.lowir', work/name
        subprocess.run([ROOT/'dev/cppgm++-ref', '--emit-lowir', '-O0', '-o', ir, source], check=True)
        subprocess.run([ROOT/'dev/lowir2native-ref', '-O0', '-o', program, ir], check=True)
        result = subprocess.run([program], capture_output=True, timeout=10)
        assert result.returncode == 1 and result.stdout == b'', result
        results.append(dict(reducer=name, required_exit=0, pinned_bundle_exit=result.returncode,
                            source_sha256=hashlib.sha256(source.read_bytes()).hexdigest()))
print(json.dumps(results, indent=2))
