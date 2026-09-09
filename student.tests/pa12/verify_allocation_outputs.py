#!/usr/bin/env python3
"""Verify final compiler outputs against the frozen allocation campaign."""
from pathlib import Path
import hashlib
import json
import subprocess
import sys

root = Path(__file__).resolve().parents[2]
compiler, campaign, work, output = map(Path, sys.argv[1:5])
work.mkdir(parents=True, exist_ok=True)
sha = lambda path: hashlib.sha256(path.read_bytes()).hexdigest()
data = dict(compiler=str(compiler), compiler_sha256=sha(compiler),
            campaign_sha256=sha(campaign), compile_flags=['--emit-lowir', '-O0', '--validate-lowir'],
            native_flags=['-O0'], workloads={})
for name, item in json.loads(campaign.read_text())['workloads'].items():
    source = Path(item['source_path'])
    assert sha(source) == item['source_sha256']
    ir, exe = work/(name+'.lowir'), work/name
    subprocess.run([str(x) for x in [compiler, '--emit-lowir', '-O0', '--validate-lowir', '-o', ir, source]], check=True)
    before = item['outputs'][-1]
    assert sha(ir) == before['lowir_sha256'], name+': LowIR changed'
    subprocess.run([str(x) for x in [root/'dev/lowir2native-ref', '-O0', '-o', exe, ir]], check=True)
    assert sha(exe) == before['executable_sha256'], name+': native output changed'
    data['workloads'][name] = dict(source_sha256=sha(source), lowir_sha256=sha(ir),
                                  executable_sha256=sha(exe), equivalent=True)
    print(name+': byte-identical LowIR and native output', flush=True)
output.write_text(json.dumps(data, indent=2)+'\n')
