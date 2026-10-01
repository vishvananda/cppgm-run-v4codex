#!/usr/bin/env python3
"""Final required gates, retaining every status and the unchanged failure set."""
import hashlib,json,pathlib,subprocess,sys
root=pathlib.Path(__file__).resolve().parents[2];out=pathlib.Path(sys.argv[1]).resolve();out.mkdir(parents=True,exist_ok=True)
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
r=dict(code_tip=subprocess.check_output(['git','rev-parse','HEAD'],cwd=root,text=True).strip(),compiler_sha256=sha(root/'dev/cppgm++'),checks=[])
for name,args in [('stage',['make','test-pa29']),('prior',['make','test-report-through-pa28']),('through',['make','test-report-through-pa29']),('file-audit',['perl','scripts/cppgm_file_audit.pl','--stage','pa29','--paths','dev/src'])]:
 log=out/(name+'.log')
 with log.open('w') as f:p=subprocess.run(args,cwd=root,stdout=f,stderr=subprocess.STDOUT)
 s=log.read_text();r['checks'].append(dict(command=' '.join(args),status=p.returncode,log=str(log),sha256=sha(log),summary=[l for l in s.splitlines() if 'TEST SUMMARY' in l or 'ALL TESTS' in l or 'File audit' in l or '[warning]' in l]))
 (out/'validation.json').write_text(json.dumps(r,indent=2)+'\n');print(name,p.returncode,flush=True)
 if name in ['prior','file-audit']:assert p.returncode==0
entry={x['test'] for x in json.loads((root/'student.tests/pa29/evidence165/remaining.json').read_text())['failures']}
failed={s.split(': ERROR:')[0] for s in (out/'stage.log').read_text().splitlines() if ': ERROR:' in s}
assert len(failed)<len(entry) and failed<=entry and '354 / 403' in (out/'stage.log').read_text()
assert sha(root/'dev/cppgm++')==r['compiler_sha256']
print('stageProgress passed: 53 -> '+str(len(failed)),flush=True)
