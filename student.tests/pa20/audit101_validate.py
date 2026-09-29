#!/usr/bin/env python3
"""Final whole-stage validation; run CC WORK OUT.json. Preserve all old evidence."""
from pathlib import Path
import hashlib, json, subprocess, sys, time
ROOT = Path(__file__).resolve().parents[2]
CC, WORK, OUT = [Path(x).resolve() for x in sys.argv[1:]]
BASE, ENTRY = 'a9b24ab68f1a75288df10161cb171fa239e1409a', '8c86c298'
WORK.mkdir(parents=True, exist_ok=True)
assert not OUT.exists()
def sha(p): return hashlib.sha256(Path(p).read_bytes()).hexdigest()
result = dict(base=BASE, entry=ENTRY, compiler_sha256=sha(CC), checks={}, controls={}, traces={})
def save(): OUT.write_text(json.dumps(result, indent=2)+'\n')
def check(name, command):
    start = time.monotonic()
    p = subprocess.run([str(x) for x in command], cwd=ROOT, capture_output=True, text=True, timeout=1800)
    log = WORK/(name+'.log'); log.write_text(p.stdout+p.stderr)
    result['checks'][name] = dict(command=[str(x) for x in command], exit=p.returncode,
        elapsed_s=time.monotonic()-start, log=str(log), log_sha256=sha(log),
        summaries=[l for l in (p.stdout+p.stderr).splitlines() if '=====' in l or 'audit passed' in l or '[warning]' in l])
    save(); print(name, p.returncode, flush=True)
    assert p.returncode == 0, str(log)
check('stageTests', ['make', 'test-pa20'])
check('fileAudit', ['perl', 'scripts/cppgm_file_audit.pl', '--stage', 'pa20', '--paths', 'dev/src'])
check('throughPA20', ['make', 'test-report-through-pa20'])
for name in ('deduction94', 'initialization94', 'range95', 'aggregate96', 'closure96',
             'audit97_controls', 'capture98', 'aggregate99', 'value_boundary99', 'angle100', 'audit101_controls'):
    check(name, ['python3', ROOT/f'student.tests/pa20/{name}.py', CC, WORK/name])
    rows = json.loads((WORK/name/'results.json').read_text())
    assert all(r['passed'] for r in rows)
    result['controls'][name] = rows; save()
for version in (94, 95, 96, 98, 99, 100):
    check(f'trace{version}', ['python3', ROOT/f'student.tests/pa20/trace{version}.py', CC, WORK/f'trace{version}', WORK/f'trace{version}.json'])
    result['traces'][str(version)] = json.loads((WORK/f'trace{version}.json').read_text())
check('reference95', ['python3', ROOT/'student.tests/pa20/reference95.py'])
check('referenceExecutions95', ['python3', ROOT/'student.tests/pa20/native95.py', CC, WORK/'native', WORK/'native.json'])
result['reference_executions95'] = json.loads((WORK/'native.json').read_text())
for version in (98, 99):
    check(f'reference{version}', ['python3', ROOT/f'student.tests/pa20/reference{version}.py', WORK/f'reference{version}', CC])
    result[f'reference_executions{version}'] = json.loads((WORK/f'reference{version}/results.json').read_text())
result['value_boundaries'] = json.loads((WORK/'value_boundary99/boundaries.json').read_text())
revisions = [r for version in (95, 98, 99) for r in json.loads((ROOT/f'student.tests/pa20/reference{version}-revisions.json').read_text())['revisions']]
changed = subprocess.check_output(['git', 'diff', '--name-only', BASE, '--', 'pa20/tests'], cwd=ROOT, text=True).splitlines()
assert sorted(changed) == sorted(r['path'] for r in revisions)
assert all(sha(ROOT/r['path']) == r['after_sha256'] for r in revisions)
paths = subprocess.check_output(['git', 'ls-files', 'pa20/tests', 'pa20/README.md', 'pa20/Makefile', 'pa20/scripts', 'scripts', 'shared'], cwd=ROOT, text=True).splitlines()
files = []
for path in paths:
    p = ROOT/path; content = str(p.readlink()).encode() if p.is_symlink() else p.read_bytes()
    assert content == subprocess.check_output(['git', 'show', ENTRY+':'+path], cwd=ROOT), path
    files.append((path, hashlib.sha256(content).hexdigest()))
result['coverage'] = dict(files=len(files), manifest_sha256=hashlib.sha256(json.dumps(files).encode()).hexdigest(),
    unchanged_since_entry=True, stage_reference_corrections=changed)
result['source_hashes'] = {str(p.relative_to(ROOT)):sha(p) for p in sorted((ROOT/'dev/src').rglob('*')) if p.is_file()}
assert sha(CC) == result['compiler_sha256']
save()
