#!/usr/bin/env python3
"""Record the complete reviewed range and independently verify checkpoint gates.

Run after the cohesive code commit and before the records-only commit:
  python3 student.tests/pa29/audit162.py ARTIFACT_DIR
"""
import hashlib,json,pathlib,re,subprocess,sys
root=pathlib.Path(__file__).resolve().parents[2]
out=pathlib.Path(sys.argv[1]).resolve();ev=root/'student.tests/pa29/evidence162';ev.mkdir(exist_ok=True)
base='1ab3499d7046daf5c298d958a8770b413edb3615'
entry='9662716b8a8aa0bef94f5a293d7900b28c42701c'
def git(*args):return subprocess.check_output(['git',*args],cwd=root)
def sha(data):return hashlib.sha256(data).hexdigest()
def save(name,data):(ev/name).write_text(json.dumps(data,indent=2)+'\n')
tip=git('rev-parse','HEAD').decode().strip();compiler=sha((root/'dev/cppgm++').read_bytes())
assert not git('diff','HEAD','--','dev'), 'commit implementation before recording review'
commits=[]
for commit in git('rev-list','--reverse',base+'..'+tip).decode().splitlines():
 diff=git('show','--format=','--binary',commit,'--','dev')
 commits.append(dict(commit=commit,subject=git('show','-s','--format=%s',commit).decode().strip(),
  paths=git('diff-tree','--no-commit-id','--name-only','-r',commit).decode().splitlines(),
  implementation_diff_sha256=sha(diff),full_diff_sha256=sha(git('show','--format=','--binary',commit))))
save('range.json',dict(stage_base='2734e5c67eaa7c0cf4bbbd510dba8d60f36d6543',previous_review=base,entry=entry,
 last_reviewed_commit=tip,commits=commits,combined_implementation_diff_sha256=sha(git('diff',base,tip,'--','dev'))))
def failures(path):return sorted(set(re.findall(r'(pa29/tests/[^\s:]+\.t): ERROR:',path.read_text())))
before=failures(out/'entry-stage.log');after=failures(out/'stage-final.log')
assert len(before)==65 and before==after
assert '338 / 403' in (out/'stage-final.log').read_text()
assert '4538 / 4538' in (out/'prior-final.log').read_text()
assert '4876 / 4941' in (out/'through-final.log').read_text()
assert failures(out/'through-final.log')==before
status=json.loads((out/'final-check-status.json').read_text())
assert status=={'make test-pa29':2,'make test-report-through-pa28':0,'make test-report-through-pa29':2}
assert 'File audit passed for pa29 with 4 warning(s).' in (out/'file-audit-final.log').read_text()
assert not git('diff',base,tip,'--','pa29/tests','pa29/scripts','scripts','Makefile','pa29/Makefile')
fixtures={}
for name in git('ls-files','pa29/tests').decode().splitlines():
 data=(root/name).read_bytes();assert data==git('show',base+':'+name)
 fixtures[name]=sha(data)
assert sum(name.endswith('.t') for name in fixtures)==403
save('coverage.json',dict(source_count=403,all_fixtures_and_sidecars_unchanged=True,fixture_sha256=fixtures,
 comparison_and_discovery_changes=[],reference_corrections=[]))
save('stage-delta.json',dict(entry=entry,code_tip=tip,total=403,before_pass=338,after_pass=338,
 entry_failures=before,final_failures=after,new_failures=[],resolved=[]))
controls={}
for name,folder in [('inherited158','controls158-final'),('inherited159','controls159-final'),
 ('inherited160','controls160-final'),('inherited161','controls161-final'),('controls','controls-complete')]:
 data=json.loads((out/folder/'results.json').read_text())
 assert data['compiler_sha256']==compiler and all(row['passed'] for row in data['checks']),name
 controls[name]=dict(passed=len(data['checks']),total=len(data['checks']))
 save(name+'.json',data)
for name,folder in [('inspection159','inspection159-final'),('inspection160','inspection160-final'),
 ('inspection161','inspection161-final'),('inspection','inspection-complete')]:
 data=json.loads((out/folder/'inspection.json').read_text());assert data['compiler_sha256']==compiler
 save(name+'.json',data)
save('entry-regressions.json',json.loads((out/'entry-controls/results.json').read_text()))
logs={}
for name in ['stage-final.log','prior-final.log','through-final.log','file-audit-final.log']:
 path=out/name;logs[name]=dict(path=str(path),sha256=sha(path.read_bytes()),tail=path.read_text().splitlines()[-5:])
save('validation.json',dict(code_tip=tip,compiler_sha256=compiler,commands=status,file_audit_status=0,
 stage_progress_preserved=True,controls=controls,logs=logs))
print(json.dumps(dict(code_tip=tip,commands=status,controls=controls,coverage=403,new_failures=0)),flush=True)
