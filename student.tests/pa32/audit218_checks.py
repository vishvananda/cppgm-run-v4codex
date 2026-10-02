#!/usr/bin/env python3
"""Final audit checks, sequential reports and explicit personal coverage."""
import hashlib
import json
import pathlib
import subprocess
import sys

ROOT = pathlib.Path(__file__).resolve().parents[2]
OUT = pathlib.Path(sys.argv[1]).resolve()
OUT.mkdir(parents=True, exist_ok=True)
checks = json.loads((OUT / 'checks.json').read_text()) if '--resume' in sys.argv else []


def check(name, args, cwd=ROOT, required=True):
    previous = [row for row in checks if row['name'] == name]
    if previous and (previous[-1]['exit_code'] == 0 or not required):
        return
    args = list(map(str, args))
    log = OUT / (name + ('.attempt' + str(len(previous)+1) if previous else '') + '.log')
    with log.open('w') as stream:
        result = subprocess.run(args, cwd=cwd, stdout=stream, stderr=subprocess.STDOUT)
    checks.append(dict(name=name, command=args, cwd=str(cwd), exit_code=result.returncode,
                       log=str(log), sha256=hashlib.sha256(log.read_bytes()).hexdigest()))
    (OUT / 'checks.json').write_text(json.dumps(checks, indent=2) + '\n')
    print(name, result.returncode, flush=True)
    if required:
        assert result.returncode == 0, name


check('stage', ['make', 'test-pa32'])
check('through', ['make', 'test-report-through-pa32'])
check('file', ['perl', 'scripts/cppgm_file_audit.pl', '--stage', 'pa32', '--paths', 'dev/src'])
# PA8's README excludes source/optimization/native/DWARF behavior from its
# milestone. Record this exploratory later-tool lane without promoting its
# exact source/MIR reference shapes into additional PA32 requirements.
check('prior-debug', ['make', '-C', 'pa8', 'test-debuginfo'], required=False)
check('debug', ['make', '-C', 'pa32', 'test-debuginfo'])
check('debug-values', ['python3', 'student.tests/pa32/debug_values.py', OUT / 'debug-values'])
for name in ['source_identity', 'pointer_congruence', 'floating_facts', 'context_guards',
             'contextual_calls', 'calls', 'control_closure', 'range_fills', 'pointer_loops',
             'memory', 'memory_native', 'loops', 'loop_trip_properties', 'objects', 'local',
             'dataflow', 'audit', 'range_native']:
    check(name, ['python3', f'student.tests/pa32/{name}.py'])
for name in ['context', 'memory', 'range']:
    check(name + '-bounds', ['python3', f'student.tests/pa32/{name}_bounds.py', OUT / (name + '-bounds.json')])
check('loop-bounds', ['python3', 'student.tests/pa32/loops_bounds.py', OUT / 'loop-bounds'])
for name in ['audit_trace', 'loops_trace', 'memory_trace', 'range_trace', 'audit214']:
    check(name, ['python3', f'student.tests/pa32/{name}.py', OUT / name])
