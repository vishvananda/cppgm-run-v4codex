#!/usr/bin/env python3
"""Retain reduced source proofs and the preceding compiler's actual outcomes."""
from pathlib import Path
import hashlib,json,subprocess,sys
ROOT=Path(__file__).resolve().parents[2]
A,CONTROLS,WORK,OUT=map(lambda x:Path(x).resolve(),sys.argv[1:5]);WORK.mkdir(parents=True,exist_ok=True)
def sha(p):return hashlib.sha256(Path(p).read_bytes()).hexdigest()
def file(p):return dict(path=str(p),sha256=sha(p))
controls=json.loads(CONTROLS.read_text())
assert len({r['source_path'] for r in controls['checks']})==len(controls['checks'])
result=dict(entry_commit='7f401fa8',entry=file(A),controls=file(CONTROLS),harness_sha256=sha(__file__),
 standard=file(ROOT/'doc/n3485.txt'),handout=file(ROOT/'pa14/README.md'),spec=file(ROOT/'spec.md'),
 clauses={
  'definition_time':'PA14 Assignment Boundary explicitly requires definition-time checks of unused supported bodies; N3485 [temp.res]/8 describes non-dependent semantic constraints.',
  'default_initialization':'N3485 [dcl.init]/7,9,12; [class.ctor]/5-6: default initialization, deleted default constructors and their subobject requirements.',
  'destruction':'N3485 [class.dtor]/5 and [class.temporary]/1: deleted defaulted destructors and temporary semantic constraints.',
  'decltype':'N3485 [expr.call]/11 exempts the decltype operand function-call result from result completeness/abstractness/temporary creation; functional construction remains a conversion under [expr.type.conv].',
  'casts_calls_lists':'N3485 [expr.type.conv], [expr.static.cast], [over.match.ctor], [over.match.call], [over.match.oper], [dcl.init.list]/7: selected functions must be usable; list narrowing is rejected.',
  'conditional':'N3485 [expr.cond]/1,3-6: contextually convert the condition, perform directional matches and builtin overload resolution, preserve the resulting category and evaluate only the selected branch.',
  'declarations':'N3485 [stmt.ambig]/1, [basic.scope.block], [basic.link]/6: declaration preference, local declaration identity, and block extern linkage.',
  'const_classes':'The unchanged course fixtures accept const empty/fully initialized classes. This follows the const-default-constructible correction recorded in CWG253 (https://cplusplus.github.io/CWG/issues/253.html), rather than claiming that N3485 literal wording already includes that correction.'},
 reference_policy='No reference, fixture, comparison rule or bundle changed; compiler agreement is not the language proof.',checks=[])
for row in controls['checks']:
 name=row['name'];source=Path(row['source_path']);assert sha(source)==row['source_sha256']
 ir=WORK/(name+'.lowir');command=[A,'--emit-lowir','-O0','--validate-lowir','-o',ir,source]
 p=subprocess.run(list(map(str,command)),cwd=ROOT,capture_output=True,text=True,timeout=60)
 log=WORK/(name+'.log');log.write_text(p.stdout+p.stderr)
 check=dict(name=name,source=file(source),required_exit=int(row['reject']),entry_exit=p.returncode,log=file(log),incorrect=p.returncode!=int(row['reject']))
 if not row['reject'] and p.returncode==0:
  exe=WORK/name;p=subprocess.run(list(map(str,[ROOT/'dev/lowir2native-ref','-O0','-o',exe,ir])),capture_output=True,text=True,timeout=60)
  log=WORK/(name+'-native.log');log.write_text(p.stdout+p.stderr)
  check['backend']=dict(exit_code=p.returncode,log=file(log))
  if 'backend_limitation' in row:
   assert p.returncode==1 and 'undefined native symbol: pure_virtual' in p.stderr
  else:
   assert p.returncode==0,(name,p.stderr)
   p=subprocess.run([str(exe)],capture_output=True,text=True,timeout=15)
   check['native']=dict(binary=file(exe),exit_code=p.returncode)
   check['incorrect']|=p.returncode!=0
 result['checks'].append(check)
 OUT.write_text(json.dumps(result,indent=2)+'\n')
print(len(result['checks']),'retained entry observations;',sum(r['incorrect'] for r in result['checks']),'incorrect outcomes')
