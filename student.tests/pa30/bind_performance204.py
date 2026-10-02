#!/usr/bin/env python3
"""Rebuild affected fixed images and bind prior runtime/text evidence to final.

These isolated final-only observations are absolute costs, not A/B claims.
"""
import hashlib
import json
import os
import pathlib
import subprocess
import sys
import time

root = pathlib.Path(__file__).resolve().parents[2]
out = pathlib.Path(sys.argv[1]).resolve()
out.mkdir(parents=True, exist_ok=True)
binary = pathlib.Path(os.environ['RALPH_ARTIFACT_DIR']) / 'audit204/final-cppgm++'
evidence = root / 'student.tests/pa30/evidence204'


def sha(p):
    return hashlib.sha256(p.read_bytes()).hexdigest()


record = dict(compiler_sha256=sha(binary), flags=['-O0', '-c', '--stats'],
              affinity=['taskset', '-c', '2'], launchers=[], images=[])


def measure(args):
    command = ['/usr/bin/time', '-f', '%M', '-o', str(out / 'rss'),
               *record['affinity'], *map(str, args)]
    start = time.perf_counter()
    p = subprocess.run(command, capture_output=True, text=True, timeout=45)
    assert not p.returncode, (command, p.stderr)
    return dict(args=command, status=p.returncode, wall_s=time.perf_counter()-start,
                peak_rss_kib=int((out / 'rss').read_text()),
                stdout=p.stdout, stderr=p.stderr)


for mode, args in [('compile', [binary, '--help']), ('runtime', ['/bin/true'])]:
    for trial in range(8):
        record['launchers'].append(dict(mode=mode, trial=trial, **measure(args)))
for filename in ['vector-performance', 'hosted-new-performance']:
    source = root / ('student.tests/pa30/evidence203/' + filename + '.json')
    previous = json.loads(source.read_text())
    for name, item in previous['inputs'].items():
        if 'source' in item:
            src = out / (name + '.cpp')
            src.write_text(item['source'])
        else:
            src = root / item['path']
        assert sha(src) == item['sha256']
        obj = out / (name + '.o')
        row = measure([binary, *record['flags'], src, '-o', obj])
        old_image = previous['images'][name]
        if 'B' in old_image:
            old_image = old_image['B']
        assert sha(obj) == old_image['object_sha256'], name
        record['images'].append(dict(workload=name, source_sha256=sha(src),
                                     historical_record=str(source.relative_to(root)),
                                     historical_sha256=sha(source), image=old_image, **row))
        (evidence / 'performance-image-binding.json').write_text(json.dumps(record, indent=2)+'\n')
assert sha(binary) == record['compiler_sha256']
print(len(record['images']), 'affected images identical;', len(record['launchers']), 'launcher observations')
