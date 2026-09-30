#!/usr/bin/env python3
"""The PA26 output contract applies independently of the requested filename."""
import hashlib
import json
import pathlib
import subprocess
import sys

root = pathlib.Path(__file__).resolve().parents[2]
out = pathlib.Path(sys.argv[1]).resolve()
out.mkdir(parents=True, exist_ok=True)
compiler = pathlib.Path(sys.argv[2]).resolve()
source = root / 'student.tests/pa26/output_policy.cpp'
result = {'compiler_sha256': hashlib.sha256(compiler.read_bytes()).hexdigest(),
          'source_sha256': hashlib.sha256(source.read_bytes()).hexdigest(), 'cases': []}
for name in ['output.o', 'output.obj', 'output', 'output.bin']:
    obj = out / name
    proc = subprocess.run([str(compiler), '-c', '-o', str(obj), str(source)], capture_output=True, timeout=30)
    case = {'name': name, 'compile_status': proc.returncode,
            'elf': proc.returncode == 0 and obj.read_bytes()[:4] == b'\x7fELF'}
    if case['elf']:
        exe = out / (name + '-run')
        link = subprocess.run(['g++', str(obj), '-o', str(exe)], capture_output=True, timeout=30)
        case['link_status'] = link.returncode
        if link.returncode == 0:
            case['run_status'] = subprocess.run([str(exe)], capture_output=True, timeout=30).returncode
    result['cases'].append(case)
(out / 'policy.json').write_text(json.dumps(result, indent=2) + '\n')
failed = [c['name'] for c in result['cases'] if not c['elf'] or c.get('link_status') != 0 or c.get('run_status') != 0]
print('output-policy failures:', failed)
sys.exit(bool(failed))
