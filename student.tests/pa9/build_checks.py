#!/usr/bin/env python3
"""Build explicit direct-API or sanitizer checks; no generated repository files."""
import argparse
import subprocess
from pathlib import Path
ROOT = Path(__file__).resolve().parents[2]
p = argparse.ArgumentParser()
p.add_argument('--sanitize', action='store_true')
p.add_argument('--output', default='/tmp/pa9-evidence/check-api')
a = p.parse_args()
Path(a.output).parent.mkdir(parents=True, exist_ok=True)
sources = sorted(str(x) for x in (ROOT / 'dev/src/abi/itanium').glob('*.cpp'))
flags = ['-O1', '-g', '-fsanitize=address,undefined', '-fno-omit-frame-pointer', '-fno-pie', '-no-pie'] if a.sanitize else ['-O2']
subprocess.run(['g++', '-std=c++11', '-Wall', *flags, '-I' + str(ROOT / 'dev/src'),
    str(ROOT / 'student.tests/pa9/check_api.cpp'), str(ROOT / 'dev/src/preprocess/source.cpp'),
    str(ROOT / 'dev/src/preprocess/identifier_table.cpp'), *sources, '-o', a.output], check=True)
