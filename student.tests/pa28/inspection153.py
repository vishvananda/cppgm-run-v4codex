#!/usr/bin/env python3
"""Inspect the actual typed host path, including mixed vcall/vbase covariance."""
import hashlib,json,pathlib,re,subprocess,sys
root=pathlib.Path(__file__).resolve().parents[2];out=pathlib.Path(sys.argv[1]).resolve();out.mkdir(parents=True,exist_ok=True)
records=[]
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def run(args):
 p=subprocess.run(list(map(str,args)),cwd=root,capture_output=True,timeout=60)
 records.append(dict(args=list(map(str,args)),status=p.returncode,stdout=p.stdout.decode(),stderr=p.stderr.decode()))
 assert not p.returncode,records[-1]
 return p.stdout.decode()
objects=run(['make','-s','-C','dev','--eval','print-objects:;@echo $(call frontend_objs,cppgm++)','print-objects']).split()
run(['g++','-std=c++11','-O2','-Idev/src','student.tests/pa27/audit150-view.cpp',*[(root/'dev'/p).resolve() for p in objects],'-o',out/'view'])
source=root/'pa28/tests/general/200-host-covariant-return-layout-finalization.t.1'
stats=json.loads(run([out/'view',source,out/'trace']))
run(['dev/cppgm++','-c',source,'-o',out/'ordinary.o'])
assert (out/'ordinary.o').read_bytes()==(out/'trace.o').read_bytes()
run(['g++',out/'trace.o','-o',out/'trace']);run([out/'trace'])
mir=(out/'trace.mir').read_text();ir=(out/'trace.lowir').read_text()
assert 'adjustor_thunk' in ir and 'index i8' in ir
assert 'function ' in mir
frames=run(['readelf','-wf',out/'trace.o']);relocs=run(['readelf','-rW',out/'trace.o']);names=run(['nm',out/'trace.o'])
assert 'CIE' in frames and 'FDE' in frames
assert 'R_X86_64' in relocs
assert '_ZTch0_v0_n32_N14VirtualDerived4selfEv' in names
assert '_ZTch0_h32_N13ReturnDerived4selfEv' in names
result=dict(status='pass',source_sha256=sha(source),binary_sha256=sha(root/'dev/cppgm++'),statistics=stats,hashes={n:sha(out/n) for n in ['trace.lowir','trace.mir','trace.o','ordinary.o']},verified=['fixed and virtual covariant thunk ABI identities','mixed prefix result projection','validated typed LowIR','MIR encoding and ordinary object byte equality','host runtime and unwind relocations'],commands=records)
(out/'inspection.json').write_text(json.dumps(result,indent=2)+'\n')
print('native inspection PASS:',stats)
