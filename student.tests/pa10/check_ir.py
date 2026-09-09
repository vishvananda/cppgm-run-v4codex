#!/usr/bin/env python3
"""Audit successful course programs in memory, before serialization."""
from pathlib import Path
import subprocess
import sys
import tempfile
ROOT=Path(__file__).resolve().parents[2]
binary=Path(sys.argv[1]).resolve() if len(sys.argv)>1 else ROOT/'dev/cppgm++'
inputs=[p for p in sorted((ROOT/'pa10/tests/general').glob('*.t'))
        if p.with_suffix('.ref.exit_status').read_text().strip()=='EXIT_SUCCESS']
inputs+=sorted((ROOT/'pa10/tests/controls').glob('*.cpp'))
with tempfile.TemporaryDirectory(prefix='pa10-ir-audit-') as work:
    for source in inputs:
        r=subprocess.run([binary,'--emit-lowir','--validate-lowir','-O0','-o',Path(work)/'out.lowir',source],capture_output=True,text=True)
        assert r.returncode==0,(source,r.returncode,r.stderr)
print(f'PASS: {len(inputs)} source programs passed typed in-memory validation')
