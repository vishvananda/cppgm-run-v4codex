#!/usr/bin/env python3
"""Reproduce PA19 final audit controls/trace/coverage. Run CC WORK OUT."""
from pathlib import Path
import hashlib, json, re, shutil, struct, subprocess, sys
ROOT = Path(__file__).resolve().parents[2]
CC, WORK, OUT = [Path(p).resolve() for p in sys.argv[1:]]
WORK.mkdir(parents=True, exist_ok=True)
assert not OUT.exists(), 'Preserve earlier evidence'
BASE = 'e5f4c3ed78972c8d161671d145bf525cb99033f4'
sha = lambda p: hashlib.sha256(Path(p).read_bytes()).hexdigest()
def run(command):
    r = subprocess.run([str(x) for x in command],cwd=ROOT,capture_output=True,text=True,timeout=300)
    assert r.returncode == 0, (command,r.returncode,r.stdout,r.stderr)
    return r
result = dict(stage='pa19',audit=93,base=BASE,reviewed=run(['git','rev-parse','HEAD']).stdout.strip(),
    compiler_sha256=sha(CC),commands=[],controls={})
for script in ('reference91.py','reference92.py'):
    command=['python3',ROOT/'student.tests/pa19'/script]
    r=run(command)
    result['commands'].append(dict(command=[str(x) for x in command],exit=0,stdout=r.stdout))
for name, script in [('composition','composition91.py'),('variable','variable92.py'),
                     ('reference','reference_controls92.py'),('cross_handoff','audit93_controls.py')]:
    directory=WORK/name
    command=['python3',ROOT/'student.tests/pa19'/script,CC,directory]
    r=run(command)
    result['commands'].append(dict(command=[str(x) for x in command],exit=0,stdout=r.stdout))
    rows=json.loads((directory/'results.json').read_text())
    assert all(r['passed'] for r in rows)
    result['controls'][name]=rows

source=ROOT/'student.tests/pa19/audit93_trace.cpp'
ir, plain, native = WORK/'trace.lowir', WORK/'trace-plain.lowir', WORK/'trace'
command=[CC,'--emit-lowir','-O0','-o',ir,source]
r=run([*command,'--stats','--validate-lowir'])
stats=[json.loads(line) for line in r.stderr.splitlines()]
run([CC,'--emit-lowir','-O0','-o',plain,source])
assert ir.read_bytes() == plain.read_bytes(), 'Telemetry/audit must not change output'
text=ir.read_text()
assert 'obj<16x8>' in text and 'projection=field' in text
assert text.count('copyobj 12x4 @__constant_array_')==2
assert len(re.findall(r'^global @__constant_array_',text,re.M))==1
assert 'store i32 7, $seven' in text and 'dormant' not in text
assert not re.search(r'call i32 @[^\n]*operator',text)
assert stats[0]['variable_template_initializers']==2
assert stats[0]['semantic_conversion_result_work']==1
assert stats[0]['lower_constant_data_records']==1 and stats[0]['lower_constant_data_hits']==1
run([ROOT/'dev/lowir2native-ref','-O0','-o',native,ir])
run([native])
# Sectionless ELF: preserve the executable segment for inspection; its payload
# includes readonly data. Do not label it an exact .text section.
data=native.read_bytes()
entry, phoff=struct.unpack_from('<QQ',data,24)
kind, flags, offset, address, _, size, _, _=struct.unpack_from('<IIQQQQQQ',data,phoff)
assert data[:5]==b'\x7fELF\x02' and kind==1 and flags&1 and offset==0
start=entry-address
payload=WORK/'trace-payload.bin';payload.write_bytes(data[start:size])
dis=run(['objdump','-D','-b','binary','-m','i386:x86-64','--adjust-vma='+str(entry),payload]).stdout
(WORK/'trace.dis').write_text(dis)
result['trace']=dict(source_sha256=sha(source),lowir_sha256=sha(ir),native_sha256=sha(native),
    telemetry=stats,checked_exit=0,telemetry_parity=True,payload_bytes=size-start,
    disassembly_sha256=sha(WORK/'trace.dis'),lowir=text)
if shutil.which('strace'):
    log=WORK/'self-contained.log'
    run(['strace','-f','-e','trace=execve,openat','-o',log,*command])
    trace=log.read_text()
    assert trace.count('execve(')==1
    assert not re.search(r'openat\([^\n]*(?:reference-binaries|\.ref["/])',trace)
    result['self_containment']=dict(execve_count=1,syscalls=trace)
reducer=ROOT/'student.tests/pa19/defaulted_pack91.cpp'
reducer_ir,reducer_exe=WORK/'defaulted-pack.lowir',WORK/'defaulted-pack'
run([CC,'--emit-lowir','-O0','--validate-lowir','-o',reducer_ir,reducer])
run([ROOT/'dev/lowir2native-ref','-O0','-o',reducer_exe,reducer_ir])
run([reducer_exe])
result['defaulted_pack_reducer']=dict(source_sha256=sha(reducer),checked_exit=0)

changed=run(['git','diff','--name-only',BASE,'--','pa19/tests']).stdout.splitlines()
revisions=[]
for filename in ('reference91-revisions.json','reference92-revisions.json'):
    record=json.loads((ROOT/'student.tests/pa19'/filename).read_text())
    revisions += [r['path'] for r in record.get('revisions',[record])]
assert sorted(changed)==sorted(revisions)
assert all(p.endswith('.ref') for p in changed)
assert not run(['git','diff',BASE,'--','scripts','pa19/scripts','pa19/Makefile']).stdout
inputs=sorted((ROOT/'pa19/tests').rglob('*.t'))
assert len(inputs)==423
result['coverage']=dict(inputs=423,unchanged_sources_and_statuses=True,unchanged_comparison=True,revised_oracles=changed)
paths=run(['git','ls-files','dev/src']).stdout.splitlines()
result['source_hashes']={p:sha(ROOT/p) for p in paths}
result['artifact_hashes']={str(p.relative_to(ROOT)):sha(p) for p in
    [source,Path(__file__),ROOT/'spec.md',ROOT/'pa19/README.md',ROOT/'TESTING_AND_REFERENCES.md',
     ROOT/'student.tests/pa19/audit93_benchmark.py',ROOT/'student.tests/pa19/audit93-performance.json']}
OUT.write_text(json.dumps(result,indent=2)+'\n')
print('PASS:',sum(len(v) for v in result['controls'].values()),'controls, trace, 15 oracle revisions and coverage')
