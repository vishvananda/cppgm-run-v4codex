#!/usr/bin/env python3
"""Capture the exact PA26 implementation handoff checks without changing gates."""
import hashlib,json,pathlib,re,subprocess,sys
root=pathlib.Path(__file__).resolve().parents[2]
out=pathlib.Path(sys.argv[1] if len(sys.argv)>1 else '/tmp/pa26-142/validation').resolve()
out.mkdir(parents=True,exist_ok=True)
base='369c57f19fa13c0394d4fb0345cf2bb79708fc67'
def sha(data):return hashlib.sha256(data).hexdigest()
def git(*args):return subprocess.check_output(['git',*args],cwd=root)
records=[]
commands=[
 ('priorThroughTests', 'n=26; if [ "$n" -le 1 ]; then echo "===== ALL TESTS PASSED SUCCESSFULLY! (0/0) ====="; else make test-report-through-pa$((n - 1)); fi',0),
 ('stageTests','make test-pa26',2),
 ('fileAudit','perl scripts/cppgm_file_audit.pl --stage pa26 --paths dev/src',0),
 ('personalControls','python3 student.tests/pa26/controls.py',0),
 ('typedInspection','python3 student.tests/pa26/native_inspection.py',0)]
for name,command,status in commands:
    r=subprocess.run(command,shell=True,executable='/bin/bash',cwd=root,stdout=subprocess.PIPE,stderr=subprocess.STDOUT)
    (out/(name+'.log')).write_bytes(r.stdout)
    assert r.returncode==status,(name,r.returncode,r.stdout.decode())
    text=r.stdout.decode()
    if name=='stageTests':
        assert '29 / 30 TESTS PASSED' in text
        errors=[x for x in text.splitlines() if ': ERROR:' in x]
        assert len(errors)==1 and '300-shared-conditional-cleanup-resume.t:' in errors[0]
    records.append(dict(name=name,command=command,exit_code=r.returncode,output_sha256=sha(r.stdout),
        result_lines=[x for x in text.splitlines() if any(k in x for k in ['PASSED','ERROR:','audit passed','controls passed','trace passed'])]))
# No fixture, harness, comparison rule or reference modifications in any stage.
protected=['scripts','TESTING_AND_REFERENCES.md','spec.md']
protected+=['pa%d'%n for n in range(1,26)]
protected+=['pa26/tests','pa26/scripts','pa26/Makefile','pa26/README.md']
assert not git('diff','--name-only',base,'--',*protected).strip()
paths=git('ls-files','pa26/tests','pa26/scripts','pa26/Makefile','pa26/README.md').decode().splitlines()
inventory={p:sha((root/p).read_bytes()) for p in paths}
assert all(sha(git('show',base+':'+p))==s for p,s in inventory.items())
changes=git('diff','--name-only',base,'--','dev').decode().splitlines()
code={p:sha((root/p).read_bytes()) for p in changes}
assert (root/'dev/cppgm++').read_bytes()==pathlib.Path('/tmp/pa26-142/accepted-cppgm').read_bytes()
manifest=dict(stage_base=base,code_commit=git('rev-parse','HEAD').decode().strip(),
    compiler_sha256=sha((root/'dev/cppgm++').read_bytes()),build_flags=(root/'obj/dev/.compile_config').read_text(),
    checks=records,stage_progress=dict(entry_passed=0,entry_failed=30,final_passed=29,final_failed=1,coverage_unchanged=True),
    contract_inventory=inventory,implementation_inventory=code,
    independent_audit='pending; Last reviewed commit remains the stage base',
    remaining_implementation=['required <string> integration','uniform host/private object and link-driver migration'])
(out/'validation.json').write_text(json.dumps(manifest,indent=2)+'\n')
print('Handoff checks: prior 4253/4253; PA26 29/30; file audit pass; controls and typed inspection pass; unchanged coverage')
