#!/usr/bin/env python3
"""Coalescing resolved views must preserve later competing final overriders."""
from pathlib import Path
import hashlib,json,subprocess,sys
ROOT=Path(__file__).resolve().parents[2]
cc,work=map(lambda x:Path(x).resolve(),sys.argv[1:3]);work.mkdir(parents=True,exist_ok=True)
rows=[]
for resolved in (False,True):
 source='struct V{virtual int f(){return 1;}};struct A0:virtual V{};struct B0:virtual V{};'
 for i in range(1,4):source+='struct A%d:virtual A%d,virtual B%d{};struct B%d:virtual A%d,virtual B%d{};'%(i,i-1,i-1,i,i-1,i-1)
 source+='struct L:A3{int f(){return 2;}};struct R:B3{int f(){return 3;}};struct D:L,R{'+('int f(){return 7;}' if resolved else '')+'};'
 source+='int main(){D d;V&v=d;return v.f()!=7;}' if resolved else 'int main(){return 0;}'
 src=work/('resolved.cpp' if resolved else 'ambiguous.cpp');src.write_text(source);ir=src.with_suffix('.lowir');exe=src.with_suffix('.exe')
 p=subprocess.run([str(cc),'--emit-lowir','-O0','--validate-lowir','-o',str(ir),str(src)],capture_output=True,text=True)
 row=dict(source=source,expected_accept=resolved,compile_exit=p.returncode,diagnostic=p.stderr)
 if resolved and not p.returncode:
  p=subprocess.run([str(ROOT/'dev/lowir2native-ref'),'-O0','-o',str(exe),str(ir)],capture_output=True,text=True);row.update(backend_exit=p.returncode,backend_diagnostic=p.stderr)
  if not p.returncode:row['runtime_exit']=subprocess.run([str(exe)],timeout=20).returncode
 row['passed']=row.get('runtime_exit')==0 if resolved else row['compile_exit']!=0;rows.append(row)
print(json.dumps(dict(compiler_sha256=hashlib.sha256(cc.read_bytes()).hexdigest(),cases=rows),indent=2))
sys.exit(not all(r['passed'] for r in rows))
