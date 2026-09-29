#!/usr/bin/env python3
"""Diagnostic work growth and bounded-proof fallback (not timing gates)."""
from pathlib import Path
import json,subprocess,time,hashlib
ROOT=Path(__file__).resolve().parents[2];WORK=Path('/tmp/pa22-114/growth');WORK.mkdir(parents=True,exist_ok=True)
rows=[]
for n in (512,2048):
 source='struct C{int x;int f(int y)const{return x+y;}};\n'+''.join('int f%d(C&c){int(C::*p)(int)const=&C::f;return (c.*p)(3);}\n'%i for i in range(n))+'int main(){C c;c.x=4;return f0(c)!=7;}'
 src=WORK/(str(n)+'.cpp');src.write_text(source);ir=src.with_suffix('.lowir')
 run=subprocess.run([ROOT/'dev/cppgm++','--emit-lowir','-O0','--stats','--validate-lowir','-o',ir,src],capture_output=True,text=True);assert not run.returncode,run.stderr
 facts=[json.loads(s) for s in run.stderr.splitlines()];assert facts[0]['member_pointer_proof_work']==2*n
 rows.append(dict(functions=n,source_sha256=hashlib.sha256(source.encode()).hexdigest(),output_bytes=ir.stat().st_size,telemetry=facts))
# A long chain exhausts the shared 64-node budget and keeps generic adjustment.
source='struct C{int x;int f()const{return x;}};int main(){C c;c.x=7;int(C::*p0)()const=&C::f;'+''.join('int(C::*p%d)()const=p%d;'%(i,i-1) for i in range(1,200))+'p0=p199;return (c.*p199)()!=7;}'
src=WORK/'chain.cpp';src.write_text(source);ir=src.with_suffix('.lowir');exe=src.with_suffix('')
for cmd in ([ROOT/'dev/cppgm++','--emit-lowir','-O0','--validate-lowir','-o',ir,src],[ROOT/'dev/lowir2native-ref','-O0','-o',exe,ir],[exe]):
 r=subprocess.run(cmd,capture_output=True,text=True);assert not r.returncode,(cmd,r.stderr)
assert 'binary shr i128' in ir.read_text()
result=dict(growth=rows,chain_variables=200,chain_execution=0,generic_fallback=True,note='A cyclic write dependency through 200 locals retains generic adjustment. Per-demand budget is 64 expression nodes.')
(ROOT/'student.tests/pa22/growth114.json').write_text(json.dumps(result,indent=2)+'\n')
print('work growth and chain execution passed')
