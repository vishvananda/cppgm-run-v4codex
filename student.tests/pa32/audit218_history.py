#!/usr/bin/env python3
"""Verify the unaudited 215-217 handoffs against their committed snapshots."""
import hashlib
import json
import os
import pathlib
import subprocess
import sys
from audit214_history import measurement

ROOT = pathlib.Path(__file__).resolve().parents[2]


def git(*args):
    return subprocess.check_output(['git', *args], cwd=ROOT)


def sha(data):
    return hashlib.sha256(data).hexdigest()


rows = []
for number, commit in [(215, '33561837'), (216, '7e04081b'), (217, '5a767f20')]:
    folder = ROOT / f'student.tests/pa32/evidence{number}'
    binding = json.loads((folder / 'binding.json').read_text())
    entries = {}
    for entry in git('ls-tree', '-r', commit).decode().splitlines():
        header, name = entry.split('\t', 1)
        entries[name] = header.split()[0]
    sources = binding.get('sources', binding.get('implementation'))
    for name, digest in sources.items():
        target = name
        while entries[target] == '120000':
            target = os.path.normpath(os.path.join(os.path.dirname(target), git('show', commit + ':' + target).decode()))
        assert sha(git('show', commit + ':' + target)) == digest, (number, name)
    for name, digest in binding['evidence'].items():
        assert sha((folder / name).read_bytes()) == digest, (number, name)
    artifacts = {**binding['artifacts'], **binding.get('binaries', {})}
    rebound = {}
    for name, digest in artifacts.items():
        target = pathlib.Path(name)
        if target.parent == ROOT / 'dev' and sha(target.read_bytes()) != digest:
            candidates = [pathlib.Path(p) for p, h in artifacts.items()
                          if h == digest and pathlib.Path(p).parent != ROOT / 'dev']
            target = next((p for p in candidates if p.is_file() and sha(p.read_bytes()) == digest), target)
            rebound[name] = str(target)
        assert sha(target.read_bytes()) == digest, (number, name)
    samples, files = 0, []
    for file in sorted(folder.glob('*.json')):
        data = json.loads(file.read_text())
        if not isinstance(data, dict) or not data.get('runs') or not data.get('summary'):
            continue
        for binary in data['binaries'].values():
            assert sha(pathlib.Path(binary['path']).read_bytes()) == binary['sha256']
        samples += measurement(data)
        files.append(file.name)
    rows.append(dict(handoff=number, commit=git('rev-parse', commit).decode().strip(),
                     sources=len(sources), artifacts=len(binding['artifacts']),
                     observations=samples, lanes=files, frozen_rebindings=rebound))
pathlib.Path(sys.argv[1]).write_text(json.dumps(rows, indent=2) + '\n')
print(json.dumps(rows))
