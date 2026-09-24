#!/usr/bin/env python3
"""Verify loop 54 handoff evidence without certifying the independent audit."""
from pathlib import Path
import hashlib, json, re, statistics, subprocess

ROOT = Path(__file__).resolve().parents[2]

def git(*args):
    return subprocess.check_output(['git', *args], cwd=ROOT, text=True).strip()

def sha(path):
    return hashlib.sha256(Path(path).read_bytes()).hexdigest()

def source_digest():
    h = hashlib.sha256()
    for name in sorted(git('ls-files', 'dev').splitlines()):
        h.update(name.encode()+b'\0'); h.update((ROOT/name).read_bytes()); h.update(b'\0')
    return h.hexdigest()

def failures(log):
    return set(re.findall(r'^(pa\d+/[^:]+): ERROR:', log, re.M))

def performance(path, tip):
    p = json.loads(path.read_text())
    assert p['finished_utc'] and p['source_commit'] == tip and not p['source_diff']
    assert p['flags'] == ['--emit-lowir', '-O0'] and len(p['workloads']) == 17
    assert p['binaries'][1]['sha256'] == sha(ROOT/'dev/cppgm++')
    for b in p['binaries']: assert sha(b['path']) == b['sha256']
    assert sha(p['backend']['path']) == p['backend']['sha256']
    assert sha(ROOT/'student.tests/pa17/publication_benchmark.py') == p['harness_sha256']
    for w in p['workloads'].values():
        assert hashlib.sha256(w['source'].encode()).hexdigest() == w['source_sha256']
        common = w['comparison'] != 'entry-rejected'
        assert len(w['outputs']) == (2 if common else 1)
        if w['comparison'] == 'exact': assert len({o['sha256'] for o in w['outputs']}) == 1
        if not common: assert w['entry_behavior']['compile_exit'] > 0
        for o in w['outputs']:
            assert sha(o['path']) == o['sha256']
            if 'native' in o:
                assert sha(o['native']['path']) == o['native']['sha256'] and o['native']['checked_exit'] == 0
        for phase in ['compiler', 'runtime']:
            if phase not in w: continue
            m = w[phase]
            assert [r['binary'] for r in m['warmups']] == ([0, 1] if common else [1])
            assert [r['binary'] for r in m['observations']] == ([0]*4+[0, 1, 1, 0]*4 if common else [1]*6)
            assert all(r['wall_s'] > 0 and r['rss_kib'] > 0 and r['checked_exit'] == 0 for r in m['warmups']+m['observations'])
            if common:
                rows = m['observations'][4:]
                ratios = [statistics.mean(r['wall_s'] for r in rows[j:j+4] if r['binary'] == 1)/
                          statistics.mean(r['wall_s'] for r in rows[j:j+4] if r['binary'] == 0) for j in range(0, 16, 4)]
                assert ratios == m['paired_b_over_a'] and statistics.median(ratios) == m['median_b_over_a']
            for i in ([0, 1] if common else [1]):
                rows = [r for r in m['observations'][4 if common else 0:] if r['binary'] == i]
                assert m[str(i)]['median_wall_s'] == statistics.median(r['wall_s'] for r in rows)
                assert m[str(i)]['peak_rss_kib'] == max(r['rss_kib'] for r in rows)
        if 'runtime' in w and w['comparison'] == 'exact':
            assert len({o['native']['sha256'] for o in w['outputs']}) == 1
    assert p['workloads']['union-zero-runtime']['comparison'] == 'native-results'
    follow = json.loads((ROOT/'student.tests/pa17/publication-recheck.json').read_text())
    assert follow['finished_utc'] and follow['campaign_sha256'] == sha(path)
    assert follow['binaries'] == p['binaries'] and len(follow['workloads']) == 6
    assert follow['harness_sha256'] == sha(ROOT/'student.tests/pa17/publication_recheck.py')
    for name, w in follow['workloads'].items():
        assert w['source_sha256'] == p['workloads'][name]['source_sha256']
        for phase in ['compiler', 'runtime']:
            if phase not in w: continue
            m = w[phase]
            assert [r['binary'] for r in m['warmups']] == [0, 1]
            assert [r['binary'] for r in m['observations']] == [0]*4+[0, 1, 1, 0]*4
            assert all(r['checked_exit'] == 0 and r['wall_s'] > 0 and r['rss_kib'] > 0 for r in m['observations'])
            rows = m['observations'][4:]
            ratios = [statistics.mean(r['wall_s'] for r in rows[j:j+4] if r['binary'] == 1)/
                      statistics.mean(r['wall_s'] for r in rows[j:j+4] if r['binary'] == 0) for j in range(0, 16, 4)]
            assert ratios == m['paired_b_over_a'] and statistics.median(ratios) == m['median_b_over_a']

def verify():
    e = json.loads((ROOT/'student.tests/pa17/publication-handoff.json').read_text()); tip = e['code_commit']
    assert source_digest() == e['source_digest'] and not git('diff', tip, '--', 'dev')
    subprocess.check_call(['git', 'merge-base', '--is-ancestor', tip, 'HEAD'], cwd=ROOT)
    assert not git('diff', e['stage_base'], '--', *e['protected_paths'])
    for label, key in [('Stage base commit', 'stage_base'), ('Last reviewed commit', 'last_reviewed_commit')]:
        assert f'{label}: `{e[key]}`' in (ROOT/'pa17/plan.md').read_text()
        assert f'{label}: `{e[key]}`' in (ROOT/'pa17/audit.md').read_text()
    logs = {}
    for name, log in e['logs'].items():
        assert sha(log['path']) == log['sha256']; logs[name] = Path(log['path']).read_text()
    entry, final = failures(logs['entry']), failures(logs['stage'])
    assert len(entry) == 28 and len(final) == 22 and final < entry
    assert sorted(entry-final) == e['resolved_failures'] and sorted(final) == e['remaining_failures']
    assert 'TEST SUMMARY: 321 / 343 TESTS PASSED' in logs['stage'] and e['logs']['stage']['exit_code'] == 2
    assert not failures(logs['prior']) and 'ALL TESTS PASSED SUCCESSFULLY! (2266 / 2266)' in logs['prior']
    assert e['logs']['prior']['exit_code'] == 0
    assert failures(logs['through']) == final and 'TEST SUMMARY: 2587 / 2609 TESTS PASSED' in logs['through']
    assert 'File audit passed for pa17 with 3 warning(s).' in logs['file_audit'] and e['logs']['file_audit']['exit_code'] == 0
    assert e['logs']['stage_progress']['exit_code'] == 0 and 'PASS' in logs['stage_progress']
    assert sorted(str(p.relative_to(ROOT)) for p in (ROOT/'pa17/tests').glob('*/*.t')) == e['course_tests']
    assert len(e['course_tests']) == 343
    grouped = [c['path'] for group in e['remaining_groups'].values() for c in group['failures']]
    assert len(grouped) == len(set(grouped)) and set(grouped) == final
    assert all(g['status'] == 'unfinished implementation' for g in e['remaining_groups'].values())
    for artifact in e['evidence_files']: assert sha(ROOT/artifact['path']) == artifact['sha256']
    controls = json.loads((ROOT/'student.tests/pa17/publication-controls.json').read_text())
    assert sum(map(len, controls.values())) == 391 and len(controls['publication']) == 47
    for rows in controls.values():
        for r in rows:
            assert r['passed']
            if r.get('expected') == 'native': assert r['compiler_exit'] == r['backend_exit'] == r['native_exit'] == 0
            if r.get('expected') == 'reject': assert r['compiler_exit'] > 0
    entry_controls = json.loads((ROOT/'student.tests/pa17/publication-entry-controls.json').read_text())
    assert len(entry_controls) == 47 and sum(not r['passed'] for r in entry_controls) == e['new_control_entry_failures']
    performance(ROOT/'student.tests/pa17/publication-performance.json', tip)
    trace = json.loads((ROOT/'student.tests/pa17/publication-trace.json').read_text())
    for key in ['source', 'lowir', 'native']: assert sha(trace[key]['path']) == trace[key]['sha256']
    assert trace['native_exit'] == 0 and trace['telemetry'] and trace['disassembly']
    assert not e['reference_corrections'] and not e['waivers']
    assert e['handoff_boundary'] and e['independent_review_questions']
    print('PA17 implementation handoff verified: 315 -> 321/343, six original failures fixed; 391 controls pass. 22 implementation failures and independent review remain.')

if __name__ == '__main__': verify()
