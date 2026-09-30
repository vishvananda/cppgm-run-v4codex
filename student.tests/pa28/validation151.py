#!/usr/bin/env python3
"""PA28 implementation handoff checks, including unchanged fixture coverage."""
import hashlib, json, pathlib, re, subprocess, sys, time
root = pathlib.Path(__file__).resolve().parents[2]
out = pathlib.Path(sys.argv[1]).resolve()
baseline = pathlib.Path(sys.argv[2]).resolve()
out.mkdir(parents=True,exist_ok=True)
base = 'bec9389f62014dfecac8b41d330de142e0904b8c'
def sha(p): return hashlib.sha256(p.read_bytes()).hexdigest()
def git(*args): return subprocess.check_output(['git',*args],cwd=root,text=True).strip()
def failures(text):
    return sorted(re.findall(r'^(pa28/.*?\.t): ERROR:',text,re.M))
entry = baseline.read_text()
assert '81 / 97 TESTS PASSED' in entry and len(failures(entry)) == 16
result = dict(stage_base=base,code_tip=git('rev-parse','HEAD'),
              binary_sha256=sha(root/'dev/cppgm++'),
              baseline=dict(passed=81,total=97,failures=failures(entry),
                            log_sha256=sha(baseline)),checks=[])
def save(): (out/'validation.json').write_text(json.dumps(result,indent=2)+'\n')
commands = [
    ('priorThroughTests',['bash','-c',"n=28; if [ \"$n\" -le 1 ]; then echo '===== ALL TESTS PASSED SUCCESSFULLY! (0/0) ====='; else make test-report-through-pa$((n - 1)); fi"],0),
    ('stageTests',['make','test-pa28'],2),
    ('fileAudit',['perl','scripts/cppgm_file_audit.pl','--stage','pa28','--paths','dev/src'],0),
    ('naming',['python3','student.tests/pa28/naming151.py',str(out/'naming')],0),
    ('diffCheck',['git','diff','--check'],0)]
for name,command,expected in commands:
    log = out/(name+'.log'); start=time.monotonic()
    with log.open('w') as stream:
        p=subprocess.run(command,cwd=root,stdout=stream,stderr=subprocess.STDOUT)
    result['checks'].append(dict(name=name,command=command,status=p.returncode,
        seconds=time.monotonic()-start,log=str(log),log_sha256=sha(log),tail=log.read_text()[-3000:]))
    save()
    assert p.returncode == expected,(name,p.returncode,log)
    print(name,p.returncode,flush=True)
stage=(out/'stageTests.log').read_text()
current=failures(stage)
assert '91 / 97 TESTS PASSED' in stage and len(current)==6
assert set(current) < set(result['baseline']['failures'])
result['stageProgress']=dict(status='pass',passed=91,total=97,remaining=current,
    resolved=sorted(set(result['baseline']['failures'])-set(current)))
prior=(out/'priorThroughTests.log').read_text()
assert '4441 / 4441' in prior
result['earlier']=dict(passed=4441,total=4441,stages=27)
scope=[f'pa{i}/tests' for i in range(1,29)]
protected=[*scope,'scripts','TESTING_AND_REFERENCES.md','reference-binaries',
           *[f'pa{i}/Makefile' for i in range(1,29)],*[f'pa{i}/scripts' for i in range(1,29)]]
assert not git('diff','--name-only',base,'--',*protected)
inventory=git('ls-files','-s','--',*scope)
result['coverage']=dict(stage_anchors=len(list((root/'pa28/tests').rglob('*.t'))),
    tracked_contract_paths=len(inventory.splitlines()),
    inventory_sha256=hashlib.sha256(inventory.encode()).hexdigest(),changes=[])
paths=git('diff','--name-only',base,'HEAD','--','dev').splitlines()
result['implementation_hashes']={p:sha(root/p) for p in paths}
assert result['binary_sha256']==sha(root/'dev/cppgm++')
result['handoff_status']='validated incomplete implementation'
save()
