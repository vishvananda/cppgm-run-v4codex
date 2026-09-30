#!/usr/bin/env python3
"""Serial final audit validation, exact inventories and command results."""
import hashlib, json, pathlib, subprocess, sys
root=pathlib.Path(__file__).resolve().parents[2]
out=pathlib.Path(sys.argv[1]).resolve(); out.mkdir(parents=True,exist_ok=True)
base='369c57f19fa13c0394d4fb0345cf2bb79708fc67'
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def git(*args):return subprocess.check_output(['git',*args],cwd=root)
binary=sha(root/'dev/cppgm++')
commands=[('fileAudit',['perl','scripts/cppgm_file_audit.pl','--stage','pa26','--paths','dev/src']),
 ('stage',['make','test-pa26']),('through',['make','test-report-through-pa26']),
 ('outputPolicy',['python3','student.tests/pa26/output_policy144.py',out/'policy','dev/cppgm++']),
 ('linkControls',['python3','student.tests/pa26/link_controls144.py',out/'link','dev/cppgm++']),
 ('driverControls',['python3','student.tests/pa25/driver.py']),
 ('hostControls',['python3','student.tests/pa26/controls.py','--out',out/'host']),
 ('headers',['python3','student.tests/pa26/header_controls.py','--out',out/'headers']),
 ('intrinsics',['python3','student.tests/pa26/intrinsic_controls.py','--out',out/'intrinsics']),
 ('functionAddress',['python3','student.tests/pa26/function_address.py',out/'address']),
 ('inspection',['python3','student.tests/pa26/native_inspection.py',out/'inspect'])]
result={'stage_base':base,'code_commit':git('rev-parse','HEAD').decode().strip(),
        'compiler_sha256':binary,'checks':[]}
def save(): (out/'validation.json').write_text(json.dumps(result,indent=2)+'\n')
for name,args in commands:
    args=list(map(str,args)); log=out/(name+'.log')
    with log.open('wb') as stream:p=subprocess.run(args,cwd=root,stdout=stream,stderr=subprocess.STDOUT)
    text=log.read_text(errors='replace')
    result['checks'].append(dict(name=name,command=args,exit_code=p.returncode,log_sha256=sha(log),
        result_lines=[s for s in text.splitlines() if any(k in s for k in ['PASSED','ERROR:','passed','PASS','trace passed','failures:','controls','identity'])]))
    save(); print(name,p.returncode,flush=True); assert p.returncode==0,(name,text[-4000:])
    if name=='through':assert 'ALL TESTS PASSED SUCCESSFULLY! (4283 / 4283)' in text
protected=['scripts','TESTING_AND_REFERENCES.md','spec.md']+['pa%d'%n for n in range(1,26)]+['pa26/tests','pa26/scripts','pa26/Makefile','pa26/README.md']
assert not git('diff','--name-only',base,'--',*protected).strip()
paths=git('ls-files','pa26/tests','pa26/scripts','pa26/README.md','pa26/Makefile').decode().splitlines()
result['contract_inventory']={p:sha(root/p) for p in paths}
paths=git('diff','--name-only',base,'--','dev').decode().splitlines()
result['implementation_inventory']={p:sha(root/p) for p in paths}
result['build_config']=(root/'obj/dev/.compile_config').read_text()
assert binary==sha(root/'dev/cppgm++')
result['coverage_unchanged']=True; result['test_count']=4283;result['stages_passed']=26
save();print('Final audit checks pass; protected fixtures unchanged',flush=True)
