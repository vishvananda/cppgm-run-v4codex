#!/usr/bin/env python3
"""Final PA27 gates and explicit personal controls, with unchanged-contract proof."""
import hashlib, json, pathlib, re, subprocess, sys, time

root = pathlib.Path(__file__).resolve().parents[2]
out = pathlib.Path(sys.argv[1]).resolve()
out.mkdir(parents=True, exist_ok=True)
base = 'f833cf1ff361529147361cada33eca62e55330cf'
entry = '9f5e095d450519d8660a29d4629ec5b8f5bcddde'

def git(*args):
    return subprocess.check_output(['git',*args],cwd=root,text=True).strip()

def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()

result = dict(stage_base=base, audit_entry=entry, code_tip=git('rev-parse','HEAD'),
              binary_sha256=sha(root/'dev/cppgm++'), checks=[])

def save():
    (out/'validation.json').write_text(json.dumps(result,indent=2)+'\n')

commands = [('throughStage',['make','test-report-through-pa27']),
            ('stage',['make','test-pa27']),
            ('fileAudit',['perl','scripts/cppgm_file_audit.pl','--stage','pa27','--paths','dev/src']),
            ('trace',['python3','student.tests/pa27/audit150.py',str(out/'trace')])]
commands += [(name,['python3',f'student.tests/pa27/{name}.py',str(out/name)]) for name in
             ('audit148-controls','storage-controls','declarator-controls','linkage-controls',
              'tls-controls','object-controls','hosted149-controls','header-context150')]
commands += [('signature79',['python3','student.tests/pa18/signature79_controls.py','dev/cppgm++',str(out/'signature79')]),
             ('diffCheck',['git','diff','--check'])]
for name, command in commands:
    log = out/(name+'.log'); start = time.monotonic()
    with log.open('w') as stream:
        p = subprocess.run(command,cwd=root,stdout=stream,stderr=subprocess.STDOUT)
    text = log.read_text()
    result['checks'].append(dict(name=name,command=command,status=p.returncode,
        seconds=time.monotonic()-start,log=str(log),log_sha256=sha(log),tail=text[-1000:]))
    save()
    assert p.returncode == 0, (name, log)
    print(name,'PASS',flush=True)

scope = [f'pa{i}/tests' for i in range(1,28)]
protected = [*scope,'scripts','TESTING_AND_REFERENCES.md','reference-binaries','Makefile',
             *[f'pa{i}/Makefile' for i in range(1,28)], *[f'pa{i}/scripts' for i in range(1,28)]]
assert not git('diff','--name-only',entry,'--',*protected)
corrections = git('diff','--name-only',base,'--',*protected).splitlines()
expected = sorted(f'pa27/tests/general/200-host-{kind}-global-data-{rel}.{suffix}'
                  for kind,rel in [('defined','pcrel'),('imported','got')]
                  for suffix in ('inspect.expect','ref.inspect'))
assert sorted(corrections) == expected
inventory = git('ls-files','-s','--',*scope)
result['coverage'] = dict(tracked_contract_paths=len(inventory.splitlines()),
    inventory_sha256=hashlib.sha256(inventory.encode()).hexdigest(),
    stage_anchors=len(list((root/'pa27/tests').rglob('*.t'))),
    changes_since_entry=[], stage_reference_overlay=corrections)
paths = git('diff','--name-only',base,'HEAD','--','dev').splitlines()
result['implementation_hashes'] = {p:sha(root/p) for p in paths}
result['reviewed_commits'] = git('log','--reverse','--format=%H %s',base+'..HEAD').splitlines()
report = (out/'throughStage.log').read_text()
match = re.search(r'ALL TESTS PASSED SUCCESSFULLY! \((\d+) / (\d+)\)',report)
assert match and match[1] == match[2]
result['through_tests'] = dict(passed=int(match[1]),total=int(match[2]),
    stages=re.findall(r'^===== (pa\d+) =====$',report,re.M))
assert result['through_tests']['stages'] == ['pa'+str(n) for n in range(1,28)]
assert result['binary_sha256'] == sha(root/'dev/cppgm++')
result['status'] = 'pass'
save()
