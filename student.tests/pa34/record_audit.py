#!/usr/bin/env python3
"""Bind the independent audit after every command has terminated successfully.

Usage: record_audit.py ARTIFACT_DIRECTORY OUTPUT_JSON
Historical bindings remain immutable; changed/moved sources get new identities.
"""
import hashlib
import json
import pathlib
import re
import subprocess
import sys

root = pathlib.Path(__file__).resolve().parents[2]
artifacts = pathlib.Path(sys.argv[1]).resolve()
target = pathlib.Path(sys.argv[2]).resolve()


def sha(path):
    digest = hashlib.sha256()
    with path.open('rb') as source:
        for chunk in iter(lambda: source.read(1024 * 1024), b''):
            digest.update(chunk)
    return digest.hexdigest()


def run(args, cwd=root):
    return subprocess.check_output(args, cwd=cwd, text=True).strip()


def expand(cwd, expression):
    return run(['make', '-s', '--no-print-directory', '--eval',
                'audit-value: ; @echo ' + expression, 'audit-value'], cwd).split()


checks = json.loads((artifacts / 'final-checks.json').read_text())
required = {'fileAudit', 'pa34HostThrough33', 'pa34SelfThroughPa5',
            'pa34PptokenInception', 'pa34Inception', 'selfThrough33'}
assert {r['name'] for r in checks} == required and len(checks) == len(required)
assert all(r['status'] == 0 for r in checks)
for row in checks:
    log = artifacts / (row['name'] + '-final.log')
    row.update(log=str(log), sha256=sha(log))
supplemental = json.loads((artifacts / 'supplemental-checks.json').read_text())
assert len(supplemental) == 25 and all(r['status'] == 0 for r in supplemental)
result = dict(stage_base='99ea7d9d', review_entry='5a90033a', checks=checks,
              source_sets={}, objects={}, binaries={}, sources={}, artifacts={})
cases = {}
for line in (artifacts / 'selfThrough33-final.log').read_text().splitlines():
    match = re.fullmatch(r'pa(\d+) .*: PASS \((\d+)/(\d+)\)', line)
    if match:
        stage, passed, total = map(int, match.groups())
        assert passed == total
        cases[stage] = cases.get(stage, 0) + passed
assert set(cases) == set(range(1, 34))
host = re.search(r'ALL TESTS PASSED SUCCESSFULLY! \((\d+) / (\d+)\)',
                 (artifacts / 'pa34HostThrough33-final.log').read_text())
assert host and int(host[1]) == int(host[2]) == sum(cases.values())
result['self_stage_cases'] = cases
for tool in ('pptoken', 'posttoken', 'ppexpr', 'preproc', 'abimangle',
             'lowir', 'lowiropt', 'lowir2native', 'cppgm++'):
    seed = expand(root / 'dev', '$(call frontend_obj_basenames,' + tool + ')')
    self = expand(root / 'pa34', '$(call inception_checkpoint_source_ids,' + tool + ')')
    assert seed == self and len(seed) == len(set(seed)), tool
    assert all((root / 'dev/src' / (name + '.cpp')).is_file() for name in seed)
    result['source_sets'][tool] = seed
for tool in ('cppgm++', 'pptoken'):
    inputs = expand(root / 'pa34', '$(call inception_link_inputs,' + tool + ')')
    hashes = {}
    for file in inputs:
        a = (root / 'pa34' / file).resolve()
        relative = a.relative_to(root / 'obj/pa34/selfhost')
        b = root / 'obj/pa34/inception' / relative
        assert sha(a) == sha(b), relative
        hashes[str(relative)] = sha(a)
    result['objects'][tool] = hashes
    a, b = root / 'pa34' / (tool + '-self'), root / 'pa34' / (tool + '-inception')
    assert sha(a) == sha(b), tool
for file in ('dev/cppgm++', 'pa34/cppgm++-self', 'pa34/cppgm++-inception',
             'pa34/pptoken-self', 'pa34/pptoken-inception'):
    p = root / file
    result['binaries'][file] = dict(sha256=sha(p), bytes=p.stat().st_size)
old = json.loads((root / 'student.tests/pa34/evidence/binding.json').read_text())
assert result['objects']['cppgm++'] == old['objects']
assert result['binaries'] == old['binaries']
files = set(run(['git', 'ls-files', '--', 'dev', 'pa34', 'student.tests/pa34',
                 'AGENTS.md', 'spec.md', 'TESTING_AND_REFERENCES.md', 'PROJECT_LAYOUT.md',
                 'Makefile', 'pa33/Makefile', 'reference-binaries/manifest.tsv',
                 'scripts/run_with_timeout.pl']).splitlines())
files.update(run(['git', 'diff', '--name-only', '99ea7d9d', '--', 'student.tests',
                  'pa29/tests/preproc/300-has-builtin.ref']).splitlines())
files.update(run(['git', 'ls-files', '--others', '--exclude-standard', '--', 'student.tests']).splitlines())
files.update(('student.tests/pa32/selfhost_performance.py', 'student.tests/pa32/common_levels.py',
              'student.tests/pa33/audit-trace.cpp', 'student.tests/pa26/evidence144/common-performance.json',
              'obj/dev/generated/builtin_host_config.h', 'obj/pa34/generated/builtin_host_config.h'))
for file in sorted(files):
    p = root / file
    if p.is_file() and p.resolve() != target:
        result['sources'][file] = sha(p)
assert result['sources']['obj/dev/generated/builtin_host_config.h'] == result['sources']['obj/pa34/generated/builtin_host_config.h']
for p in sorted(artifacts.rglob('*')):
    if p.is_file() and p.resolve() != target:
        result['artifacts'][str(p.relative_to(artifacts))] = dict(sha256=sha(p), bytes=p.stat().st_size)
target.write_text(json.dumps(result, indent=2) + '\n')
print('Nine source sets match; 420 compiler and', len(result['objects']['pptoken']),
      'pptoken object pairs match; successful audit evidence bound')
