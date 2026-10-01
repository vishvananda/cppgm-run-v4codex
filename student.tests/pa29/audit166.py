#!/usr/bin/env python3
"""Record accumulated audit166 after its validated code commit, before records commit."""
import hashlib,json,pathlib,re,subprocess,sys
root=pathlib.Path(__file__).resolve().parents[2];out=pathlib.Path(sys.argv[1]).resolve()
ev=root/'student.tests/pa29/evidence166';ev.mkdir(exist_ok=True)
base='cce8634c3c835cf6d5e8f4fa5fea0db213959718';entry='3036bd4ec4b1f8d315cf826437244de5236955bf'
def git(*args):return subprocess.check_output(['git',*args],cwd=root)
def sha(data):return hashlib.sha256(data).hexdigest()
def save(name,data):(ev/name).write_text(json.dumps(data,indent=None if 'performance' in name else 2)+'\n')
tip=git('rev-parse','HEAD').decode().strip();compiler=sha((root/'dev/cppgm++').read_bytes())
assert not git('diff','HEAD','--','dev')
commits=[]
for commit in git('rev-list','--reverse',base+'..'+tip).decode().splitlines():
 commits.append(dict(commit=commit,subject=git('show','-s','--format=%s',commit).decode().strip(),paths=git('diff-tree','--no-commit-id','--name-only','-r',commit).decode().splitlines(),implementation_diff_sha256=sha(git('show','--format=','--binary',commit,'--','dev')),full_diff_sha256=sha(git('show','--format=','--binary',commit))))
save('range.json',dict(stage_base='2734e5c67eaa7c0cf4bbbd510dba8d60f36d6543',previous_review=base,entry=entry,last_reviewed_commit=tip,commits=commits,combined_implementation_diff_sha256=sha(git('diff',base,tip,'--','dev'))))
def failures(path):return sorted(set(re.findall(r'(pa29/tests/[^\s:]+\.t): ERROR:',path.read_text())))
before=failures(out/'entry-stage.log');after=failures(out/'stage-final.log')
assert len(before)==53 and before==after and after==failures(out/'through-final.log')
assert '350 / 403' in (out/'stage-final.log').read_text()
assert '4538 / 4538' in (out/'prior-final.log').read_text()
assert '4888 / 4941' in (out/'through-final.log').read_text()
status=json.loads((out/'final-check-status.json').read_text())
assert status=={'make test-pa29':2,'make test-report-through-pa28':0,'make test-report-through-pa29':2,'perl scripts/cppgm_file_audit.pl --stage pa29 --paths dev/src':0}
assert 'File audit passed for pa29 with 4 warning(s).' in (out/'file-audit-final.log').read_text()
assert not git('diff',base,tip,'--','pa29/tests','pa29/scripts','scripts','Makefile','pa29/Makefile')
fixtures={}
for name in git('ls-files','pa29/tests').decode().splitlines():
 data=(root/name).read_bytes();assert data==git('show',base+':'+name);fixtures[name]=sha(data)
assert sum(n.endswith('.t') for n in fixtures)==403
save('coverage.json',dict(source_count=403,all_fixtures_and_sidecars_unchanged=True,fixture_sha256=fixtures,comparison_and_discovery_changes=[],reference_corrections=[]))
save('stage-delta.json',dict(entry=entry,code_tip=tip,total=403,before_pass=350,after_pass=350,entry_failures=before,final_failures=after,new_failures=[],resolved=[]))
checks={}
for version,folder in [('162','controls162-final'),('163','controls163-final'),('164','controls164-final'),('165','controls165-final'),('166','controls-final')]:
 data=json.loads((out/folder/'results.json').read_text());assert data['compiler_sha256']==compiler and all(r['passed'] for r in data['checks'])
 checks['controls'+version]=len(data['checks']);save('controls'+version+'.json',data)
for version,folder in [('161','inspection161-final'),('163','inspection163-final'),('165','inspection165-final'),('166','inspection-final')]:
 data=json.loads((out/folder/'inspection.json').read_text());assert data['compiler_sha256']==compiler
 assert all(r['passed'] for r in data['checks'] if isinstance(r,dict))
 checks['inspection'+version]=len(data['checks']);save('inspection'+version+'.json',data)
save('entry-regressions.json',json.loads((out/'entry-controls/results.json').read_text()))
logs={}
for name in ['stage-final.log','prior-final.log','through-final.log','file-audit-final.log']:
 p=out/name;logs[name]=dict(path=str(p),sha256=sha(p.read_bytes()),tail=p.read_text().splitlines()[-5:])
for name in ['cumulative','audit-cost','affected163','affected164','affected165','affected166']:
 data=json.loads((out/name/'performance.json').read_text());assert all(r['status']==0 for r in data['runs'])
 assert (data['binaries']['B']['sha256'] if 'binaries' in data else data['binary']['sha256'])==compiler
 assert len(data['runs'])==(224 if 'binaries' in data else 96)
 if 'binaries' in data:
  assert all(v['A']['object_sha256']==v['B']['object_sha256'] and v['A']['executable_sha256']==v['B']['executable_sha256'] for v in data['images'].values())
 save(name+'-performance.json',data)
manifest=json.loads((out/'performance-manifest.json').read_text());manifest['code_tip']=tip;save('performance-manifest.json',manifest)
save('affected-entry.json',json.loads((out/'affected-entry.json').read_text()))
for path in sorted((out/'preliminary').rglob('performance.json')):
 save('preliminary-'+path.parent.name+'-performance.json',json.loads(path.read_text()))
for name in ['performance-manifest.json','affected-entry.json']:
 save('preliminary-'+name,json.loads((out/'preliminary'/name).read_text()))
save('historical-performance-review.json',json.loads((out/'historical-performance-review.json').read_text()))
save('validation.json',dict(code_tip=tip,compiler_sha256=compiler,commands=status,stage_progress_preserved=True,checks=checks,logs=logs,coverage=403,final_failures=53,whole_stage='unfinished; no advancement'))
print(json.dumps(dict(code_tip=tip,commands=status,checks=checks,coverage=403,new_failures=0)),flush=True)
