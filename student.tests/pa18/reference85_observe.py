#!/usr/bin/env python3
"""Observe pinned bundle reducers; validate independently revised PA18 oracles."""
from pathlib import Path
import hashlib,json,re,subprocess,sys
ROOT=Path(__file__).resolve().parents[2];W=Path(sys.argv[1]).resolve();W.mkdir(parents=True,exist_ok=True)
CASES={
 'dormant_effect':'int hits;int init(){return ++hits;}template<class T>struct X{static const int n;};template<class T>const int X<T>::n=init();int main(){X<int>x;return hits;}',
 'dormant_invalid':'template<class T>struct X{static const int n;};template<class T>const int X<T>::n=T::missing;int main(){X<int>x;return 0;}',
 'discard_call':'volatile int value=7;int calls;volatile int&get(){++calls;return value;}void probe(){(void)get();}int main(){probe();return calls!=1;}',
 'discard_name':'volatile int value=7;void probe(){(void)value;}int main(){probe();return 0;}',
}
def sha(p):return hashlib.sha256(Path(p).read_bytes()).hexdigest()
def run(command):return subprocess.run([str(x) for x in command],cwd=ROOT,text=True,capture_output=True,timeout=30)
def execute(ir,name):
 exe=W/(name+'.exe');b=run([ROOT/'dev/lowir2native-ref','-O0','-o',exe,ir]);result=dict(backend_exit=b.returncode,backend_diagnostic=b.stderr)
 if b.returncode==0:result.update(native_exit=run([exe]).returncode,native_sha256=sha(exe))
 return result
rows=[]
for name,source in CASES.items():
 src=W/(name+'.cpp');src.write_text(source)
 for label,cc in [('student',ROOT/'dev/cppgm++'),('reference',ROOT/'reference-binaries/cppgm++')]:
  ir=W/(name+'-'+label+'.lowir');r=run([cc,'--emit-lowir','-O0','-o',ir,src]);row=dict(name=name,label=label,source=source,source_sha256=sha(src),compiler_sha256=sha(cc),exit=r.returncode,diagnostic=r.stderr)
  if r.returncode==0:
   body=ir.read_text();probe=re.findall(r'^function @probe[^\n]*\{\n(.*?)^}',body,re.M|re.S)
   row.update(lowir=body,lowir_sha256=sha(ir),probe_volatile_reads=sum(len(re.findall(r'load volatile i32',x)) for x in probe),**execute(ir,name+'-'+label))
  rows.append(row)
  if label=='student':assert row['exit']==0 and row['native_exit']==0,row
for revision in json.loads((ROOT/'student.tests/pa18/reference85-revisions.json').read_text())['revisions']:
 ir=ROOT/revision['path'];v=run([ROOT/'dev/lowir','-o',W/(ir.stem+'.roundtrip'),ir]);assert v.returncode==0,v.stderr
 row=dict(path=revision['path'],sha256=sha(ir),validator_exit=v.returncode)
 if 'transitive-base' not in ir.name:
  row.update(execute(ir,ir.stem));assert row['native_exit']==0,row
 else:row['execution']='Not used as C++ evidence: course source binds a reference through a null pointer.'
 rows.append(row)
result=dict(bundle='c2f713cd70d06170632bfde3e75dd6fe1aa44d98',rows=rows)
(W/'observations.json').write_text(json.dumps(result,indent=2)+'\n')
for r in rows:print(r.get('name',r.get('path')),r.get('label','oracle'),'compile',r.get('exit',r.get('validator_exit')),'native',r.get('native_exit'),'volatile reads',r.get('probe_volatile_reads'))
