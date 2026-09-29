#!/usr/bin/env python3
"""Validate student-generated LowIR with supplied object backend and host RTTI.

The reference compiler only consumes .lowir here; no source-generated answer is
used. Run CC WORK. Preserve the separate freestanding-runtime observations.
"""
from pathlib import Path
import hashlib, json, re, subprocess, sys
from rtti102 import runner, info
ROOT=Path(__file__).resolve().parents[2]
CC,WORK=[Path(x).resolve() for x in sys.argv[1:]]
WORK.mkdir(parents=True,exist_ok=True)
rows=[]
def sha(p):return hashlib.sha256(Path(p).read_bytes()).hexdigest()
def run(cmd):
 p=subprocess.run([str(x) for x in cmd],capture_output=True,text=True,timeout=60)
 assert p.returncode==0,(cmd,p.returncode,p.stdout,p.stderr)
 return p

def check(name,sources):
 irs=[];objects=[]
 for i,source in enumerate(sources):
  src=WORK/(name+str(i)+'.cpp');src.write_text(source)
  ir=src.with_suffix('.lowir');obj=src.with_suffix('.o')
  run([CC,'--emit-lowir','-O0','--validate-lowir','-o',ir,src])
  run([ROOT/'dev/cppgm++-ref','-c','-O0','-o',obj,ir])
  irs.append(ir);objects.append(obj)
 exe=WORK/name
 run(['g++','-no-pie',*objects,'-o',exe]);run([exe])
 row=dict(name=name,sources=sources,lowir_sha256=[sha(p) for p in irs],host_exit=0)
 rows.append(row)
 print(name,'HOST PASS',flush=True)
 return row,irs
for name,source in runner.GOOD.items():check(name,[source])
common=info+'struct Shared{};inline const std::type_info&local_type(){struct Local{};return typeid(Local);}'
a=common+'namespace{struct Hidden{};}const std::type_info&shared_a(){return typeid(Shared);}const std::type_info&hidden_a(){return typeid(Hidden);}const std::type_info&local_a(){return local_type();}'
b=common+'namespace{struct Hidden{};}const std::type_info&shared_a();const std::type_info&hidden_a();const std::type_info&local_a();int main(){return shared_a()!=typeid(Shared)||hidden_a()==typeid(Hidden)||local_a()!=local_type();}'
row,irs=check('translation_unit_identity',[a,b])
exe=WORK/'tu-freestanding'
combined=WORK/'tu-combined.lowir'
run([CC,'--emit-lowir','-O0','--validate-lowir','-o',combined,*[p.with_suffix('.cpp') for p in irs]])
run([ROOT/'dev/lowir2native-ref','-O0','-o',exe,combined]);run([exe]);row['freestanding_exit']=0
a=info+'struct A;const std::type_info&ptr_a(){return typeid(A*);}'
b=info+'struct R{virtual int f(){return 1;}};struct B:R{};struct A:B{};const std::type_info&ptr_a();int main(){A a;R*p=&a;return ptr_a()!=typeid(A*)||dynamic_cast<B*>(p)!=static_cast<B*>(&a);}'
row,irs=check('incomplete_complete_identity',[a,b])
exe=WORK/'incomplete-freestanding';combined=WORK/'incomplete-combined.lowir'
run([CC,'--emit-lowir','-O0','--validate-lowir','-o',combined,*[p.with_suffix('.cpp') for p in irs]])
run([ROOT/'dev/lowir2native-ref','-O0','-o',exe,combined]);run([exe]);row['freestanding_exit']=0
assert 'binding=internal, object=_ZTI1A]' in irs[0].read_text()
assert 'binding=internal, object=_ZTIP1A]' in irs[0].read_text()
assert 'binding=weak, object=_ZTSP1A]' in irs[0].read_text()
source=(ROOT/'student.tests/pa21/rtti_name_reducer102.cpp').read_text()
row,irs=check('name_reducer',[source])
expected='1OIN1n1WINS0_1MENS0_1VENS3_IiJEEEEEE'
text=irs[0].read_text()
assert 'object=_ZTI'+expected+']' in text
block=re.search(r'global @\S+ \[binding=weak, object=_ZTS'+expected+r'\] = \{\n(.*?)\n\}',text,re.S)
assert block
assert bytes(int(v) for v in re.findall(r'^  i8 (\d+)$',block[1],re.M))==expected.encode()+b'\0'
row['expected_rtti_name']=expected
result=dict(compiler_sha256=sha(CC),object_backend_sha256=sha(ROOT/'reference-binaries/cppgm++'),
 host=run(['g++','--version']).stdout.splitlines()[0],rows=rows)
(WORK/'results.json').write_text(json.dumps(result,indent=2)+'\n')
