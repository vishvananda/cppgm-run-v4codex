#!/usr/bin/env python3
"""Required gates, coverage preservation and exact PA29 failure delta."""
import hashlib,json,pathlib,subprocess,sys
root=pathlib.Path(__file__).resolve().parents[2];out=pathlib.Path(sys.argv[1]).resolve();out.mkdir(parents=True,exist_ok=True)
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
entry='217dc69ff3f741114d2614688b1e4e2eec335980'
r=dict(code_tip=subprocess.check_output(['git','rev-parse','HEAD'],cwd=root,text=True).strip(),entry=entry,compiler_sha256=sha(root/'dev/cppgm++'),checks=[])
for name,args in [('stage',['make','test-pa29']),('prior',['make','test-report-through-pa28']),('through',['make','test-report-through-pa29']),('file-audit',['perl','scripts/cppgm_file_audit.pl','--stage','pa29','--paths','dev/src'])]:
 log=out/(name+'.log')
 with log.open('w') as f:p=subprocess.run(args,cwd=root,stdout=f,stderr=subprocess.STDOUT)
 s=log.read_text();r['checks'].append(dict(command=' '.join(args),status=p.returncode,log=str(log),sha256=sha(log),summary=[l for l in s.splitlines() if 'TEST SUMMARY' in l or 'ALL TESTS' in l or 'File audit' in l or '[warning]' in l]))
 (out/'validation.json').write_text(json.dumps(r,indent=2)+'\n');print(name,p.returncode,flush=True)
 if name in ['prior','file-audit']:assert p.returncode==0
baseline=json.loads((root/'student.tests/pa29/evidence168/remaining.json').read_text())
old={x['test'] for x in baseline['failures']}
failed={s.split(': ERROR:')[0] for s in (out/'stage.log').read_text().splitlines() if ': ERROR:' in s}
assert len(old)==46 and len(failed)<len(old) and failed<=old
assert '361 / 403' in (out/'stage.log').read_text()
(out/'stage-delta.json').write_text(json.dumps(dict(entry_failures=46,final_failures=len(failed),fixed=sorted(old-failed),new_failures=sorted(failed-old),remaining=sorted(failed)),indent=2)+'\n')
tracked=subprocess.check_output(['git','ls-files','pa29/tests','pa29/scripts','pa29/Makefile','TESTING_AND_REFERENCES.md'],cwd=root,text=True).splitlines()
coverage=[]
for path in tracked:
 original=subprocess.check_output(['git','show',entry+':'+path],cwd=root)
 current=(root/path).read_bytes();assert original==current,path
 coverage.append(dict(path=path,sha256=hashlib.sha256(current).hexdigest()))
(out/'coverage.json').write_text(json.dumps(dict(entry=entry,unchanged=True,files=coverage),indent=2)+'\n')
assert sha(root/'dev/cppgm++')==r['compiler_sha256']
print('stageProgress passed: 46 -> '+str(len(failed)),flush=True)
