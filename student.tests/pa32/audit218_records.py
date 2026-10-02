#!/usr/bin/env python3
"""Bind/verify the final PA32 audit, preserving successful and failed evidence.
Usage: audit218_records.py bind|verify [--clean]
Artifacts remain in RALPH_ARTIFACT_DIR; no compiler outputs enter the checkout.
"""
import hashlib
import json
import os
import pathlib
import shutil
import subprocess
import sys
from audit214_history import measurement

ROOT = pathlib.Path(__file__).resolve().parents[2]
OUT = ROOT / 'student.tests/pa32/evidence218'
ART = pathlib.Path(os.environ['RALPH_ARTIFACT_DIR']) / 'pa32-218'
ENTRY = '5a767f2053b83c830c287263a58c732a05965b5c'
REVIEWED = 'dcd298afff36573014188a2c3cee826f2500cbf8'
BASE = 'e82bf4152fe8d6d68b9cd966655db0d8142cf81b'
PREVIOUS = '401519044a2c28130d4085b3bc7c3d0411b5dc8f'
LANES = ['common-o0', 'common-o1', 'common-o3', 'selfhost', 'scalar', 'objects',
         'loops', 'memory', 'ranges', 'context', 'source', 'debug']


def git(*args):
    return subprocess.check_output(['git', *args], cwd=ROOT, text=True).strip()


def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def read(path):
    return json.loads(path.read_text())


def write(path, value):
    path.write_text(json.dumps(value, indent=2) + '\n')


def summaries(data):
    if 'paired_ratio_median' in data['summary']:
        return {'component': {'compile': data['summary']}}
    if 'compile' in data['summary']:
        return {'kernel': data['summary']}
    return data['summary']


def report():
    lines = ['# Audit 218 frozen measurements', '',
             'Ratios are paired B/A medians with all six paired extrema in brackets. '
             'Times are separate sample medians; RSS is the maximum in the measured ABBA blocks. '
             'Every JSON retains the A/A calibration, all individual observations, flags, inputs, '
             'checked exit statuses and compiler/object/executable hashes.', '',
             'Affected baselines precede their owning implementation; B is the final reviewed compiler. '
             'Common controls compare entry/final at the same level. Scalar compares final O0/O1. '
             'Debug compares final O3 g0/line-table mode and measures the cost of required snapshots. '
             'The self-hosting component has no executable entry. These are diagnostics interpreted '
             'under the stage acceptance in [audit.md](../../../pa32/audit.md).', '']
    for lane in LANES:
        data = read(OUT / (lane + '.json'))
        lines += ['## ' + lane, '', '[All samples](' + lane + '.json)', '',
                  '| Workload | Compiler ratio [spread] | Compile ms A/B | Compiler RSS KiB A/B | Runtime ratio [spread] | Runtime ms A/B | Object text bytes A/B |',
                  '| --- | --- | --- | --- | --- | --- | --- |']
        for workload, modes in summaries(data).items():
            images = data['images'] if workload in ['kernel', 'component'] else data['images'][workload]
            def pair(mode, key, scale=1):
                return '/'.join(f'{mode[k][key]*scale:.2f}' if scale != 1 else str(mode[k][key]) for k in 'AB')
            def ratio(mode):
                return '%.3f [%.3f–%.3f]' % (mode['paired_ratio_median'], *mode['paired_ratio_range'])
            c = modes['compile']
            r = modes.get('runtime')
            text = '/'.join(str(images[k].get('object_text_bytes', images[k].get('text_bytes'))) for k in 'AB')
            lines.append('| ' + ' | '.join([workload, ratio(c), pair(c, 'median_s', 1000),
                         pair(c, 'peak_rss_kib'), ratio(r) if r else 'N/A',
                         pair(r, 'median_s', 1000) if r else 'N/A', text]) + ' |')
        lines.append('')
    (OUT / 'performance.md').write_text('\n'.join(lines))


def bind():
    OUT.mkdir(exist_ok=True)
    for lane in LANES:
        shutil.copyfile(ART / 'performance' / lane / 'performance.json', OUT / (lane + '.json'))
    for name, path in [('checks', 'final-checks/checks.json'), ('history', 'history.json'),
                       ('commands', 'performance/commands.json'),
                       ('historical-common-o1', 'common-o1/performance.json'),
                       ('historical-selfhost', 'selfhost/performance.json'),
                       ('context-bounds', 'final-checks/context-bounds.json'),
                       ('memory-bounds', 'final-checks/memory-bounds.json'),
                       ('range-bounds', 'final-checks/range-bounds.json')]:
        shutil.copyfile(ART / path, OUT / (name + '.json'))
    report()
    owners = {}
    for lane, prior in [('objects', 'evidence214/objects.json'), ('loops', 'evidence214/loops.json'),
                        ('memory', 'evidence214/memory.json'), ('ranges', 'evidence215/affected-runtime.json'),
                        ('context', 'evidence216/affected.json'), ('source', 'evidence217/affected.json')]:
        current = read(OUT / (lane + '.json'))
        old = read(OUT.parent / prior)
        matches = {key: image['B']['object_sha256'] == old['images'][key]['B']['object_sha256']
                   for key, image in current['images'].items()}
        assert all(matches.values()), (lane, matches)
        owners[lane] = dict(prior=prior, sha256=sha(OUT.parent / prior), identical_objects=matches)
    write(OUT / 'owner-objects.json', owners)
    paths = set(git('ls-files', 'dev').splitlines())
    paths.update(str(p.relative_to(ROOT)) for p in (OUT.parent).glob('*.py'))
    paths.update(['AGENTS.md', 'spec.md', 'TESTING_AND_REFERENCES.md', 'PROJECT_LAYOUT.md',
                  'pa32/README.md', 'pa32/plan.md', 'pa32/audit.md', 'pa32/reference-corrections.md',
                  'pa8/README.md', 'pa8/lowir.md'])
    files = [p for p in ART.rglob('*') if p.is_file() and p.name not in ['verify.log', 'history-verify.json']]
    files += [ART.parent / 'pa32-218-debug-entry.log']
    evidence = {p.name: sha(p) for p in sorted(OUT.iterdir()) if p.is_file() and p.name != 'binding.json'}
    binding = dict(entry=ENTRY, reviewed_commit=REVIEWED, previous_reviewed_commit=PREVIOUS, stage_base=BASE,
                   sources={p: sha(ROOT / p) for p in sorted(paths)}, evidence=evidence,
                   artifacts={str(p): sha(p) for p in sorted(files)},
                   fixture_trees={f'pa{i}/tests': git('rev-parse', f'HEAD:pa{i}/tests') for i in range(1, 33)},
                   optional_probes={'prior-debug': 'PA8 excludes source/optimization/native debug shape; same five failures at entry and final. Required PA32 debug passes.'},
                   final_observations=sum(len(read(OUT / (lane + '.json'))['runs']) for lane in LANES),
                   intermediate_observations=252, newly_verified_historical_observations=4564,
                   required_latest_checks=32, course_count=5397, stages=32,
                   fixture_delta_since_entry=[], unfinished_implementation=[], unaudited_handoffs=[])
    write(OUT / 'binding.json', binding)


def verify():
    binding = read(OUT / 'binding.json')
    assert binding['reviewed_commit'] == REVIEWED
    subprocess.run(['git', 'merge-base', '--is-ancestor', REVIEWED, 'HEAD'], cwd=ROOT, check=True)
    assert not git('diff', REVIEWED, '--', 'dev'), 'Implementation changed after review'
    assert not git('diff', ENTRY, '--', *binding['fixture_trees']), 'Course fixtures changed during audit'
    for name, digest in binding['sources'].items():
        assert sha(ROOT / name) == digest, name
    for name, digest in binding['evidence'].items():
        assert sha(OUT / name) == digest, name
    for name, digest in binding['artifacts'].items():
        assert sha(pathlib.Path(name)) == digest, name
    for path, tree in binding['fixture_trees'].items():
        assert git('rev-parse', 'HEAD:' + path) == tree, path
    for binary in ['cppgm++', 'lowiropt', 'lowir2native']:
        assert sha(ROOT / 'dev' / binary) == sha(ART / 'final' / binary), binary
    checks = read(OUT / 'checks.json')
    latest = {row['name']: row for row in checks}
    assert len(latest) == 33
    for row in checks:
        assert sha(pathlib.Path(row['log'])) == row['sha256']
    for name, row in latest.items():
        if name not in binding['optional_probes']:
            assert row['exit_code'] == 0, (name, row['exit_code'])
    assert latest['prior-debug']['exit_code'] == 2
    assert '(5397 / 5397)' in pathlib.Path(latest['through']['log']).read_text()
    assert 'File audit passed for pa32' in pathlib.Path(latest['file']['log']).read_text()
    assert 'object-roundtrip debuginfo: PASS (25/25)' in pathlib.Path(latest['debug']['log']).read_text()
    count = 0
    for lane in [*LANES, 'historical-common-o1', 'historical-selfhost']:
        data = read(OUT / (lane + '.json'))
        binaries = data.get('binaries', {'same': data.get('binary')})
        for binary in binaries.values():
            assert sha(pathlib.Path(binary['path'])) == binary['sha256']
        count += measurement(data)
        if lane in ['common-o0', 'common-o1', 'common-o3']:
            assert all(image['A']['object_sha256'] == image['B']['object_sha256'] for image in data['images'].values())
    assert count == binding['final_observations'] + binding['intermediate_observations'] == 2408
    commands = read(OUT / 'commands.json')
    assert [row['name'] for row in commands] == LANES
    assert all(row['exit_code'] == 0 for row in commands)
    for lane, owner in read(OUT / 'owner-objects.json').items():
        prior = OUT.parent / owner['prior']
        assert sha(prior) == owner['sha256']
        a, b = read(prior), read(OUT / (lane + '.json'))
        assert all(a['images'][key]['B']['object_sha256'] == val['B']['object_sha256'] for key, val in b['images'].items())
    # Historical records bind their own commits, not the current source files.
    subprocess.run([sys.executable, str(OUT.parent / 'audit218_history.py'), str(ART / 'history-verify.json')],
                   cwd=ROOT, check=True, stdout=subprocess.DEVNULL)
    assert read(ART / 'history-verify.json') == read(OUT / 'history.json')
    for doc in ['pa32/plan.md', 'pa32/audit.md']:
        content = (ROOT / doc).read_text()
        assert 'Stage base commit: ' + BASE in content
        assert 'Last reviewed commit: ' + REVIEWED in content
    if '--clean' in sys.argv:
        assert not git('status', '--short'), 'Worktree is not clean'
    print('PASS: 32 required checks; unchanged course fixtures; 2156 final + 252 intermediate '
          '+ 4564 historical observations; frozen sources/binaries/artifacts; complete review boundary')


if __name__ == '__main__':
    assert sys.argv[1] in ['bind', 'verify']
    if sys.argv[1] == 'bind':
        bind()
    verify()
