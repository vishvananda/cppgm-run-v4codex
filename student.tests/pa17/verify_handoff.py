#!/usr/bin/env python3
"""Verify current implementation evidence; this does not perform independent audit."""
from pathlib import Path
import hashlib, json, re, subprocess
ROOT=Path(__file__).resolve().parents[2]
def git(*args):return subprocess.check_output(['git',*args],cwd=ROOT,text=True).strip()
def sha(p):return hashlib.sha256(Path(p).read_bytes()).hexdigest()
def source_digest():
    h=hashlib.sha256()
    for name in sorted(git('ls-files','dev').splitlines()):
        h.update(name.encode()+b'\0');h.update((ROOT/name).read_bytes());h.update(b'\0')
    return h.hexdigest()
def failures(text):return set(re.findall(r'^(pa\d+/[^:]+): ERROR:',text,re.M))
def verify():
    e=json.loads((ROOT/'student.tests/pa17/handoff.json').read_text())
    assert source_digest()==e['source_digest']
    subprocess.check_call(['git','merge-base','--is-ancestor',e['source_commit'],'HEAD'],cwd=ROOT)
    assert not git('diff',e['source_commit'],'--','dev')
    assert not git('diff',e['stage_base'],'--',*e['protected_paths'])
    plan=(ROOT/'pa17/plan.md').read_text()
    for title,key in [('Stage base commit','stage_base'),('Last reviewed commit','last_reviewed_commit')]:
        assert f'{title}: `{e[key]}`' in plan
    for log in e['logs'].values():assert sha(log['path'])==log['sha256']
    logs={name:Path(log['path']).read_text() for name,log in e['logs'].items()}
    assert e['logs']['prior']['exit_code']==0 and not failures(logs['prior'])
    assert f'ALL TESTS PASSED SUCCESSFULLY! ({e["earlier_cases"]} / {e["earlier_cases"]})' in logs['prior']
    assert f'TEST SUMMARY: {e["stage_passed"]} / {e["course_cases"]} TESTS PASSED' in logs['stage']
    assert f'TEST SUMMARY: {e["stage_passed"]+e["earlier_cases"]} / {e["course_cases"]+e["earlier_cases"]} TESTS PASSED' in logs['through']
    entry,final=failures(logs['entry']),failures(logs['stage'])
    assert failures(logs['through'])==final and all(p.startswith('pa17/') for p in final)
    assert len(entry)==e['course_cases']-e['entry_passed'] and len(final)==e['course_cases']-e['stage_passed']
    delta=e['failure_delta']
    assert set(delta['entry_failures'])==entry and set(delta['final_failures'])==final
    assert set(delta['fixed'])==entry-final and not delta['new_failures'] and final<entry
    assert len(list((ROOT/'pa17/tests').glob('*/*.t')))==e['course_cases']==343
    grouped=[r['path'] for g in e['remaining_groups'].values() for r in g['failures']]
    assert len(grouped)==len(set(grouped)) and set(grouped)==final
    assert all(g['status']=='unfinished implementation' for g in e['remaining_groups'].values())
    assert not e['waivers'] and e['handoff_boundary'] and e['independent_review_questions']
    assert e['logs']['file_audit']['exit_code']==0 and 'File audit passed for pa17' in logs['file_audit']
    for report in e['controls']:
        path=ROOT/report['path'];assert sha(path)==report['sha256']
        rows=json.loads(path.read_text());assert len(rows)==report['count'] and all(r['passed'] for r in rows)
        for r in rows:
            if r.get('expected')=='native':assert r['compiler_exit']==r['backend_exit']==r['native_exit']==0
            if r.get('expected')=='reject':assert r['compiler_exit']!=0
    for artifact in e['control_harnesses']+[e['performance_summary'],e['prior_performance']]:
        assert sha(ROOT/artifact['path'])==artifact['sha256']
    assert sha(ROOT/e['performance'])==e['performance_sha256']
    perf=json.loads((ROOT/e['performance']).read_text())
    assert perf['source_commit']==e['source_commit'] and not perf['source_diff'] and perf['finished_utc']
    assert perf['binaries'][1]['sha256']==sha(ROOT/'dev/cppgm++')
    for binary in perf['binaries']:assert sha(binary['path'])==binary['sha256']
    for workload in perf['workloads'].values():
        assert hashlib.sha256(workload['source'].encode()).hexdigest()==workload['source_sha256']
        assert all(r['checked_exit']==0 for r in workload['compiler']['observations'])
        if workload['comparison']=='exact':assert len({r['sha256'] for r in workload['outputs']})==1
        else:assert workload['entry_behavior']['compile_exit']!=0
        for out in workload['outputs']:assert sha(out['path'])==out['sha256']
        for phase in ['compiler','runtime']:
            if phase not in workload:continue
            measured=workload[phase]
            expected=[0]*4+[0,1,1,0]*4 if '0' in measured else [1]*6
            assert [r['binary'] for r in measured['observations']]==expected
            assert all(r['wall_s']>0 and r['rss_kib']>0 for r in measured['observations'])
        if 'runtime' in workload:
            assert all(r['checked_exit']==0 for r in workload['runtime']['observations'])
            assert len({r['native']['sha256'] for r in workload['outputs']})==1
            for out in workload['outputs']:assert sha(out['native']['path'])==out['native']['sha256']
    print(f'PA17 handoff verified: {len(entry-final)} fixed, no regressions; {len(final)} implementation failures remain. Independent audit pending.')
if __name__=='__main__':verify()
