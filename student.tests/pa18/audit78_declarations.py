#!/usr/bin/env python3
"""Incomplete declaration views across translation units: CC WORK."""
from pathlib import Path
import hashlib,json,subprocess,sys
cc=Path(sys.argv[1]).resolve();w=Path(sys.argv[2]);w.mkdir(parents=True,exist_ok=True)
root=Path(__file__).resolve().parents[2];rows=[]
for kind in ('parameter','result','both'):
 forward='struct A;'+('int f(A);' if kind=='parameter' else 'A f();' if kind=='result' else 'A f(A);')
 complete='struct A{int n;};'+('int f(A);int g(){A a={7};return f(a);}' if kind=='parameter' else 'A f();int g(){return f().n;}' if kind=='result' else 'A f(A);int g(){A a={7};return f(a).n;}')
 definition='struct A{int n;};'+('int f(A a){return a.n;}' if kind=='parameter' else 'A f(){A a={7};return a;}' if kind=='result' else 'A f(A a){return a;}')+'int g();int main(){return g()!=7;}'
 paths=[]
 for tag,source in [('forward',forward),('complete',complete),('definition',definition)]:
  p=w/(kind+'-'+tag+'.cpp');p.write_text(source);paths.append(p)
 for reverse in (False,True):
  for executable in (False,True):
   source_paths=list(reversed(paths[:2])) if reverse else paths[:2]
   if executable:source_paths.append(paths[2])
   name=f'{kind}-{int(reverse)}-{int(executable)}';ir=w/(name+'.lowir')
   r=subprocess.run([cc,'--emit-lowir','-O0','--validate-lowir','-o',ir,*source_paths],capture_output=True,text=True,timeout=30)
   row=dict(name=name,inputs=[dict(source=p.read_text(),sha256=hashlib.sha256(p.read_bytes()).hexdigest()) for p in source_paths],compiler_exit=r.returncode,diagnostic=r.stderr,passed=r.returncode==0,expectation='checked execution' if executable else 'external declaration: validated LowIR only')
   if executable and r.returncode==0:
    exe=w/name;b=subprocess.run([root/'dev/lowir2native-ref','-O0','-o',exe,ir],capture_output=True,text=True)
    row['backend_exit']=b.returncode;row['native_exit']=subprocess.run([exe]).returncode if b.returncode==0 else None
    row['passed']=b.returncode==0 and row['native_exit']==0
   rows.append(row);print(name,'PASS' if row['passed'] else 'FAIL',r.stderr.strip(),flush=True)
(w/'results.json').write_text(json.dumps(rows,indent=2)+'\n')
sys.exit(not all(r['passed'] for r in rows))
