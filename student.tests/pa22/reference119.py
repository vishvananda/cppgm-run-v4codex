#!/usr/bin/env python3
"""Reproduce each PA22 oracle correction; retain original/fixed hashes and outcomes."""
from pathlib import Path
import hashlib,json,subprocess,sys
ROOT=Path(__file__).resolve().parents[2]
CC,WORK=[Path(x).resolve() for x in sys.argv[1:3]];WORK.mkdir(parents=True,exist_ok=True)
ENTRY='17603c8a69e76820274b7bf849091d528f6ff261'
RED=ROOT/'student.tests/pa22/reference119'
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def run(cmd):
 p=subprocess.run([str(x) for x in cmd],cwd=ROOT,capture_output=True,text=True)
 return dict(command=[str(x) for x in cmd],exit=p.returncode,stdout=p.stdout,stderr=p.stderr)
def checked(cmd):
 r=run(cmd);assert r['exit']==0,r;return r
def compile(cc,stem,inputs):
 ir=WORK/(stem+'.lowir');r=checked([cc,'--emit-lowir','-O0','-o',ir,*inputs]);return ir,r
def execute(ir):
 exe=WORK/(ir.stem+'.exe');r=run([ROOT/'dev/lowir2native-ref','-O0','-o',exe,ir])
 result=dict(backend=r)
 if not r['exit']:result['execution']=run([exe])
 return result
def old(path):return subprocess.check_output(['git','show',ENTRY+':'+str(path)],cwd=ROOT,text=True)
paths=[Path('pa22/tests')/bucket/(name+'.ref') for bucket,name in [
 ('general','300-const-member-function-pointer-address-call'),
 ('spec','300-member-pointer-parameter-variadic-deduction'),
 ('spec','300-overloaded-member-pointer-function-template-deduction'),
 ('general','300-structured-bool-conditional-member-pointer-dead-branch')]]
result=dict(entry=ENTRY,compiler_sha256=sha(CC),lowir_sha256=sha(ROOT/'dev/lowir'),
 manifest=(ROOT/'reference-binaries/manifest.tsv').read_text(),reducers={str(p.relative_to(ROOT)):sha(p) for p in RED.iterdir()},fixtures=[])
for path in paths:
 o=WORK/(path.stem+'.original.lowir');o.write_text(old(path));new=ROOT/path
 row=dict(path=str(path),original_sha256=sha(o),corrected_sha256=sha(new),
          original_validation=run([ROOT/'dev/lowir','-o',WORK/'checked.lowir',o]),
          corrected_validation=checked([ROOT/'dev/lowir','-o',WORK/'checked.lowir',new]),corrected_native=execute(new))
 assert row['corrected_native'].get('execution',{}).get('exit')==0,row
 result['fixtures'].append(row)
caller,command=compile(CC,'caller',[RED/'adjustment-caller.cpp'])
caller_text='\n'.join(s for s in caller.read_text().splitlines() if not s.startswith('declare function @call'))+'\n'
result['adjustment_caller']=command;rows=[]
for lane,text in [('original',old(paths[0])),('corrected',(ROOT/paths[0]).read_text())]:
 ir=WORK/(lane+'-adjustment.lowir');ir.write_text(text.split('function @main')[0]+caller_text)
 checked([ROOT/'dev/lowir','-o',WORK/'checked.lowir',ir]);r=execute(ir);rows.append(dict(lane=lane,lowir_sha256=sha(ir),**r))
 assert r.get('execution',{}).get('exit')==(1 if lane=='original' else 0),r
result['adjustment']=rows
# The reference reducer's callee must reproduce the same missing-adjustment body.
ref,command=compile(ROOT/'dev/cppgm++-ref','ref-callee',[RED/'adjustment-callee.cpp'])
assert ref.read_text().strip()==old(paths[0]).split('function @main')[0].strip()
result['reference_callee']=dict(command=command,lowir_sha256=sha(ref),matches_original=True)
rows=[]
for lane,text in [('original',old(paths[1])),('corrected',(ROOT/paths[1]).read_text())]:
 ir=WORK/(lane+'-truth.lowir')
 body='function @mem_fn'+text.split('function @mem_fn',1)[1]
 body+='''\nfunction @main() -> i32 [role=entry] {
 block ^entry:
  %one = convert zext i128 i64 1
  %null_target = binary shl i128 %one, 64
  %result = call i32 @mem_fn(%null_target)
  return i32 %result
}\n'''
 ir.write_text(body);r=execute(ir);rows.append(dict(lane=lane,lowir_sha256=sha(ir),**r))
 assert (r.get('execution',{}).get('exit')==0)==(lane=='corrected'),r
result['null_target']=rows
# Explicit malformed-width reducer plus retained narrow compatibility.
result['wide_truth']=run([ROOT/'dev/lowir','-o',WORK/'wide.lowir',RED/'wide-truth.lowir'])
assert result['wide_truth']['exit']!=0
result['narrow_truth']=checked([ROOT/'dev/lowir','-o',WORK/'narrow.lowir',RED/'narrow-truth.lowir'])
rows=[]
for lane,cc in [('reference',ROOT/'dev/cppgm++-ref'),('student',CC)]:
 ir,r=compile(cc,lane+'-unused',[RED/'unused-static.cpp'])
 emitted=any(line.startswith('global @') for line in ir.read_text().splitlines())
 rows.append(dict(lane=lane,compile=r,emitted_static=emitted,lowir_sha256=sha(ir)))
 assert emitted==(lane=='reference')
result['unused_static']=rows
# Source-level cross-TU execution in the completed compiler, no injected representation.
ir,r=compile(CC,'source-adjustment',[RED/'adjustment-callee.cpp',RED/'adjustment-caller.cpp'])
result['source_adjustment']=dict(compile=r,**execute(ir))
assert result['source_adjustment'].get('execution',{}).get('exit')==0
result['passed']=True
print(json.dumps(result,indent=2))
