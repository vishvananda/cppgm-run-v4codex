#!/usr/bin/env python3
"""Final independent PA28 audit gates and explicit controls. Logs stay in scratch."""
import hashlib,json,pathlib,re,subprocess,sys,time
root=pathlib.Path(__file__).resolve().parents[2]
out=pathlib.Path(sys.argv[1]).resolve();out.mkdir(parents=True,exist_ok=True)
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def git(*args):return subprocess.check_output(['git',*args],cwd=root,text=True).strip()
result=dict(stage_base=git('rev-parse','bec9389f'),audit_entry=git('rev-parse','03575afb'),reviewed_code=git('rev-parse','HEAD'),binary_sha256=sha(root/'dev/cppgm++'),checks=[])
def save():(out/'validation.json').write_text(json.dumps(result,indent=2)+'\n')
commands=[('stageTests',['make','test-pa28']),('throughStage',['make','test-report-through-pa28']),
 ('fileAudit',['perl','scripts/cppgm_file_audit.pl','--stage','pa28','--paths','dev/src'])]
for name,script in [('overrides','exception-override154.py'),('naming','naming151.py'),('exceptions','exceptions152.py'),('ownership','ownership152.py'),('primary','virtual-primary153.py'),('inspection','inspection153.py')]:
 commands.append((name,['python3','student.tests/pa28/'+script,str(out/name)]))
commands += [('templateTrace',['python3','student.tests/pa28/trace154.py',str(out/'template-trace'),str(out/'inspection/view')]),('diffCheck',['git','diff','--check'])]
for name,command in commands:
 log=out/(name+'.log');start=time.monotonic()
 with log.open('w') as stream:
  p=subprocess.run(command,cwd=root,stdout=stream,stderr=subprocess.STDOUT)
 result['checks'].append(dict(name=name,command=command,status=p.returncode,seconds=time.monotonic()-start,log=str(log),log_sha256=sha(log),tail=log.read_text()[-3500:]))
 save();assert not p.returncode,(name,log)
 print(name,'PASS',flush=True)
report=(out/'throughStage.log').read_text();assert '(4538 / 4538)' in report and len(re.findall(r'^===== pa\d+ =====$',report,re.M))==28
assert '(97 / 97)' in (out/'stageTests.log').read_text()
protected=['scripts','TESTING_AND_REFERENCES.md','reference-binaries']
for n in range(1,29):protected += [f'pa{n}/tests',f'pa{n}/scripts',f'pa{n}/Makefile']
changes=git('diff','--name-only','bec9389f','--',*protected).splitlines();assert not changes
inventory=git('ls-files','-s','--',*[f'pa{n}/tests' for n in range(1,29)])
result['course_tests']=dict(passed=4538,total=4538,stages=28,pa28=97)
result['coverage']=dict(anchors=len(list((root/'pa28/tests').rglob('*.t'))),tracked_contract_paths=len(inventory.splitlines()),sha256=hashlib.sha256(inventory.encode()).hexdigest(),changes=changes,reference_manifest_sha256=sha(root/'reference-binaries/manifest.tsv'),bundle_revision='c2f713cd70d06170632bfde3e75dd6fe1aa44d98')
paths=git('diff','--name-only','bec9389f','--','dev').splitlines()
result['implementation_hashes']={p:sha(root/p) for p in paths}
assert result['binary_sha256']==sha(root/'dev/cppgm++')
save()
