#!/usr/bin/env python3
"""Implementation gates, exact failure-set comparison, source binding and coverage."""
import collections,hashlib,json,pathlib,subprocess,sys
root=pathlib.Path(__file__).resolve().parents[2]
out=pathlib.Path(sys.argv[1]).resolve();out.mkdir(parents=True,exist_ok=True)
entry=json.loads((out/'entry.json').read_text())
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def save(name,data):(out/(name+'.json')).write_text(json.dumps(data,indent=2)+'\n')
paths=subprocess.check_output(['git','ls-files','--cached','--others','--exclude-standard','dev'],cwd=root,text=True).splitlines()
source={p:sha(root/p) for p in paths}
r=dict(entry=entry['entry'],code_tip=subprocess.check_output(['git','rev-parse','HEAD'],cwd=root,text=True).strip(),compiler_sha256=sha(root/'dev/cppgm++'),checks=[])
commands=[('stage',['make','test-pa29']),('prior',['bash','-c','n=29; if [ "$n" -le 1 ]; then echo \'===== ALL TESTS PASSED SUCCESSFULLY! (0/0) =====\'; else make test-report-through-pa$((n - 1)); fi']),('through',['make','test-report-through-pa29']),('file-audit',['perl','scripts/cppgm_file_audit.pl','--stage','pa29','--paths','dev/src'])]
for name,args in commands:
    log=pathlib.Path('/tmp/pa29-179')/(name+'-final.log')
    with log.open('w') as f:p=subprocess.run(args,cwd=root,stdout=f,stderr=subprocess.STDOUT)
    text=log.read_text();r['checks'].append(dict(command=args,status=p.returncode,log=str(log),sha256=sha(log),summary=[l for l in text.splitlines() if 'TEST SUMMARY' in l or 'ALL TESTS' in l or 'File audit' in l or '[warning]' in l]))
    save('validation',r);print(name,p.returncode,flush=True)
    if name in ['prior','file-audit']:assert not p.returncode
old=set(entry['failed'])
failed={l.split(': ERROR:')[0] for l in pathlib.Path('/tmp/pa29-179/stage-final.log').read_text().splitlines() if ': ERROR:' in l}
assert len(old)==22 and len(failed)<22 and failed<=old
assert ' / 403 TESTS PASSED' in pathlib.Path('/tmp/pa29-179/stage-final.log').read_text()
save('stage-delta',dict(entry_failures=22,final_failures=len(failed),fixed=sorted(old-failed),new_failures=sorted(failed-old),remaining=sorted(failed)))
baseline=json.loads((root/'student.tests/pa29/evidence178/remaining.json').read_text())
rows=[r for r in baseline['failures'] if r['test'] in failed]
for row in rows:row['diagnostic_provenance']='Confirmed by implementation179 final full suite; retained owner grouping.'
save('remaining',dict(total=403,passed=403-len(failed),failed=len(failed),owner_counts=dict(collections.Counter(x['owner'] for x in rows)),failures=rows))
tracked=subprocess.check_output(['git','ls-files','pa29/tests','pa29/scripts','pa29/Makefile','TESTING_AND_REFERENCES.md'],cwd=root,text=True).splitlines()
coverage=[]
for path in tracked:
    current=(root/path).read_bytes()
    for boundary in [entry['entry'],entry['previous_review']]:
        original=subprocess.check_output(['git','show',boundary+':'+path],cwd=root)
        assert original==current,path
    coverage.append(dict(path=path,sha256=hashlib.sha256(current).hexdigest()))
save('coverage',dict(boundaries=[entry['previous_review'],entry['entry']],unchanged=True,total_stage_inputs=403,files=coverage))
assert source=={p:sha(root/p) for p in paths}
assert sha(root/'dev/cppgm++')==r['compiler_sha256']
save('validated-source',dict(compiler_sha256=r['compiler_sha256'],files=source))
print('stageProgress improved:',len(failed),'of',403,'failed',flush=True)
