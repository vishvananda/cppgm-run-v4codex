#!/usr/bin/env python3
"""Implementation handoff: required gates, all controls, references and source-to-ELF traces.
Run CC WORK OUT.json. Keeps the full command logs; never changes course fixtures.
"""
from pathlib import Path
import hashlib, json, re, subprocess, sys, time
ROOT = Path(__file__).resolve().parents[2]
CC, WORK, OUT = [Path(x).resolve() for x in sys.argv[1:]]
BASE = 'a9b24ab68f1a75288df10161cb171fa239e1409a'
ENTRY = '16ac49da1339edaa4a5f96bf4efab731a30c65e1'
WORK.mkdir(parents=True, exist_ok=True)
assert not OUT.exists(), 'Keep previous evidence'
def sha(p): return hashlib.sha256(Path(p).read_bytes()).hexdigest()
result = dict(base=BASE, entry=ENTRY, compiler_sha256=sha(CC), checks={}, controls={}, traces={})
def save(): OUT.write_text(json.dumps(result, indent=2)+'\n')
def check(name, command, expected=0):
    command = [str(x) for x in command]
    start = time.monotonic()
    p = subprocess.run(command, cwd=ROOT, capture_output=True, text=True, timeout=1800)
    log = WORK/(name+'.log'); log.write_text(p.stdout+p.stderr)
    result['checks'][name] = dict(command=command, exit=p.returncode, elapsed_s=time.monotonic()-start,
        log=str(log), log_sha256=sha(log), summaries=[l for l in (p.stdout+p.stderr).splitlines()
            if '=====' in l or 'audit passed' in l or '[warning]' in l])
    save(); print(name, p.returncode, flush=True)
    if expected is not None: assert p.returncode == expected, str(log)
    return p
stage = check('stageTests', ['make', 'test-pa20'], None)
check('priorThroughTests', ['bash','-c','n=20; if [ "$n" -le 1 ]; then echo \'===== ALL TESTS PASSED SUCCESSFULLY! (0/0) =====\'; else make test-report-through-pa$((n - 1)); fi'])
check('fileAudit', ['perl','scripts/cppgm_file_audit.pl','--stage','pa20','--paths','dev/src'])
check('throughPA20', ['make','test-report-through-pa20'], None)
entry = set(json.loads((ROOT/'student.tests/pa20/validation98.json').read_text())['progress']['current_failures'])
current = set(re.findall(r'^(pa20/[^:]+): ERROR:', stage.stdout+stage.stderr, re.M))
count = re.search(r'TEST SUMMARY: (\d+) / (\d+) TESTS PASSED', stage.stdout+stage.stderr)
assert count and int(count[2]) == 144 and int(count[1]) == 144-len(current)
assert len(entry) == 4 and len(current) < len(entry) and not current-entry, sorted(current-entry)
result['progress'] = dict(entry_failures=sorted(entry), current_failures=sorted(current),
    new_failures=sorted(current-entry), fixed=sorted(entry-current), total=144, passed=int(count[1]), preserved=True)
for name in ('deduction94','initialization94','range95','aggregate96','closure96','audit97_controls','capture98','aggregate99','value_boundary99'):
    check(name, ['python3',ROOT/f'student.tests/pa20/{name}.py',CC,WORK/name])
    rows = json.loads((WORK/name/'results.json').read_text())
    assert all(r['passed'] for r in rows)
    result['controls'][name] = rows
    save()
check('referenceProof', ['python3',ROOT/'student.tests/pa20/reference95.py'])
check('referenceExecutions', ['python3',ROOT/'student.tests/pa20/native95.py',CC,WORK/'native',WORK/'native.json'])
result['reference_executions'] = json.loads((WORK/'native.json').read_text())
for version in (94,95,96,98,99):
    check(f'trace{version}', ['python3',ROOT/f'student.tests/pa20/trace{version}.py',CC,WORK/f'trace{version}',WORK/f'trace{version}.json'])
    result['traces'][str(version)] = json.loads((WORK/f'trace{version}.json').read_text())
check('closureReferenceProof', ['python3',ROOT/'student.tests/pa20/reference98.py',WORK/'closure-reference',CC])
result['closure_reference_executions'] = json.loads((WORK/'closure-reference/results.json').read_text())
check('aggregateReferenceProof', ['python3',ROOT/'student.tests/pa20/reference99.py',WORK/'aggregate-reference',CC])
result['aggregate_reference_executions'] = json.loads((WORK/'aggregate-reference/results.json').read_text())
result['value_boundaries'] = json.loads((WORK/'value_boundary99/boundaries.json').read_text())
# Execute every repaired required program that has an entry point.
result['repaired_executions'] = []
for path in sorted(entry-current):
    src = ROOT/path
    reference = src.with_suffix('.ref')
    if 'function @main(' not in reference.read_text(): continue
    name = src.stem; outputs = []
    for label, ir in [('reference',reference),('student',src.with_suffix('.my'))]:
        exe = WORK/(name+'-'+label)
        backend = check(name+'-'+label,['dev/lowir2native-ref','-O0','-o',exe,ir],None)
        if backend.returncode:
            assert 'undefined native symbol:' in backend.stderr, backend.stderr
            outputs.append(dict(kind=label,exit=None,backend_exit=backend.returncode,
                limitation='fixture declares but does not define an external function',diagnostic=backend.stderr))
            continue
        execution = subprocess.run([exe],capture_output=True,text=True,timeout=30)
        outputs.append(dict(kind=label,exit=execution.returncode,sha256=sha(exe)))
    assert outputs[0]['exit']==outputs[1]['exit'],name
    result['repaired_executions'].append(dict(name=name,outputs=outputs))
# Verify the entire stage's fixture changes are exactly the documented revisions.
manifest = json.loads((ROOT/'student.tests/pa20/reference95-revisions.json').read_text())
manifest['revisions'] += json.loads((ROOT/'student.tests/pa20/reference98-revisions.json').read_text())['revisions']
manifest['revisions'] += json.loads((ROOT/'student.tests/pa20/reference99-revisions.json').read_text())['revisions']
changed = subprocess.check_output(['git','diff','--name-only',BASE,'--','pa20/tests'],cwd=ROOT,text=True).splitlines()
assert sorted(changed) == sorted(r['path'] for r in manifest['revisions'])
paths = subprocess.check_output(['git','ls-files','pa20/tests','pa20/README.md','pa20/Makefile','pa20/scripts','scripts','shared'],cwd=ROOT,text=True).splitlines()
files = []
for path in paths:
    p = ROOT/path
    content = str(p.readlink()).encode() if p.is_symlink() else p.read_bytes()
    before = subprocess.check_output(['git','show',ENTRY+':'+path],cwd=ROOT)
    revised = {r['path']:r['after_sha256'] for r in manifest['revisions']}
    assert before == content or (path in revised and sha(p)==revised[path]), path
    files.append((path,hashlib.sha256(content).hexdigest()))
result['coverage'] = dict(files=len(files), manifest_sha256=hashlib.sha256(json.dumps(files).encode()).hexdigest(),
    unchanged_since_entry_except_proved_oracles=True, sources=144, comparison_unchanged=True, stage_reference_corrections=changed)
result['source_hashes'] = {str(p.relative_to(ROOT)):sha(p) for p in sorted((ROOT/'dev/src').rglob('*')) if p.is_file()}
assert sha(CC) == result['compiler_sha256']
save()
