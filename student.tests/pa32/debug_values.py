#!/usr/bin/env python3
"""Source-value snapshots, locations and durable LowIR across all four levels."""
import json
import pathlib
import re
import subprocess
import sys

ROOT = pathlib.Path(__file__).resolve().parents[2]
OUT = pathlib.Path(sys.argv[1]).resolve()
OUT.mkdir(parents=True, exist_ok=True)
records = []


def run(args):
    args = list(map(str, args))
    p = subprocess.run(args, cwd=ROOT, capture_output=True, text=True, timeout=60)
    records.append(dict(command=args, status=p.returncode, stdout=p.stdout, stderr=p.stderr))
    (OUT / 'checks.json').write_text(json.dumps(records, indent=2) + '\n')
    assert p.returncode == 0, (args, p.stdout, p.stderr)
    return p


source = OUT / 'source.cpp'
source.write_text('''volatile int seen;
template<class T> T update(T x) {
  T y = x + 1;
  if (x & 1) y = y + 2;
  else y = y + 3;
  for (int i = 0; i < 3; ++i) { seen = i; y = y + i; }
  return y;
}
int main(int argc,char**) {
  int value = update(argc);
  { int value = update(argc + 1); seen += value; }
  double d = argc + 0.5; d += 1.0;
  return value != argc + 4 + ((argc & 1) ? 2 : 3) ||
    seen != argc + 7 + (((argc + 1) & 1) ? 2 : 3) || d != argc + 1.5;
}
''')
for debug in [False, True]:
    flag = '-gline-tables-only' if debug else '-g0'
    seed = OUT / ('debug.lowir' if debug else 'plain.lowir')
    run(['dev/cppgm++', '--emit-lowir', '--validate-lowir', '-O0', flag, source, '-o', seed])
    for level in range(4):
        prefix = OUT / f'{debug}-{level}'
        direct, replay, text, exe = [pathlib.Path(str(prefix) + s) for s in ['.o', '-replay.o', '.lowir', '.exe']]
        run(['dev/cppgm++', '--emit-lowir', '--validate-lowir', f'-O{level}', flag, source, '-o', text])
        run(['dev/cppgm++', '-c', f'-O{level}', flag, source, '-o', direct])
        run(['dev/cppgm++', '-c', f'-O{level}', seed, '-o', replay])
        assert direct.read_bytes() == replay.read_bytes(), (debug, level, 'object replay')
        run(['g++', direct, '-o', exe])
        run([exe]); run([exe, 'input'])
        if debug:
            content = text.read_text()
            assert re.search(r'%dbg_y\S* = copy.*!dbg', content), content
            assert re.search(r'return .*!dbg\([^\n]+, 7, 3\)', content), content
        else:
            assert '!dbg(' not in text.read_text()

ir = OUT / 'explicit.lowir'
ir.write_text('''function @main() -> i32 [role=entry] {
  block ^entry:
    %source_value = copy i32 9 !dbg(source.cpp, 4, 3)
    %same_value = copy i32 %source_value !dbg(source.cpp, 5, 3)
    %difference = binary sub i32 %same_value, 9
    return i32 %difference !dbg(source.cpp, 6, 3)
}
''')
for level in range(4):
    text = OUT / f'explicit-{level}.lowir'
    run(['dev/lowiropt', f'-O{level}', '-o', text, ir])
    assert text.read_text().count('= copy i32') == 2
    obj, exe = OUT / 'explicit.o', OUT / 'explicit.exe'
    run(['dev/cppgm++', '-c', '-O0', text, '-o', obj])
    run(['g++', obj, '-o', exe]); run([exe])
print('PASS: debug/plain source execution, 8 direct/replayed object pairs, '
      'template/loop/shadow/float snapshots and explicit LowIR at O0-O3')
