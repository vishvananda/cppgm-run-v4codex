#!/usr/bin/env python3
"""Required handoff gates and unchanged course coverage, with exact failure delta."""
import hashlib,json,pathlib,subprocess,sys
root=pathlib.Path(__file__).resolve().parents[2];out=pathlib.Path(sys.argv[1]).resolve();out.mkdir(parents=True,exist_ok=True)
base='0c6df4c1e290c8188e95f07732dad0e7ec271b17'
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
r=dict(code_tip=subprocess.check_output(['git','rev-parse','HEAD'],cwd=root,text=True).strip(),compiler_sha256=sha(root/'dev/cppgm++'),checks=[])
for name,args in [('stage',['make','test-pa29']),('prior',['make','test-report-through-pa28']),('through',['make','test-report-through-pa29']),('file-audit',['perl','scripts/cppgm_file_audit.pl','--stage','pa29','--paths','dev/src'])]:
 log=out/(name+'.log')
 with log.open('w') as f:p=subprocess.run(args,cwd=root,stdout=f,stderr=subprocess.STDOUT)
 s=log.read_text();r['checks'].append(dict(command=' '.join(args),status=p.returncode,log=str(log),sha256=sha(log),summary=[l for l in s.splitlines() if 'TEST SUMMARY' in l or 'ALL TESTS' in l or 'File audit' in l or '[warning]' in l]))
 (out/'validation.json').write_text(json.dumps(r,indent=2)+'\n');print(name,p.returncode,flush=True)
 if name in ['prior','file-audit']:assert p.returncode==0
entry={x['test'] for x in json.loads((root/'student.tests/pa29/evidence167/remaining.json').read_text())['failures']}
failed={s.split(': ERROR:')[0] for s in (out/'stage.log').read_text().splitlines() if ': ERROR:' in s}
assert len(entry)==49 and len(failed)<len(entry) and failed<=entry
assert '357 / 403' in (out/'stage.log').read_text()
assert sha(root/'dev/cppgm++')==r['compiler_sha256']
(out/'stage-delta.json').write_text(json.dumps(dict(base=base,entry_failures=len(entry),current_failures=len(failed),fixed=sorted(entry-failed),new_failures=sorted(failed-entry)),indent=2)+'\n')
paths=subprocess.check_output(['git','ls-tree','-r','--name-only',base,'Makefile','dev/Makefile','scripts','reference-binaries','pa29/Makefile','pa29/scripts','pa29/tests'],cwd=root,text=True).splitlines()
files={}
for p in paths:
 original=subprocess.check_output(['git','show',base+':'+p],cwd=root)
 assert (root/p).read_bytes()==original,p
 files[p]=hashlib.sha256(original).hexdigest()
(out/'coverage.json').write_text(json.dumps(dict(base=base,required_fixture_count=403,unchanged=True,files=files),indent=2)+'\n')
previous=json.loads((root/'student.tests/pa29/evidence167/remaining.json').read_text())
previous.update(entry=base,passed=403-len(failed),total=403,failures=[x for x in previous['failures'] if x['test'] in failed])
previous['counts']={}
for item in previous['failures']:
 item['diagnostic_provenance']='implementation168 final required-suite output; inherited owner label is not root-cause proof'
 previous['counts'][item['owner']]=previous['counts'].get(item['owner'],0)+1
(out/'remaining.json').write_text(json.dumps(previous,indent=2)+'\n')
print('stageProgress passed: 49 -> '+str(len(failed)),flush=True)
