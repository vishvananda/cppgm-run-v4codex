#!/usr/bin/env python3
"""Verify final bindings, preserved course coverage and every performance sample."""
import hashlib
import json
import pathlib
import statistics
import subprocess

root = pathlib.Path(__file__).resolve().parents[2]
evidence = root/'student.tests/pa31/evidence206'
old = root/'student.tests/pa31/evidence205'
binding = json.loads((evidence/'validation.json').read_text())
prior = json.loads((old/'binding.json').read_text())
checks = 0

def check(value, context):
    global checks
    assert value, context
    checks += 1

def sha(path):
    return hashlib.sha256(pathlib.Path(path).read_bytes()).hexdigest()

def output(args):
    return subprocess.check_output(args, cwd=root)

for path, digest in binding['sources'].items():
    check(sha(root/path) == digest, path)
for path, digest in binding['records'].items():
    check(sha(root/path) == digest, path)
check(not output(['git', 'diff', binding['reviewed_commit'], '--', 'dev']), 'reviewed implementation unchanged')
check(sha(root/'dev/cppgm++') == prior['binaries']['B']['sha256'], 'current compiler')
for path, digest in prior['sources'].items():
    # The old handoff bound its plan too; retain that historical binding while
    # the final plan owns the new review marker and acceptance record.
    data = output(['git', 'show', binding['reviewed_commit']+':'+path])
    check(hashlib.sha256(data).hexdigest() == digest, ('historical source', path))
for path, digest in prior['records'].items():
    check(sha(root/path) == digest, ('historical observations', path))
coverage = json.loads((old/'coverage.json').read_text())
for path, item in coverage['files'].items():
    original = output(['git', 'show', prior['stage_base']+':'+path])
    expected = original.replace(b'_Z1g', b'g') if path in coverage['corrected_sidecars'] else original
    check((root/path).read_bytes() == expected, ('preserved coverage', path))
check(len(list((root/'pa31/tests/link').glob('*.t'))) == 84, 'all stage fixtures')
changes = output(['git', 'diff', '--name-only', prior['stage_base'], '--', 'pa31/tests', 'pa31/scripts', 'scripts', 'pa31/Makefile']).decode().splitlines()
check(set(changes) == set(coverage['corrected_sidecars']), 'only proved reference corrections')

observations = launchers = 0
for folder, names, artifact in [
        (old, ['common-performance', 'owner-performance', 'hosted-performance'], pathlib.Path(prior['artifact_root'])),
        (evidence, ['common-performance', 'hosted-performance'], pathlib.Path(binding['artifact_root']))]:
    for name in names:
        r = json.loads((folder/(name+'.json')).read_text())
        directory = artifact/name
        check(r['flags'] == ['-O0', '-c', '--stats'], (name, 'flags'))
        check(r['affinity'] == ['taskset', '-c', '2'], (name, 'affinity'))
        for label in 'AB':
            check(sha(r['binaries'][label]['path']) == prior['binaries'][label]['sha256'] == r['binaries'][label]['sha256'], (name, label))
        for workload, images in r['images'].items():
            source = r['inputs'][workload]
            digest = source if isinstance(source, str) else source['sha256']
            check(sha(directory/(workload+'.cpp')) == digest, (name, workload, 'input'))
            equivalent = len(images) == 2
            if not equivalent:
                check(source['entry_failure']['status'] != 0, (workload, 'invalid baseline excluded'))
            for label, image in images.items():
                for suffix, key in [('.o', 'object_sha256'), ('', 'executable_sha256')]:
                    path = directory/(workload+label+suffix)
                    check(sha(path) == image[key], str(path))
                    size = sum(int(line.split()[1]) for line in output(['size', '-A', str(path)]).decode().splitlines()
                               if line.split() and line.split()[0].startswith('.text'))
                    check(size == image['object_text_bytes' if suffix else 'executable_text_bytes'], (str(path), 'text'))
            for mode in ['compile', 'runtime']:
                rows = [v for v in r['runs'] if v['workload'] == workload and v['mode'] == mode]
                expected = [(b, label) for b, order in enumerate(['AAAA']+['ABBA']*6 if equivalent else ['BBBB']*2) for label in order]
                check([(v['block'], v['label']) for v in rows] == expected, (workload, mode, 'order'))
                check(all(v['status'] == 0 and v['wall_s'] > 0 and v['peak_rss_kib'] > 0 for v in rows), (workload, mode, 'samples'))
                summary = r['summary'][workload][mode]
                if equivalent:
                    aa = [v['wall_s'] for v in rows if not v['block']]
                    ratios = [statistics.mean(v['wall_s'] for v in rows if v['block'] == b and v['label'] == 'B') /
                              statistics.mean(v['wall_s'] for v in rows if v['block'] == b and v['label'] == 'A') for b in range(1, 7)]
                    check(summary['AA_range_s'] == [min(aa), max(aa)], (workload, mode, 'noise'))
                    check(summary['paired_ratios'] == ratios and summary['paired_ratio_median'] == statistics.median(ratios)
                          and summary['paired_ratio_range'] == [min(ratios), max(ratios)], (workload, mode, 'pairs'))
                for label in images:
                    samples = [v for v in rows if v['label'] == label and (v['block'] or not equivalent)]
                    calculated = dict(median_s=statistics.median(v['wall_s'] for v in samples),
                        range_s=[min(v['wall_s'] for v in samples), max(v['wall_s'] for v in samples)],
                        peak_rss_kib=max(v['peak_rss_kib'] for v in samples))
                    check(summary[label] == calculated, (workload, mode, label, 'summary'))
                for v in rows:
                    if 'object_sha256' in v:
                        check(v['object_sha256'] == images[v['label']]['object_sha256'], (workload, 'timed object'))
                    for counters in v.get('phase_counters', []):
                        if 'inline_budget_work' in counters:
                            check(counters['inline_work'] <= counters['inline_budget_work'] <= 4194304
                                  and counters['inline_max_function_work'] <= 262144, (workload, 'pipeline budgets'))
        observations += len(r['runs'])
        launchers += len(r.get('launchers', []))
check(observations == 824 and launchers == 48, 'all old and new observations')

for name in ['controls', 'trace']:
    item = binding['inspection'][name]
    check(sha(item['path']) == item['sha256'], (name, 'raw inspection'))
    record = json.loads(pathlib.Path(item['path']).read_text())
    check(record['passed'] and record['compiler_sha256'] == prior['binaries']['B']['sha256'], (name, 'compiler binding'))
    for command in record['commands']:
        check((command['status'] != 0) == command.get('expected_rejection', False), (name, command['args']))
    check(len(record['commands']) == (46 if name == 'controls' else 104), (name, 'coverage'))
    if name == 'trace':
        directory = pathlib.Path(item['path']).parent
        for workload, image in record['images'].items():
            for suffix, field in [('.o', 'direct_object_sha256'), ('.rebuilt.o', 'rebuilt_object_sha256'),
                                  ('.lowir', 'lowir_sha256'), ('.mir', 'mir_sha256')]:
                check(sha(directory/(workload+suffix)) == image[field], (workload, field))
            command = next(c for c in record['commands'] if '--emit-lowir' in c['args']
                           and str(directory/(workload+'.lowir')) in c['args'])
            source = command['args'][command['args'].index('-o')-1]
            check(sha(source) == image['source_sha256'], (workload, 'trace source'))
            check(all(image[key] for key in ['text_identical', 'function_cfi_identical',
                      'named_relocations_identical', 'telemetry_identical']), (workload, 'typed boundary'))
for name, item in binding['checks'].items():
    check(item['exit_code'] == 0 and sha(item['log_path']) == item['log_sha256'], name)
check(binding['checks']['stage']['passed'] == 84 and binding['checks']['through']['passed'] == 5178, 'required suite counts')
check(binding['checks']['through']['stages'] == 31, 'required stage count')
for path in ['pa31/plan.md', 'pa31/audit.md']:
    check('Last reviewed commit: `'+binding['reviewed_commit']+'`' in (root/path).read_text(), (path, 'review marker'))
print(json.dumps(dict(passed=True, checks=checks, observations=observations, launchers=launchers)))
