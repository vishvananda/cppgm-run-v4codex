#!/usr/bin/env python3
"""Verify PA17 checkpoint-audit evidence against its committed code baseline."""
from pathlib import Path
import hashlib, json, re, subprocess
ROOT=Path(__file__).resolve().parents[2]
def git(*args):return subprocess.check_output(['git',*args],cwd=ROOT,text=True).strip()
def sha(path):return hashlib.sha256(Path(path).read_bytes()).hexdigest()
def failures(text):return set(re.findall(r'^(pa\d+/[^:]+): ERROR:',text,re.M))
def source_digest():
    h=hashlib.sha256()
    for name in sorted(git('ls-files','dev').splitlines()):
        h.update(name.encode()+b'\0');h.update((ROOT/name).read_bytes());h.update(b'\0')
    return h.hexdigest()

def verify():
    e=json.loads((ROOT/'student.tests/pa17/audit-evidence.json').read_text())
    assert source_digest()==e['source_digest']
    assert not git('diff',e['code_commit'],'--','dev')
    assert not git('diff',e['stage_base'],'--',*e['protected_paths'])
    subprocess.check_call(['git','merge-base','--is-ancestor',e['code_commit'],'HEAD'],cwd=ROOT)
    assert git('rev-list','--reverse',e['stage_base']+'..'+e['code_commit']).splitlines()==e['reviewed_commits']
    for record in ['pa17/plan.md','pa17/audit.md']:
        text=(ROOT/record).read_text()
        assert f'Last reviewed commit: `{e["code_commit"]}`' in text
        assert f'Stage base commit: `{e["stage_base"]}`' in text
    logs={}
    for key,log in e['logs'].items():
        assert sha(log['path'])==log['sha256']
        logs[key]=Path(log['path']).read_text()
    entry,baseline,final=map(lambda key:failures(logs[key]),['entry','baseline','stage'])
    assert entry==baseline and final<=entry
    assert e['entry_failures']==sorted(entry) and e['final_failures']==sorted(final)
    assert len(entry)==95 and e['course_cases']==343
    assert len(list((ROOT/'pa17/tests').glob('*/*.t')))==343
    assert f'TEST SUMMARY: {343-len(final)} / 343 TESTS PASSED' in logs['stage']
    assert e['logs']['stage']['exit_code']==(2 if final else 0)
    assert e['logs']['prior']['exit_code']==0 and not failures(logs['prior'])
    assert 'ALL TESTS PASSED SUCCESSFULLY! (2266 / 2266)' in logs['prior']
    assert failures(logs['through'])==final
    assert f'TEST SUMMARY: {2609-len(final)} / 2609 TESTS PASSED' in logs['through']
    assert e['logs']['file_audit']['exit_code']==0 and 'File audit passed for pa17' in logs['file_audit']
    for file in e['evidence_files']:
        assert sha(ROOT/file['path'])==file['sha256']
    controls=json.loads((ROOT/e['controls']).read_text())
    for name,count in [('entity',34),('member',34),('member_lowir',4),('definition',18),('audit',25)]:
        rows=controls[name];assert len(rows)==count and all(r['passed'] for r in rows)
        for row in rows:
            if row.get('expected')=='native':assert row['compiler_exit']==row['backend_exit']==row['native_exit']==0
            if row.get('expected')=='reject':assert row['compiler_exit']!=0
            if 'source_sha256' in row:assert hashlib.sha256(row['source'].encode()).hexdigest()==row['source_sha256']
    for file in e['performance']:
        p=json.loads((ROOT/file).read_text())
        assert p['source_commit']==e['code_commit'] and not p['source_diff'] and p['finished_utc']
        assert p['binaries'][1]['sha256']==sha(ROOT/'dev/cppgm++')
        for b in p['binaries']:assert sha(b['path'])==b['sha256']
        assert sha(p['backend']['path'])==p['backend']['sha256']
        for w in p['workloads'].values():
            assert hashlib.sha256(w['source'].encode()).hexdigest()==w['source_sha256']
            assert len({o['sha256'] for o in w['outputs']})==1
            for out in w['outputs']:assert sha(out['path'])==out['sha256']
            for phase in ['compiler','runtime']:
                if phase not in w:continue
                m=w[phase]
                assert [r['binary'] for r in m['observations']]==[0]*4+[0,1,1,0]*4
                assert [r['binary'] for r in m['warmups']]==[0,1]
                assert all(r['checked_exit']==0 and r['wall_s']>0 and r['rss_kib']>0 for r in m['observations']+m['warmups'])
            if 'runtime' in w:
                assert len({o['native']['sha256'] for o in w['outputs']})==1
                for out in w['outputs']:assert sha(out['native']['path'])==out['native']['sha256']
    print(f'PA17 audit verified through {e["code_commit"][:8]}: {343-len(final)}/343; earlier 2266/2266; no new failures; coverage intact.')
if __name__=='__main__':verify()
