#!/usr/bin/env python3
"""Checkpoint audit gates with exact failure-set and protected-coverage checks."""
import hashlib,json,pathlib,re,subprocess,sys,time
root=pathlib.Path(__file__).resolve().parents[2]
out=pathlib.Path(sys.argv[1]).resolve();out.mkdir(parents=True,exist_ok=True)
entry='7dbb067893e16c136d4f79b1048092f7c1f8e044'
base='f833cf1ff361529147361cada33eca62e55330cf'
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def git(*args):return subprocess.check_output(['git',*args],cwd=root,text=True).strip()
def failures(t):return sorted(set(re.findall(r'(pa27/[^ :]+\.t): ERROR',t)))
prior=json.loads((root/'student.tests/pa27/evidence147/validation.json').read_text())['progress']['remaining']
assert len(prior)==1
result=dict(stage_base=base,entry_commit=entry,code_tip=git('rev-parse','HEAD'),binary_sha256=sha(root/'dev/cppgm++'),checks=[])
def save():(out/'validation.json').write_text(json.dumps(result,indent=2)+'\n')
commands=[
 ('priorThroughTests',['bash','-c','n=27; if [ "$n" -le 1 ]; then echo "===== ALL TESTS PASSED SUCCESSFULLY! (0/0) ====="; else make test-report-through-pa$((n - 1)); fi'],[0]),
 ('stageTests',['make','test-pa27'],[0,2]),
 ('throughStage',['make','test-report-through-pa27'],[0,2]),
 ('fileAudit',['perl','scripts/cppgm_file_audit.pl','--stage','pa27','--paths','dev/src'],[0]),
 *[(label,['python3',f'student.tests/pa27/{script}.py',str(out/script)],[0]) for label,script in [
  ('auditControls','audit148-controls'),('storageControls','storage-controls'),('declaratorControls','declarator-controls'),
  ('linkageControls','linkage-controls'),('tlsControls','tls-controls'),('objectControls','object-controls')]],
 ('diffCheck',['git','diff','--check'],[0])]
for name,args,statuses in commands:
 log=out/(name+'.log');start=time.perf_counter()
 with log.open('w') as stream:p=subprocess.run(args,cwd=root,stdout=stream,stderr=subprocess.STDOUT)
 result['checks'].append(dict(name=name,command=args,status=p.returncode,elapsed_s=time.perf_counter()-start,output_sha256=sha(log),output_path=str(log)))
 save();print(name,p.returncode,flush=True)
 assert p.returncode in statuses,(name,p.returncode)
stage=(out/'stageTests.log').read_text();now=failures(stage)
assert set(now)<=set(prior) and len(now)<=len(prior)
assert ('%d / 158 TESTS PASSED'%(158-len(now))) in stage and 'properties: PASS (3/3)' in stage
assert 'ALL TESTS PASSED SUCCESSFULLY! (4283 / 4283)' in (out/'priorThroughTests.log').read_text()
assert failures((out/'throughStage.log').read_text())==now
assert sha(root/'dev/cppgm++')==result['binary_sha256']
paths=[*[f'pa{n}/tests' for n in range(1,28)],'scripts']
assert not git('diff','--name-only',entry,'--',*paths)
inventory=git('ls-files',*paths).splitlines()
assert inventory==git('ls-tree','-r','--name-only',entry,'--',*paths).splitlines()
result['coverage']=dict(tracked_contract_paths=len(inventory),inventory_sha256=hashlib.sha256('\n'.join(inventory).encode()).hexdigest(),stage_anchors=len(list((root/'pa27/tests/general').glob('*.t'))),changes_since_entry=[])
result['progress']=dict(entry_failures=prior,current_failures=now,total=158,passed=158-len(now),additional_failures=[])
result['sources']={p:sha(root/p) for p in git('diff','--name-only',base,'--','dev').splitlines()}
result['status']='pass: earlier stages, file audit, progress and coverage preserved'
save();print(result['status'])
