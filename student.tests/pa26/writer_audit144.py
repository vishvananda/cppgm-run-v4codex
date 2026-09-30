#!/usr/bin/env python3
"""Compare frozen ELF writers on every PA26 TU and explicit layout controls."""
import hashlib
import json
import pathlib
import subprocess
import sys

out = pathlib.Path(sys.argv[1]).resolve()
out.mkdir(parents=True, exist_ok=True)
binaries = [pathlib.Path(arg).resolve() for arg in sys.argv[2:4]]
root = pathlib.Path(__file__).resolve().parents[2]

def digest(path):
    h = hashlib.sha256()
    with path.open('rb') as stream:
        for block in iter(lambda: stream.read(1048576), b''):
            h.update(block)
    return h.hexdigest()

def run(args):
    return subprocess.run(list(map(str, args)), cwd=root, capture_output=True, timeout=60)

sources = sorted((root / 'pa26/tests/general').glob('*.t.[0-9]*'))
for name, source in {
    'empty': '',
    'alignment': 'alignas(4096) char data[8193]; int f(){return data[8192];}',
    'lifecycle': 'extern int seed(); int value=seed(); struct G{~G(){++value;}}; G g;',
    'aliases': 'extern "C" int f(){return 7;} extern "C" int alias() __attribute__((alias("f")));',
    'storage': 'char data[33554432]; int main(){return data[0];}',
}.items():
    path = out / (name + '.cpp')
    path.write_text(source + '\n')
    sources.append(path)

record = {'binaries': [{'path': str(p), 'sha256': digest(p)} for p in binaries],
          'flags': ['-O0', '-c'], 'objects': [], 'write_failures': []}
for source in sources:
    observations = []
    for label, binary in zip('AB', binaries):
        obj = out / (label + '.o')
        result = run([binary, '-O0', '-c', '-o', obj, source])
        assert result.returncode == 0, (source, result.stderr.decode())
        observations.append({'sha256': digest(obj), 'bytes': obj.stat().st_size})
    assert observations[0] == observations[1], source
    record['objects'].append({'source': str(source.relative_to(root)) if source.is_relative_to(root) else str(source),
                              'source_sha256': digest(source), **observations[0]})
for binary in binaries:
    result = run([binary, '-c', '-o', '/dev/full', sources[-1]])
    assert result.returncode != 0
    record['write_failures'].append({'binary': str(binary), 'status': result.returncode})
(out / 'writer.json').write_text(json.dumps(record, indent=2) + '\n')
print('%d ELF objects byte-identical; both writers reject failed output' % len(sources))
