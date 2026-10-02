#!/usr/bin/env python3
"""Sequential frozen whole-stage affected and common A/A + ABBA benchmarks."""
import json
import os
import pathlib
import subprocess
import sys

ROOT = pathlib.Path(__file__).resolve().parents[2]
OUT = pathlib.Path(sys.argv[1]).resolve()
ART = pathlib.Path(os.environ['RALPH_ARTIFACT_DIR'])
OUT.mkdir(parents=True, exist_ok=True)
after = OUT.parent / 'final/cppgm++'
entry = OUT.parent / 'baseline/cppgm++'
assert after.exists() and entry.exists()
records = json.loads((OUT / 'commands.json').read_text()) if '--resume' in sys.argv else []


def measure(name, script, before=None, env=None):
    if any(row['name'] == name and row['exit_code'] == 0 for row in records):
        return
    environment = dict(os.environ, PERF_CPU='2')
    environment.update(env or {})
    command = ['python3', str(ROOT / 'student.tests/pa32' / script), str(OUT / name)]
    if before is not None:
        command.append(str(before))
    command.append(str(after))
    with (OUT / (name + '.log')).open('w') as log:
        result = subprocess.run(command, cwd=ROOT, env=environment, stdout=log, stderr=subprocess.STDOUT)
    records.append(dict(name=name, command=command, exit_code=result.returncode,
                        environment={key: environment[key] for key in ['PERF_CPU', *(env or {})]}))
    (OUT / 'commands.json').write_text(json.dumps(records, indent=2) + '\n')
    print(name, result.returncode, flush=True)
    assert result.returncode == 0, name


for level in [0, 1, 3]:
    measure('common-o' + str(level), 'common_levels.py', entry,
            dict(PA32_BASE_LEVEL=str(level), PA32_FINAL_LEVEL=str(level)))
measure('selfhost', 'selfhost_performance.py', entry)
measure('scalar', 'performance.py')
for name, script, before in [
    ('objects', 'objects_performance.py', ART / 'pa32-211/before-cppgm'),
    ('loops', 'loops_performance.py', ART / 'pa32-212/A'),
    ('memory', 'memory_performance.py', ART / 'pa32-213/A'),
    ('ranges', 'range_performance.py', ART / 'pa32-215/cppgm-A'),
    ('context', 'context_performance.py', ART / 'pa32-216/baseline/cppgm++'),
    ('source', 'source_performance.py', ART / 'pa32-217/baseline-cppgm++'),
]:
    measure(name, script, before)
measure('debug', 'debug_performance.py')
