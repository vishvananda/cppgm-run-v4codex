#!/usr/bin/env python3
"""Run isolated reports: the course harness shares counters between reports."""
import hashlib, json, pathlib, subprocess, sys, time
root = pathlib.Path(__file__).resolve().parents[2]
out = pathlib.Path(sys.argv[1]).resolve()
out.mkdir(parents=True, exist_ok=True)
records = {}
commands = {
    'priorThroughTests': 'n=30; if [ "$n" -le 1 ]; then echo \'===== ALL TESTS PASSED SUCCESSFULLY! (0/0) =====\'; else make test-report-through-pa$((n - 1)); fi',
    'fileAudit': 'perl scripts/cppgm_file_audit.pl --stage pa30 --paths dev/src',
    'stageTests': 'make test-pa30',
    'through30': 'make test-report-through-pa30',
}
for name, command in commands.items():
    start = time.perf_counter()
    log = out/(name+'.log')
    with log.open('w') as f:
        p = subprocess.run(['bash', '-c', command], cwd=root, stdout=f, stderr=subprocess.STDOUT)
    data = log.read_bytes()
    records[name] = dict(command=command, status=p.returncode,
        wall_s=time.perf_counter()-start, sha256=hashlib.sha256(data).hexdigest(), output=data.decode())
    (out/'validation.json').write_text(json.dumps(records, indent=2)+'\n')
    print(name, p.returncode, data.decode().splitlines()[-1], flush=True)
entry = json.loads((root/'student.tests/pa30/evidence201/entry.json').read_text())
old = {r['path']: r for r in entry['cases']}
cases = []
for source in sorted((root/'pa30/tests/compile').glob('*.t')):
    stem = str(source)[:-2]
    cases.append(dict(path=str(source.relative_to(root)),
        expected=pathlib.Path(stem+'.ref.exit_status').read_text().strip(),
        actual=pathlib.Path(stem+'.my.exit_status').read_text().strip(),
        diagnostic=pathlib.Path(stem+'.my.stdout').read_text()))
failures = [r['path'] for r in cases if r['expected'] != r['actual']]
old_failures = [r['path'] for r in old.values() if r['expected'] != r['actual']]
delta = dict(entry_failures=old_failures, final_failures=failures, cases=cases,
    fixed=sorted(set(old_failures)-set(failures)), regressions=sorted(set(failures)-set(old_failures)))
(out/'stage-delta.json').write_text(json.dumps(delta, indent=2)+'\n')
assert set(old) == {r['path'] for r in cases}
delta['reference_correction'] = 'pa30/tests/compile/700-hosted-replaceable-operator-new-dynamic-exception-spec.t'
delta['implementation_fixed'] = [x for x in delta['fixed'] if x != delta['reference_correction']]
(out/'stage-delta.json').write_text(json.dumps(delta, indent=2)+'\n')
assert len(delta['implementation_fixed']) >= 2
assert not delta['regressions'] and len(failures) < len(old_failures) == 5
assert records['priorThroughTests']['status'] == records['fileAudit']['status'] == 0
print('stageProgress: pass;', len(cases)-len(failures), '/', len(cases), flush=True)
