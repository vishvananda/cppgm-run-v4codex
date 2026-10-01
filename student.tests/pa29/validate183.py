#!/usr/bin/env python3
"""Required PA29 gates; immutable course coverage and exact entry failure delta."""
import pathlib,subprocess,json,hashlib,sys,collections
root=pathlib.Path(__file__).resolve().parents[2];out=root/'student.tests/pa29/evidence183';out.mkdir(exist_ok=True)
scratch=pathlib.Path(sys.argv[1]).resolve();scratch.mkdir(parents=True,exist_ok=True)
entry='a2ce2670669e0ef392d61b1e1e4aeb6f2e302467'
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def save(n,v):(out/(n+'.json')).write_text(json.dumps(v,indent=2)+'\n')
paths=subprocess.check_output(['git','ls-files','--cached','--others','--exclude-standard','dev'],text=True).splitlines()
source={p:sha(root/p) for p in paths};binary=sha(root/'dev/cppgm++')
results=[]
commands=[('stage',['make','test-pa29']),('prior',['bash','-c','n=29; if [ "$n" -le 1 ]; then echo \'===== ALL TESTS PASSED SUCCESSFULLY! (0/0) =====\'; else make test-report-through-pa$((n - 1)); fi']),('through',['make','test-report-through-pa29']),('file-audit',['perl','scripts/cppgm_file_audit.pl','--stage','pa29','--paths','dev/src'])]
for name,args in commands:
 log=scratch/(name+'-final.log')
 with log.open('w') as f:p=subprocess.run(args,cwd=root,stdout=f,stderr=subprocess.STDOUT)
 text=log.read_text();results.append(dict(command=args,status=p.returncode,log=str(log),sha256=sha(log),summary=[l for l in text.splitlines() if 'TEST SUMMARY' in l or 'ALL TESTS' in l or 'File audit' in l or '[warning]' in l]))
 save('validation',dict(compiler_sha256=binary,checks=results));print(name,p.returncode,flush=True)
 if name in ['prior','file-audit']:assert p.returncode==0
prior=json.loads((root/'student.tests/pa29/evidence182/remaining.json').read_text())
old={r['test'] for r in prior['failures']};text=(scratch/'stage-final.log').read_text()
failed={l.split(': ERROR:')[0] for l in text.splitlines() if ': ERROR:' in l}
assert ' / 403 TESTS PASSED' in text
assert failed<old,(failed-old,len(failed),len(old))
save('stage-delta',dict(total=403,entry_failures=len(old),final_failures=len(failed),fixed=sorted(old-failed),new_failures=sorted(failed-old),remaining=sorted(failed)))
rows=[r for r in prior['failures'] if r['test'] in failed]
for row in rows:row['diagnostic_provenance']='Implementation183 final full suite; inherited semantic ownership/disposition.'
save('remaining',dict(total=403,passed=403-len(failed),failed=len(failed),owner_counts=dict(collections.Counter(r['owner'] for r in rows)),failures=rows))
paths=subprocess.check_output(['git','ls-files','pa29/tests','pa29/scripts','pa29/Makefile','TESTING_AND_REFERENCES.md'],text=True).splitlines()
assert not subprocess.check_output(['git','diff','--name-only',entry,'--','pa29/tests','pa29/scripts','pa29/Makefile','TESTING_AND_REFERENCES.md'],text=True)
save('coverage',dict(entry=entry,unchanged=True,total_stage_inputs=403,files={p:sha(root/p) for p in paths}))
assert source=={p:sha(root/p) for p in source} and binary==sha(root/'dev/cppgm++')
save('validated-source',dict(compiler_sha256=binary,files=source))
print('stageProgress: pass; failures',len(failed),'coverage',len(paths),flush=True)
