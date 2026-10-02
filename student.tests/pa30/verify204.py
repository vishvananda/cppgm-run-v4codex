#!/usr/bin/env python3
"""Verify final audit bindings, coverage, outcomes and calibrated measurements."""
import hashlib
import json
import pathlib
import re
import statistics
import subprocess

root = pathlib.Path(__file__).resolve().parents[2]
out = root / 'student.tests/pa30/evidence204'
checks = []


def read(name):
    return json.loads((out / (name + '.json')).read_text())


def sha(p):
    return hashlib.sha256(p.read_bytes()).hexdigest()


def check(value, name):
    assert value, name
    checks.append(name)


def git(*args):
    return subprocess.check_output(['git', *args], cwd=root, text=True).strip()


binding = read('source-binding')
code = binding['code_commit']
final = binding['binaries']['cppgm++']
check(not git('diff', code, '--', 'dev'), 'production code matches reviewed commit')
check(git('rev-parse', code + ':dev') == binding['dev_tree'], 'reviewed dev tree')
for name, digest in binding['files'].items():
    check(sha(root / name) == digest, 'bound source ' + name)
for name, digest in binding['binaries'].items():
    check(sha(root / 'dev' / name) == digest, 'final tool ' + name)
for label, item in binding['frozen'].items():
    check(sha(pathlib.Path(item['path'])) == item['sha256'], 'frozen ' + label)
protected = git('diff', '--name-only', binding['stage_base'], '--',
                ':(glob)pa*/tests/**', 'scripts', 'reference-binaries').splitlines()
check(sorted(protected) == sorted(binding['authorized_reference_sidecars']),
      'only six proven reference sidecars changed across whole stage')
for name, digest in binding['reference_sidecars'].items():
    check(sha(root / name) == digest, 'reference ' + name)
check(git('diff', '--name-only', '547b2678', '--', ':(glob)pa*/tests/**',
          'scripts', 'reference-binaries') == '', 'audit changes no contract input or oracle')
cases = sorted(str(p.relative_to(root)) for p in (root / 'pa30/tests/compile').glob('*.t'))
check(cases == sorted(binding['stage_cases']), 'all 153 stage case identities retained')
for name, digest in binding['stage_cases'].items():
    check(sha(root / name) == digest, 'stage input ' + name)

v = read('validation')
check(v['binaries'] == binding['binaries'], 'validation final binary bindings')
expected = [
    ['perl', 'scripts/cppgm_file_audit.pl', '--stage', 'pa30', '--paths', 'dev/src'],
    ['make', 'test-pa30'], ['make', 'test-report-through-pa30']]
check([r['args'] for r in v['commands']] == expected, 'exact required commands')
for r in v['commands']:
    check(r['status'] == 0, 'required status ' + ' '.join(r['args']))
check('(153 / 153)' in v['commands'][1]['output'], 'PA30 complete')
report = v['commands'][2]['output']
check('(5094 / 5094)' in report, 'whole prior-through report complete')
check(len(re.findall(r'^===== pa\d+ =====$', report, re.M)) == 30, 'all 30 stages present')

inspection = read('inspection')
check(inspection['binaries'] == binding['binaries'], 'inspection final tools')
check(len(inspection['commands']) == 344, 'inspection command inventory')
for i, r in enumerate(inspection['commands']):
    check((r['status'] != 0) == r['expected_rejection'], 'inspection outcome ' + str(i))
check(sum(r['expected_rejection'] for r in inspection['commands']) == 12,
      'six malformed descriptor reducers reject in both consumers')
for n, count in [(198, 107), (199, 88), (200, 50), (201, 111), (202, 35), (203, 48)]:
    data = inspection['controls'][str(n)]
    check(data['compiler_sha256'] == final and len(data['commands']) == count,
          'inherited control compiler/count ' + str(n))
    for i, r in enumerate(data['commands']):
        check((r['status'] != 0) == r['expected_rejection'], 'control %d/%d' % (n, i))
before = read('reducers-before')
check(len(before['commands']) == 8 and all(r['entry_status'] == 0 for r in before['commands']),
      'entry accepts malformed and valid reducers')
for r in before['commands']:
    check(sha(root / r['source']) == r['source_sha256'], 'unchanged reduced input ' + r['source'])
diff = read('entry-differential')
check(len(diff['cases']) == len(inspection['differential_images']) == 286,
      'differential image inventory')
for r, rebuilt in zip(diff['cases'], inspection['differential_images']):
    check(r['source'] == rebuilt['source'] and r['sha256'] == rebuilt['source_sha256'],
          'differential source binding ' + r['source'])
    check(r['equal'] and len(r['commands']) == 8 and all(c['status'] == 0 for c in r['commands']),
          'differential execution ' + r['source'])
for name, image in inspection['images'].items():
    check(image['telemetry_identical'], 'telemetry unchanged ' + name)
    check(all(read('elf-equivalence')['images'][name].values()), 'hosted ELF facts equal ' + name)

observations = 0
for filename, count in [('common-performance', 224), ('hosted-performance', 56)]:
    data = read(filename)
    check(data['binaries']['B']['sha256'] == final, 'timing final binding ' + filename)
    check(data['affinity'] == ['taskset', '-c', '2'] and data['flags'] == ['-O0', '-c', '--stats'],
          'frozen timing policy ' + filename)
    check(len(data['runs']) == count, 'all observations ' + filename)
    observations += count
    for name, images in data['images'].items():
        check(images['A']['object_sha256'] == images['B']['object_sha256'], 'equal objects ' + name)
        if filename == 'common-performance':
            check(images['A']['executable_sha256'] == images['B']['executable_sha256'],
                  'equal executable ' + name)
        for mode in (['compile', 'runtime'] if filename == 'common-performance' else ['compile']):
            rows = [r for r in data['runs'] if r['workload'] == name and r.get('mode', 'compile') == mode]
            for block in range(7):
                check(''.join(r['label'] for r in rows if r['block'] == block) == ('AAAA' if block == 0 else 'ABBA'),
                      'noise/ABBA order %s/%s/%d' % (name, mode, block))
            summary = data['summary'][name]
            if filename == 'common-performance':
                summary = summary[mode]
            ratios = [statistics.mean(r['wall_s'] for r in rows if r['block'] == b and r['label'] == 'B') /
                      statistics.mean(r['wall_s'] for r in rows if r['block'] == b and r['label'] == 'A')
                      for b in range(1, 7)]
            check(ratios == summary['paired_ratios'] and statistics.median(ratios) == summary['paired_ratio_median'],
                  'recomputed paired summary ' + name + '/' + mode)
            aa = [r['wall_s'] for r in rows if r['block'] == 0]
            check([min(aa), max(aa)] == summary['AA_range_s'], 'noise spread ' + name + '/' + mode)
    for i, r in enumerate(data['runs']):
        check(r['status'] == 0 and r['peak_rss_kib'] > 0, 'sample result %s/%d' % (filename, i))
        if r.get('mode', 'compile') == 'compile':
            check(r['wall_s'] < 45, 'mandated timeout %s/%d' % (filename, i))

data = read('performance-image-binding')
check(data['compiler_sha256'] == final and len(data['images']) == 8 and len(data['launchers']) == 16,
      'affected image and launcher inventory')
for r in data['images']:
    check(r['status'] == 0 and r['wall_s'] < 45, 'affected-image final cost ' + r['workload'])
    check(sha(root / r['historical_record']) == r['historical_sha256'],
          'historical raw evidence preserved ' + r['workload'])
    if r['workload'].startswith(('vector', 'simd')):
        n = int(re.search(r'\d+$', r['workload'])[0])
        packed = r['workload'].startswith('simd')
        counters = {k: v for s in r['stderr'].splitlines() if s.startswith('{') for k, v in json.loads(s).items()}
        check(counters['semantic_specializations'] == counters['template_body_transitions'] == n,
              'one demanded specialization/body ' + r['workload'])
        check(counters['instructions'] == (284 if packed else 40)*n+53 and
              counters['native_instructions'] == (367 if packed else 41)*n+72,
              'linear work retained ' + r['workload'])
observations += len(data['images'])
for name in ['pa30/plan.md', 'pa30/audit.md']:
    s = (root / name).read_text()
    check('Last reviewed commit: `' + code + '`' in s, 'final review marker ' + name)
    check('Stage base commit: `' + binding['stage_base'] + '`' in s, 'stage base marker ' + name)
(out / 'verification.json').write_text(json.dumps(dict(passed=len(checks),
    current_workload_observations=observations, launcher_observations=16, checks=checks), indent=2)+'\n')
print(len(checks), 'checks passed;', observations, 'current workload observations + 16 launchers')
