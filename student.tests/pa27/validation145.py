#!/usr/bin/env python3
"""Record required checks, unchanged fixture inventory, and handoff progress."""
import hashlib, json, pathlib, re, subprocess, sys, time
root=pathlib.Path(__file__).resolve().parents[2]
out=pathlib.Path(sys.argv[1]).resolve(); out.mkdir(parents=True,exist_ok=True)
baseline=pathlib.Path(sys.argv[2]).resolve()
base='f833cf1ff361529147361cada33eca62e55330cf'
def sha(p): return hashlib.sha256(p.read_bytes()).hexdigest()
def git(*args): return subprocess.check_output(['git',*args],cwd=root).decode().strip()
def failures(text): return sorted(set(re.findall(r'(pa27/[^ :]+\.t): ERROR',text)))
old=failures(baseline.read_text()); assert len(old)==42
result=dict(stage_base=base,implementation_commit=git('rev-parse','HEAD'),
            binary_sha256=sha(root/'dev/cppgm++'),checks=[])
commands=[
    ('priorThroughTests',['make','test-report-through-pa26'],0),
    ('stageTests',['make','test-pa27'],2),
    ('fileAudit',['perl','scripts/cppgm_file_audit.pl','--stage','pa27','--paths','dev/src'],0),
    ('personalObjects',['python3','student.tests/pa27/object-controls.py',str(out/'objects')],0),
    ('diffCheck',['git','diff','--check'],0),
]
for name,args,expected in commands:
    log=out/(name+'.log'); start=time.perf_counter()
    with log.open('w') as stream:
        p=subprocess.run(args,cwd=root,stdout=stream,stderr=subprocess.STDOUT)
    result['checks'].append(dict(name=name,command=args,status=p.returncode,elapsed_s=time.perf_counter()-start,
        output_sha256=sha(log),output_path=str(log)))
    assert p.returncode==expected,(name,p.returncode)
    print(name,p.returncode,flush=True)
stage=(out/'stageTests.log').read_text(); current=failures(stage)
assert set(current)<set(old),set(current)-set(old)
assert '133 / 158 TESTS PASSED' in stage and 'properties: PASS (3/3)' in stage
prior=(out/'priorThroughTests.log').read_text()
assert 'ALL TESTS PASSED SUCCESSFULLY! (4283 / 4283)' in prior
assert sha(root/'dev/cppgm++')==result['binary_sha256']
result['progress']=dict(entry_pass=116,current_pass=133,total=158,entry_fail=len(old),current_fail=len(current),
    resolved=sorted(set(old)-set(current)),remaining=current,new_failures=[])
allowed={f'pa27/tests/general/{stem}{suffix}'
    for stem in ['200-host-defined-global-data-pcrel','200-host-imported-global-data-got']
    for suffix in ['.inspect.expect','.ref.inspect']}
changed=set(git('diff','--name-only',base,'--',*[f'pa{n}/tests' for n in range(1,28)],'scripts').splitlines())
assert changed==allowed,changed
paths=git('ls-files',*[f'pa{n}/tests' for n in range(1,28)],'scripts').splitlines()
before=git('ls-tree','-r','--name-only',base,'--',*[f'pa{n}/tests' for n in range(1,28)],'scripts').splitlines()
assert paths==before
result['coverage']=dict(tracked_contract_paths=len(paths),inventory_sha256=hashlib.sha256('\n'.join(paths).encode()).hexdigest(),
    pa27_anchors=len(list((root/'pa27/tests/general').glob('*.t'))),allowed_reference_overlay=sorted(changed),
    source_and_comparison_changes=[])
result['sources']={p:sha(root/p) for p in git('diff','--name-only',base,'--','dev').splitlines()}
result['status']='validated incomplete implementation handoff; independent audit pending'
(out/'validation.json').write_text(json.dumps(result,indent=2)+'\n')
print('PA27 handoff: 42 -> 25 failures; prior stages and file audit pass')
