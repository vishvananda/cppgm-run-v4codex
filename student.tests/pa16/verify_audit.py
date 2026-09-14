#!/usr/bin/env python3
"""Verify the accumulated PA16 audit record against artifacts and the code tip."""
from pathlib import Path
import hashlib, json, posixpath, re, subprocess
ROOT=Path(__file__).resolve().parents[2]
def sha(p): return hashlib.sha256(Path(p).read_bytes()).hexdigest()
def git(*args): return subprocess.check_output(['git',*args],cwd=ROOT)
def checked(item):
    p=Path(item['path'])
    if not p.is_absolute(): p=ROOT/p
    assert sha(p)==item['sha256'],str(p)
    return p
def historical_source(commit,path):
    while True:
        mode=git('ls-tree',commit,'--',path)
        data=git('show',commit+':'+path)
        if not mode.startswith(b'120000 '): return data
        path=posixpath.normpath(posixpath.join(posixpath.dirname(path),data.decode()))
def failure_set(path):
    return set(re.findall(r'(pa16/tests/[^:]+): ERROR:',path.read_text()))
report=json.loads((ROOT/'student.tests/pa16/audit-checkpoint.json').read_text())
tip=report['code_tip']
assert report['reviewed_commits']==git('rev-list','--reverse',report['review_start']+'..'+tip).decode().splitlines()
paths=git('diff','--name-only',report['review_start'],tip,'--','dev').decode().splitlines()
assert paths==[r['path'] for r in report['source_files']]
for entry in report['source_files']:
    checked(entry)
    assert hashlib.sha256(historical_source(tip,entry['path'])).hexdigest()==entry['sha256']
assert not git('diff',tip,'--','dev')
for entry in report['evidence']: checked(entry)
assert sha(ROOT/'dev/cppgm++')==report['compiler']['sha256']
checked(report['compiler'])
contract=['spec.md','TESTING_AND_REFERENCES.md','Makefile','scripts','reference-binaries']
for n in range(1,17):
    contract += [f'pa{n}/tests',f'pa{n}/scripts',f'pa{n}/Makefile',f'pa{n}/README.md',f'pa{n}/pa{n}.gram']
assert not git('diff',report['review_start'],'--',*contract)
assert len(list((ROOT/'pa16/tests').rglob('*.t')))==154
logs={name:checked(item) for name,item in report['logs'].items()}
entry,current=failure_set(logs['entry']),failure_set(logs['stage'])
assert len(entry)==61 and len(current)<=len(entry) and not current-entry
assert sorted(current)==report['remaining_failures']
assert 'TEST SUMMARY: 93 / 154 TESTS PASSED' in logs['stage'].read_text()
assert 'ALL TESTS PASSED SUCCESSFULLY! (2112 / 2112)' in logs['prior'].read_text()
assert 'TEST SUMMARY: 2205 / 2266 TESTS PASSED' in logs['through'].read_text()
assert failure_set(logs['through'])==current
assert not re.search(r'pa(?:[1-9]|1[0-5])/tests/[^:]+: ERROR:',logs['through'].read_text())
assert 'File audit passed for pa16 with 3 warning(s)' in logs['file_audit'].read_text()
for name,summary in {
    'controls':'16 native and 6 rejection audit controls passed',
    'scalar_execution':'21 native and 10 rejection controls passed',
    'floating':'19 native and 13 rejection controls passed',
    'storage':'27 native storage controls passed',
    'noexcept':'41 native, 8 rejection controls; failures: []',
    'validity':'24 native, 29 rejection controls; failures: []',
}.items(): assert summary in logs[name].read_text(),name
def campaign(record, current=False):
    c=json.loads(checked(record).read_text())
    assert c['finished_utc']
    for b in c['binaries']: checked(b)
    checked(c['backend'])
    if current:
        # Audit-tool-only commits may follow the measured implementation.
        # The production tree and frozen compiler must still match exactly.
        assert not git('diff',c['implementation_commit'],tip,'--','dev') and not c['source_diff']
        assert c['binaries'][1]['sha256']==report['compiler']['sha256']
    count=0
    for name,w in c['workloads'].items():
        assert sha(w['source_path'])==w['source_sha256']
        for output in w['outputs']:
            checked(output)
            if 'native' in output:
                checked(output['native'])
                assert output['native']['checked_exit']==0
        if w['comparison']=='exact':
            assert len({o['sha256'] for o in w['outputs']})==1
            if 'runtime' in w: assert len({o['native']['sha256'] for o in w['outputs']})==1
        if w['comparison']=='entry-rejected': assert w['entry_probe']['exit_code']!=0
        if 'entry_behavior' in w: assert w['entry_behavior']['exit_code']!=0
        for phase in ['compiler','runtime']:
            if phase not in w: continue
            samples=w[phase]
            common=len(w['outputs'])==2
            assert [r['binary'] for r in samples['warmups']]==([0,1] if common else [1])
            assert [r['binary'] for r in samples['observations']]==([0,0,0,0,0,1,1,0,0,1,1,0] if common else [1]*6)
            for row in samples['warmups']+samples['observations']:
                assert row['wall_s']>0 and row['rss_kib']>0
                assert row.get('checked_exit',0)==0
                count+=1
    return count
observations=sum(campaign(c,True) for c in report['campaigns'])
assert observations==report['observations']
for old in report['historical']:
    h=json.loads(checked(old).read_text())
    for source in h['source_files']:
        assert hashlib.sha256(historical_source(old['commit'],source['path'])).hexdigest()==source['sha256']
    for entry in list(h['logs'].values())+h.get('evidence_files',[])+h.get('retained_evidence',[]):
        path=entry['path']
        if not Path(path).is_absolute() and git('ls-tree',old['commit'],'--',path):
            assert hashlib.sha256(historical_source(old['commit'],path)).hexdigest()==entry['sha256'],path
        else: checked(entry)
    for c in h['campaigns']: campaign(c)
    if 'noise' in h: checked(h['noise'])
for path in ['pa16/plan.md','pa16/audit.md']:
    text=(ROOT/path).read_text()
    assert f'Last reviewed commit: `{tip}`' in text
    assert f"Stage base commit: `{report['review_start']}`" in text
assert not git('diff','--check')
print(f'PASS: full range/source provenance; 154 unchanged fixtures, {len(current)} failures with no lost passes; '
      f'prior 2112/2112; through 2205/2266; file audit; 148 native/66 rejection controls; '
      f'{observations} current observations and historical evidence; reviewed code markers')
