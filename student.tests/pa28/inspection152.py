#!/usr/bin/env python3
"""Inspect the actual typed host path, including a demanded throw(T) template."""
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
source=root/'student.tests/pa28/exceptions152.cpp'
stats=json.loads(run([out/'view',source,out/'trace']))
run(['dev/cppgm++','-c',source,'-o',out/'ordinary.o'])
assert (out/'ordinary.o').read_bytes()==(out/'trace.o').read_bytes()
run(['g++',out/'trace.o','-o',out/'trace']);run([out/'trace'])
mir=(out/'trace.mir').read_text();ir=(out/'trace.lowir').read_text()
assert 'eh_filter ' in ir and 'template_spec' in ir
assert 'filter @__rtti' in mir and 'host_selector=-1' in mir
assert 'host_eh landing ^native' in mir and not re.search(r'host_eh landing\s*\n',mir)
frames=run(['readelf','-wf',out/'trace.o']);relocs=run(['readelf','-rW',out/'trace.o']);names=run(['nm',out/'trace.o'])
assert 'zPLR' in frames and 'DW_CFA_restore_state' in frames
assert '__gxx_personality_v0' in relocs and 'R_X86_64_PC32' in relocs
assert '__cxa_call_unexpected' in names and '_Unwind_Resume' in names
result=dict(status='pass',source_sha256=sha(source),binary_sha256=sha(root/'dev/cppgm++'),statistics=stats,hashes={n:sha(out/n) for n in ['trace.lowir','trace.mir','trace.o','ordinary.o']},verified=['demanded template exception filter','signed LSDA selector and explicit MIR block IDs','validated typed LowIR','MIR encoding and ordinary object byte equality','host runtime and unwind relocations'],commands=records)
(out/'inspection.json').write_text(json.dumps(result,indent=2)+'\n')
print('native inspection PASS:',stats)
