#!/usr/bin/env python3
"""Verify the accumulated checkpoint audit against its frozen code tip."""
from pathlib import Path
import hashlib, json, re, statistics, subprocess

ROOT = Path(__file__).resolve().parents[2]
def git(*args):
    return subprocess.check_output(['git', *args], cwd=ROOT, text=True).strip()
def sha(path):
    return hashlib.sha256(Path(path).read_bytes()).hexdigest()
def failures(log):
    return set(re.findall(r'^(pa\d+/[^:]+): ERROR:', log, re.M))
def source_digest():
    h = hashlib.sha256()
    for name in sorted(git('ls-files', 'dev').splitlines()):
        h.update(name.encode()+b'\0'); h.update((ROOT/name).read_bytes()); h.update(b'\0')
    return h.hexdigest()

def performance(path, tip=None):
    p = json.loads(path.read_text())
    assert p['finished_utc'] and not p['source_diff']
    if tip:
        assert p['source_commit'] == tip
        assert p['binaries'][1]['sha256'] == sha(ROOT/'dev/cppgm++')
    for b in p['binaries']:
        assert sha(b['path']) == b['sha256']
    assert sha(p['backend']['path']) == p['backend']['sha256']
    assert p['flags'] == ['--emit-lowir', '-O0']
    for w in p['workloads'].values():
        assert hashlib.sha256(w['source'].encode()).hexdigest() == w['source_sha256']
        common = w.get('comparison', 'exact') == 'exact'
        if common:
            assert len(w['outputs']) == 2 and len({o['sha256'] for o in w['outputs']}) == 1
        else:
            assert w['entry_behavior']['compile_exit'] > 0
        for o in w['outputs']:
            assert sha(o['path']) == o['sha256']
            if 'native' in o:
                assert sha(o['native']['path']) == o['native']['sha256']
                assert o['native']['checked_exit'] == 0
        for phase in ('compiler', 'runtime'):
            if phase not in w:
                continue
            m = w[phase]
            assert [r['binary'] for r in m['warmups']] == ([0,1] if common else [1])
            assert [r['binary'] for r in m['observations']] == ([0]*4+[0,1,1,0]*4 if common else [1]*6)
            assert all(r['wall_s'] > 0 and r['rss_kib'] > 0 and r['checked_exit'] == 0
                       for r in m['warmups']+m['observations'])
            if common:
                rows = m['observations'][4:]
                ratios = [statistics.mean(r['wall_s'] for r in rows[j:j+4] if r['binary']==1)/
                          statistics.mean(r['wall_s'] for r in rows[j:j+4] if r['binary']==0)
                          for j in range(0,16,4)]
                assert ratios == m['paired_b_over_a']
                assert statistics.median(ratios) == m['median_b_over_a']
            for i in ([0,1] if common else [1]):
                rows = [r for r in m['observations'][4 if common else 0:] if r['binary']==i]
                assert m[str(i)]['median_wall_s'] == statistics.median(r['wall_s'] for r in rows)
                assert m[str(i)]['peak_rss_kib'] == max(r['rss_kib'] for r in rows)
        if 'runtime' in w and common:
            assert len({o['native']['sha256'] for o in w['outputs']}) == 1

def verify():
    e = json.loads((ROOT/'student.tests/pa17/checkpoint52-evidence.json').read_text())
    tip = e['code_commit']
    assert source_digest() == e['source_digest']
    assert not git('diff', tip, '--', 'dev')
    assert not git('diff', e['stage_base'], '--', *e['protected_paths'])
    subprocess.check_call(['git','merge-base','--is-ancestor',tip,'HEAD'],cwd=ROOT)
    assert git('rev-list','--reverse',e['review_start']+'..'+tip).splitlines() == e['reviewed_commits']
    assert git('diff','--name-only',e['review_start'],tip,'--','dev').splitlines() == e['reviewed_implementation_paths']
    for record in ('pa17/plan.md','pa17/audit.md'):
        text = (ROOT/record).read_text()
        assert f'Last reviewed commit: `{tip}`' in text
        assert f'Stage base commit: `{e["stage_base"]}`' in text
    logs = {}
    for name, log in e['logs'].items():
        assert sha(log['path']) == log['sha256']
        logs[name] = Path(log['path']).read_text()
    entry, baseline, final = [failures(logs[n]) for n in ('entry','baseline','stage')]
    assert len(entry) == 37 and baseline == entry and final <= entry
    assert sorted(entry) == e['entry_failures'] and sorted(final) == e['final_failures']
    assert f'TEST SUMMARY: {343-len(final)} / 343 TESTS PASSED' in logs['stage']
    assert e['logs']['stage']['exit_code'] == (2 if final else 0)
    assert e['logs']['prior']['exit_code'] == 0 and not failures(logs['prior'])
    assert 'ALL TESTS PASSED SUCCESSFULLY! (2266 / 2266)' in logs['prior']
    assert failures(logs['through']) == final
    assert f'TEST SUMMARY: {2609-len(final)} / 2609 TESTS PASSED' in logs['through']
    assert e['logs']['file-audit']['exit_code'] == 0 and 'File audit passed for pa17' in logs['file-audit']
    assert sorted(str(p.relative_to(ROOT)) for p in (ROOT/'pa17/tests').glob('*/*.t')) == e['course_tests']
    assert len(e['course_tests']) == 343
    groups = [r['path'] for g in e['remaining_groups'].values() for r in g['failures']]
    assert len(groups) == len(set(groups)) and set(groups) == final
    for file in e['evidence_files']:
        assert sha(ROOT/file['path']) == file['sha256']
    controls = json.loads((ROOT/e['controls']).read_text())
    counts = dict(entity=34,member=34,member_lowir=4,definition=18,audit=25,head=43,friend=47,name=56,checkpoint52=52)
    for name,count in counts.items():
        rows = controls[name]
        assert len(rows) == count and all(r['passed'] for r in rows)
        for r in rows:
            assert hashlib.sha256(r['source'].encode()).hexdigest() == r['source_sha256']
            if r.get('expected') == 'native':
                assert r['compiler_exit'] == r['backend_exit'] == r['native_exit'] == 0
            elif r.get('expected') == 'reject':
                assert r['compiler_exit'] > 0
    old = controls['entry_checkpoint52']
    assert len(old) == 52 and sum(not r['passed'] for r in old) == 17
    assert [(r['name'],r['source_sha256'],r['expected']) for r in old] == [
        (r['name'],r['source_sha256'],r['expected']) for r in controls['checkpoint52']]
    for p in e['performance']:
        performance(ROOT/p,tip)
    for p in e['historical_performance']:
        performance(ROOT/p)
    print(f'PA17 checkpoint audit verified through {tip[:8]}: {343-len(final)}/343, earlier 2266/2266, '
          f'{sum(counts.values())} controls, no new failures or coverage loss.')

if __name__ == '__main__':
    verify()
