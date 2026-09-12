#!/usr/bin/env python3
"""Independent signature provenance and runtime-extent work checks."""
from pathlib import Path
import json, re, subprocess, sys, tempfile

root = Path(__file__).resolve().parents[2]
compiler = Path(sys.argv[1]).resolve() if len(sys.argv) > 1 else root/'dev/cppgm++'

def compile_source(work, text):
    source, output = work/'source.cpp', work/'out.lowir'
    source.write_text(text)
    run = subprocess.run([str(compiler), '--emit-lowir', '-O0', '--stats',
        '--validate-lowir', '-o', str(output), str(source)], capture_output=True,
        text=True, timeout=60)
    assert run.returncode == 0, run.stderr
    return output.read_text(), json.loads(run.stderr.splitlines()[-1])

with tempfile.TemporaryDirectory(prefix='pa13-audit-') as tmp:
    work = Path(tmp)
    ir, _ = compile_source(work, '''
struct B { virtual int f() { return 7; } };
int ordinary(B* p) { return p == nullptr ? 9 : 8; }
int run(B& b, int (*p)(B*)) { return b.f() + p(nullptr); }
int main() { B b; return run(b, ordinary) != 16; }
''')
    calls = re.findall(r'call i32 %[^\n]* as ([^\n]*)', ir)
    assert len(calls) == 2, calls
    assert 'object_bytes=8' in calls[0], calls
    assert 'object_bytes' not in calls[1], calls
    exe = work/'exec'
    for cmd in ([root/'dev/lowir2native-ref', '-O0', '-o', exe, work/'out.lowir'], [exe]):
        result = subprocess.run(list(map(str, cmd)), capture_output=True, text=True, timeout=60)
        assert result.returncode == 0, (cmd, result.returncode, result.stderr)
    counts = []
    for extent in (19, 1000000):
        _, stats = compile_source(work, '''
struct B { B() {} virtual int f() { return 7; } virtual ~B() {} };
int main() { volatile int n = '''+str(extent)+''';
B* p = new B[n]; int answer = p[n-1].f(); delete[] p; return answer != 7; }
''')
        counts.append(stats['instructions'])
    assert counts[0] == counts[1], counts
    print('signature ownership and constant-size array lowering passed', counts)
