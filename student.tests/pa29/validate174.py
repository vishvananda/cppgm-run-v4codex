#!/usr/bin/env python3
"""Sequential checkpoint gates and exact failure/contract preservation."""
import hashlib, json, pathlib, subprocess, sys
root = pathlib.Path(__file__).resolve().parents[2]
out = pathlib.Path(sys.argv[1]).resolve(); out.mkdir(parents=True, exist_ok=True)
entry = '914e1a0a07e40884c91c0b967b0421eea9ef0d48'
reviewed = '221d6d0e4930da05db2913bdf5f50d808f89c744'
def sha(p): return hashlib.sha256(p.read_bytes()).hexdigest()
result = dict(entry=entry, previous_review=reviewed,
              code_tip=subprocess.check_output(['git','rev-parse','HEAD'],cwd=root,text=True).strip(),
              compiler_sha256=sha(root/'dev/cppgm++'), checks=[])
prior = 'n=29; if [ "$n" -le 1 ]; then echo \'===== ALL TESTS PASSED SUCCESSFULLY! (0/0) =====\'; else make test-report-through-pa$((n - 1)); fi'
checks = [('stage',['make','test-pa29']),('prior',['bash','-c',prior]),
          ('through',['make','test-report-through-pa29']),
          ('file-audit',['perl','scripts/cppgm_file_audit.pl','--stage','pa29','--paths','dev/src'])]
for name,args in checks:
    log=out/(name+'.log')
    with log.open('w') as f: p=subprocess.run(args,cwd=root,stdout=f,stderr=subprocess.STDOUT)
    text=log.read_text()
    result['checks'].append(dict(command=args,status=p.returncode,log=str(log),sha256=sha(log),
        summary=[s for s in text.splitlines() if 'TEST SUMMARY' in s or 'ALL TESTS' in s or 'File audit' in s or '[warning]' in s]))
    (out/'validation.json').write_text(json.dumps(result,indent=2)+'\n')
    print(name,p.returncode,flush=True)
    if name in ['prior','file-audit']: assert not p.returncode
baseline=json.loads((root/'student.tests/pa29/evidence173/remaining.json').read_text())
old={x['test'] for x in baseline['failures']}
text=(out/'stage.log').read_text()
failed={s.split(': ERROR:')[0] for s in text.splitlines() if ': ERROR:' in s}
assert len(old)==26 and failed<=old and ' / 403 TESTS PASSED' in text
(out/'stage-delta.json').write_text(json.dumps(dict(entry_failures=len(old),final_failures=len(failed),
    fixed=sorted(old-failed),new_failures=sorted(failed-old),remaining=sorted(failed)),indent=2)+'\n')
tracked=subprocess.check_output(['git','ls-files','pa29/tests','pa29/scripts','pa29/Makefile',
    'TESTING_AND_REFERENCES.md'],cwd=root,text=True).splitlines()
coverage=[]
for path in tracked:
    current=(root/path).read_bytes()
    for rev in [entry,reviewed]:
        assert current==subprocess.check_output(['git','show',rev+':'+path],cwd=root),path
    coverage.append(dict(path=path,sha256=hashlib.sha256(current).hexdigest()))
(out/'coverage.json').write_text(json.dumps(dict(entry=entry,previous_review=reviewed,unchanged=True,
    total_stage_inputs=403,files=coverage),separators=(',',':'))+'\n')
assert sha(root/'dev/cppgm++')==result['compiler_sha256']
print('stageProgressPreserved: 26 ->',len(failed),flush=True)
