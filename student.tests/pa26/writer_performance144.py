#!/usr/bin/env python3
"""Frozen A/A and ABBA storage workload for the native-to-ELF ownership path."""
import hashlib
import json
import pathlib
import statistics
import subprocess
import sys
import time

out = pathlib.Path(sys.argv[1]).resolve()
out.mkdir(parents=True, exist_ok=True)
binaries = {k: pathlib.Path(v).resolve() for k, v in zip('AB', sys.argv[2:4])}
def digest(path):
    with path.open('rb') as stream:
        h = hashlib.sha256()
        for chunk in iter(lambda: stream.read(1048576), b''):
            h.update(chunk)
        return h.hexdigest()
def run(args):
    r = subprocess.run(list(map(str, args)), capture_output=True, timeout=60)
    assert r.returncode == 0, (args, r.returncode, r.stderr.decode())
    return r
def text_size(path):
    return sum(int(row.split()[1]) for row in run(['size', '-A', path]).stdout.decode().splitlines()
               if row.split() and row.split()[0] == '.text')

values = [0] * 8192
expected = 0
for repeat in range(256):
    for page in range(8192):
        values[page] = (values[page] + repeat + page) & 255
        expected += values[page]
source = out / 'storage.cpp'
source.write_text('''unsigned char payload[33554432];
int main(int argc,char**) {
 unsigned long sum=0;
 for(int repeat=0;repeat<argc*256;++repeat)
  for(unsigned i=0;i<sizeof(payload);i+=4096) {
   payload[i]=(payload[i]+repeat+i/4096)&255; sum+=payload[i];
  }
 return sum==%dUL ? 0:1;
}
''' % expected)
result = {'binaries': {k: {'path': str(v), 'sha256': digest(v)} for k, v in binaries.items()},
          'flags': ['-O0', '-c', '--stats'], 'input_sha256': digest(source), 'expected_sum': expected,
          'host_linker': run(['g++', '--version']).stdout.decode(), 'runs': [], 'images': {}}
for label, binary in binaries.items():
    obj = out / (label + '.o'); exe = out / label
    run([binary, '-O0', '-c', '-o', obj, source]); run(['g++', obj, '-o', exe]); run([exe])
    result['images'][label] = {'object_sha256': digest(obj), 'executable_sha256': digest(exe),
                               'object_text_bytes': text_size(obj), 'executable_text_bytes': text_size(exe),
                               'object_bytes': obj.stat().st_size, 'executable_bytes': exe.stat().st_size}
assert result['images']['A'] == result['images']['B']
def save():
    (out / 'performance.json').write_text(json.dumps(result, indent=2) + '\n')
for mode in ['compile', 'runtime']:
    for block, order in enumerate(['AAAA'] + ['ABBA'] * 6):
        for label in order:
            args = [binaries[label], '-O0', '-c', '--stats', '-o', out / 'measure.o', source] if mode == 'compile' else [out / label]
            started = time.perf_counter()
            proc = run(['/usr/bin/time', '-f', '%M', '-o', out / 'time.txt', *args])
            result['runs'].append({'mode': mode, 'block': block, 'label': label,
                                  'wall_s': time.perf_counter() - started,
                                  'peak_rss_kib': int((out / 'time.txt').read_text()),
                                  'phase_counters': [json.loads(line) for line in proc.stderr.decode().splitlines() if line.startswith('{')]})
            save()
result['summary'] = {}
for mode in ['compile', 'runtime']:
    rows = [r for r in result['runs'] if r['mode'] == mode]
    ratios = [statistics.mean(r['wall_s'] for r in rows if r['block'] == block and r['label'] == 'B') /
              statistics.mean(r['wall_s'] for r in rows if r['block'] == block and r['label'] == 'A') for block in range(1, 7)]
    aa = [r['wall_s'] for r in rows if not r['block']]
    summary = {'AA_range_s': [min(aa), max(aa)], 'paired_ratio_median': statistics.median(ratios),
               'paired_ratios': ratios, 'paired_ratio_range': [min(ratios), max(ratios)]}
    for label in 'AB':
        selected = [r for r in rows if r['block'] and r['label'] == label]
        summary[label] = {'median_s': statistics.median(r['wall_s'] for r in selected),
                          'min_max_s': [min(r['wall_s'] for r in selected), max(r['wall_s'] for r in selected)],
                          'peak_rss_kib': max(r['peak_rss_kib'] for r in selected)}
    result['summary'][mode] = summary
save()
print(json.dumps(result['summary'], indent=2))
