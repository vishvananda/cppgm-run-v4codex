#!/usr/bin/env python3
"""Sequential required reports, exact entry-failure comparison and coverage audit."""
import hashlib,json,pathlib,re,subprocess,sys,time
root=pathlib.Path(__file__).resolve().parents[2]
out=pathlib.Path(sys.argv[1]).resolve();out.mkdir(parents=True,exist_ok=True)
entry='8d595af6b91289109cae36554a8e8e8551192873'
stage_base='f833cf1ff361529147361cada33eca62e55330cf'
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def git(*args):return subprocess.check_output(['git',*args],cwd=root).decode().strip()
def failures(text):return sorted(set(re.findall(r'(pa27/[^ :]+\.t): ERROR',text)))
before=json.loads((root/'student.tests/pa27/evidence146/validation.json').read_text())['progress']['remaining']
assert len(before)==3
result=dict(stage_base=stage_base,last_reviewed_commit=stage_base,entry_commit=entry,implementation_commit=git('rev-parse','HEAD'),binary_sha256=sha(root/'dev/cppgm++'),checks=[])
commands=[
 ('priorThroughTests',['make','test-report-through-pa26'],0),
 ('stageTests',['make','test-pa27'],2),
 ('throughStage',['make','test-report-through-pa27'],2),
 ('fileAudit',['perl','scripts/cppgm_file_audit.pl','--stage','pa27','--paths','dev/src'],0),
 ('personalStorage',['python3','student.tests/pa27/storage-controls.py',str(out/'storage')],0),
 ('personalDeclarators',['python3','student.tests/pa27/declarator-controls.py',str(out/'declarators')],0),
 ('personalLinkage',['python3','student.tests/pa27/linkage-controls.py',str(out/'linkage')],0),
 ('personalTLS',['python3','student.tests/pa27/tls-controls.py',str(out/'tls')],0),
 ('inheritedObjects',['python3','student.tests/pa27/object-controls.py',str(out/'objects')],0),
 ('diffCheck',['git','diff','--check'],0),
]
for name,args,status in commands:
 log=out/(name+'.log');start=time.perf_counter()
 with log.open('w') as stream:p=subprocess.run(args,cwd=root,stdout=stream,stderr=subprocess.STDOUT)
 result['checks'].append(dict(name=name,command=args,status=p.returncode,elapsed_s=time.perf_counter()-start,output_path=str(log),output_sha256=sha(log)))
 print(name,p.returncode,flush=True)
 assert p.returncode==status,(name,p.returncode)
stage=(out/'stageTests.log').read_text();now=failures(stage)
assert '157 / 158 TESTS PASSED' in stage and 'properties: PASS (3/3)' in stage
assert set(now)<set(before) and len(now)==1
assert 'ALL TESTS PASSED SUCCESSFULLY! (4283 / 4283)' in (out/'priorThroughTests.log').read_text()
through=(out/'throughStage.log').read_text()
assert failures(through)==now and '4440 / 4441 TESTS PASSED' in through
assert sha(root/'dev/cppgm++')==result['binary_sha256']
result['progress']=dict(entry_pass=155,current_pass=157,total=158,entry_fail=3,current_fail=1,resolved=sorted(set(before)-set(now)),remaining=now,new_failures=[])
paths=[*[f'pa{n}/tests' for n in range(1,28)],'scripts']
assert not git('diff','--name-only',entry,'--',*paths)
inventory=git('ls-files',*paths).splitlines()
assert inventory==git('ls-tree','-r','--name-only',entry,'--',*paths).splitlines()
result['coverage']=dict(tracked_contract_paths=len(inventory),inventory_sha256=hashlib.sha256('\n'.join(inventory).encode()).hexdigest(),pa27_anchors=len(list((root/'pa27/tests/general').glob('*.t'))),source_reference_and_comparison_changes=[])
result['sources']={p:sha(root/p) for p in git('diff','--name-only',entry,'--','dev').splitlines()}
result['status']='validated incomplete implementation handoff; whole-stage audit remains pending'
(out/'validation.json').write_text(json.dumps(result,indent=2)+'\n')
print('PA27 progress: 3 -> 1 failure; earlier PAs and file audit pass.')
