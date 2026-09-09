#!/usr/bin/env python3
"""Explicit PA12 semantic reducers, validated and executed through the PA8 backend."""
from pathlib import Path
import subprocess
import tempfile
ROOT = Path(__file__).resolve().parents[2]
HERE = Path(__file__).resolve().parent
with tempfile.TemporaryDirectory(prefix='pa12-personal-') as directory:
    scratch = Path(directory)
    for source in sorted(HERE.glob('*.cpp')):
        ir, exe = scratch/'test.lowir', scratch/'test'
        compile_command = [ROOT/'dev/cppgm++', '--emit-lowir', '-O0', '--validate-lowir', '-o', ir, source]
        result = subprocess.run([str(x) for x in compile_command], capture_output=True, timeout=60)
        if source.stem.endswith('-bad'):
            assert result.returncode, str(source) + ': accepted invalid source'
        else:
            assert result.returncode == 0, str(source) + ': ' + result.stderr.decode()
            subprocess.run([str(x) for x in [ROOT/'dev/lowir2native-ref', '-O0', '-o', exe, ir]], check=True, timeout=60)
            subprocess.run([str(exe)], check=True, timeout=60)
        print(source.name + ': pass')
