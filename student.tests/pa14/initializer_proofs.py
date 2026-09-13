#!/usr/bin/env python3
"""Archive reduced source proofs without changing fixtures or comparison rules."""
from pathlib import Path
import hashlib,json,subprocess,sys
from body_compare import compare
ROOT=Path(__file__).resolve().parents[2]
A,B,WORK,OUT=map(lambda x:Path(x).resolve(),sys.argv[1:5]);WORK.mkdir(parents=True,exist_ok=True)
def sha(p):return hashlib.sha256(Path(p).read_bytes()).hexdigest()
def file(p):return dict(path=str(p),sha256=sha(p))
validation=ROOT/'student.tests/pa14/initializer-validation.json';v=json.loads(validation.read_text())
row=next(r for r in v['checks'] if r['name']=='release-initializers')
manifest=Path(row['command'][-1])/'checks.json';controls=json.loads(manifest.read_text())
assert controls['binary']['sha256']==sha(B)
r=dict(harness=file(Path(__file__).resolve()),validation=file(validation),control_manifest=file(manifest),standard=file(ROOT/'doc/n3485.txt'),binaries=[file(A),file(B)],cases=[],demand_controls=[],remaining=[])
for item in controls['checks']:
 src=Path(item['source_path']);assert sha(src)==item['source_sha256'];n=item['name']
 ir=WORK/(n+'.lowir');p=subprocess.run([str(A),'--emit-lowir','-O0','--validate-lowir','-o',str(ir),str(src)],capture_output=True,text=True)
 log=WORK/(n+'.log');log.write_text(p.stdout+p.stderr)
 case=dict(name=n,source=file(src),required_exit=int(item['reject']),entry_exit=p.returncode,current_exit=item['exit_code'],entry_log=file(log))
 if not item['reject']:
  current=src.with_suffix('.lowir');outputs=[file(ir),file(current)] if not p.returncode else [file(current)]
  case['outputs']=outputs
  if len(outputs)==2 and outputs[0]['sha256']!=outputs[1]['sha256']:case['comparison']=compare(src,outputs,WORK/'comparison',n)
  case['current_native']=item['native'];case['native_exit']=0
  if not p.returncode:
   exe=WORK/n;p=subprocess.run([str(ROOT/'dev/lowir2native-ref'),'-O0','-o',str(exe),str(ir)],capture_output=True,text=True);assert p.returncode==0,(n,p.stderr)
   p=subprocess.run([str(exe)],timeout=10);assert p.returncode==0,(n,p.returncode)
   case['entry_native']=file(exe)
 r['cases'].append(case)
for name,source,expected in [
 ('unused-constructor-body','template<class T>struct A{A(int){int n=T::missing;}};template<class T>void f(){A<int>a(1);}int main(){return 0;}',0),
 ('used-constructor-body','template<class T>struct A{A(int){int n=T::missing;}};template<class T>void f(){A<int>a(1);}int main(){f<int>();}',1),
 ('unused-default-body','template<class T>int bad(){return T::missing;}struct A{A(int,int=bad<int>());};template<class T>void f(){A a(1);}int main(){return 0;}',0),
 ('used-default-body','template<class T>int bad(){return T::missing;}struct A{A(int,int=bad<int>());};template<class T>void f(){A a(1);}int main(){f<int>();}',1),
]:
 src=WORK/(name+'.cpp');src.write_text(source);item=dict(name=name,source=file(src),required_exit=expected,outputs=[])
 for i,binary in enumerate([A,B,Path(v['binaries'][2]['path'])]):
  ir=WORK/(name+'-'+str(i)+'.lowir')
  p=subprocess.run([str(binary),'--emit-lowir','-O0','--validate-lowir','-o',str(ir),str(src)],capture_output=True,text=True)
  log=WORK/(name+'-'+str(i)+'.log');log.write_text(p.stdout+p.stderr)
  assert p.returncode==expected,(name,i,p.returncode,p.stderr)
  assert 'AddressSanitizer' not in p.stderr and 'runtime error:' not in p.stderr,(name,i,p.stderr)
  item['outputs'].append(dict(binary=file(binary),exit_code=p.returncode,log=file(log)))
 r['demand_controls'].append(item)
remaining=Path(v['binaries'][1]['path']).parent/'remaining-source-owners/observations.json'
observations=json.loads(remaining.read_text());assert observations['binary_sha256']==sha(B)
r['remaining_manifest']=file(remaining);r['remaining']=observations['cases']
r['rules']=[dict(clauses='N3485 [dcl.init]/16-17; [dcl.init.ref]/5; [dcl.init.list]/3,7; [dcl.init.aggr]/1-2; [dcl.init.string]/2; [class.dtor]/11; [stmt.return]/2; [temp.res]/8',proof='The fixed initializer must satisfy the target type conversion, reference binding, aggregate extent/order, narrowing, constructor/access and destructor rules. Template substitution cannot repair these fixed invalid operations. PA14 requires definition-time checks of unused supported bodies. Valid controls separately exercise per-use objects, references, lists, constructors and cleanup. No reference is changed; compiler agreement is not the proof.'),dict(clauses='spec.md sections 2,4,5,6,8,9',proof='Source identity and fixed target own immutable selections/conversion recipes. Complete-class scheduling applies only to nonstatic member initializers; static dependence binds before later declarations. Concrete occurrences own materialization and projected list operands. Unrelated declarations and repeated calls do not multiply source selection work. The four recorded remaining errors keep the full-stage audit open.')]
r['rules'].append(dict(clauses='N3485 [temp.inst]/10',proof='Nondependent references inside an uninstantiated template do not instantiate their referenced specialization definitions. Selecting an explicit initializer constructor and its default recipes checks declarations/conversions without demanding their bodies. The four controls independently check deferred and required constructor/default bodies under entry, current release and ASan/UBSan compilers.'))
r['rules'].append(dict(clauses='N3485 [dcl.constexpr]/9',proof='A constexpr specifier in an object declaration declares the object const. The source binding must retain that qualifier, so fixed source expressions and concrete declarations agree. The scalar control was rejected at entry by its source/concrete type invariant and now runs correctly.'))
OUT.write_text(json.dumps(r,indent=2)+'\n')
print(len(r['cases']),'source proofs;',sum(c['required_exit']==1 and c['entry_exit']==0 for c in r['cases']),'entry-accepted invalid programs;',len(r['remaining']),'open handoffs')
