#!/usr/bin/env python3
"""Trace a demanded template through production LowIR/MIR and host ELF."""
import hashlib,json,pathlib,subprocess,sys
root=pathlib.Path(__file__).resolve().parents[2]
out=pathlib.Path(sys.argv[1] if len(sys.argv)>1 else '/tmp/pa26-142/inspect').resolve()
out.mkdir(parents=True,exist_ok=True)
def run(args):
    r=subprocess.run(list(map(str,args)),cwd=root,stdout=subprocess.PIPE,stderr=subprocess.PIPE)
    if r.returncode: raise RuntimeError((args,r.returncode,r.stderr.decode()))
    return r.stdout
objects=run(['make','-s','-C','dev','--eval','print-objects:;@echo $(call frontend_objs,cppgm++)','print-objects']).decode().split()
objects=[(root/'dev'/p).resolve() for p in objects]
run(['g++','-std=c++11','-O2','-Idev/src','student.tests/pa26/dump.cpp',*objects,'-o',out/'dump'])
source=root/'student.tests/pa26/trace.cpp'
lowir=run([out/'dump',source]); mir=run([out/'dump',source,'--mir'])
assert b'@calculate' in lowir and b'eh_catch ' in lowir
assert b'host_exception [rbp' in mir and b'host_eh landing ^b' in mir
assert b'host_raw_selector [rbp' in mir and b'host_outer ^b' in mir
(out/'trace.lowir').write_bytes(lowir); (out/'trace.mir').write_bytes(mir)
obj=out/'trace.o'; exe=out/'trace'
run(['dev/cppgm++','-c','-o',obj,source]); run(['g++',obj,'-o',exe]); run([exe])
frames=run(['readelf','-wf',obj]); relocs=run(['readelf','-rW',obj]); symbols=run(['nm','-C',obj])
assert b'zPLR' in frames and b'DW_CFA_restore_state' in frames
assert b'R_X86_64_PLT32' in relocs and b'_Unwind_Resume' in relocs and b'__gxx_personality_v0' in relocs
assert b'calculate<11>' in symbols
result={'status':'pass','source':str(source),'source_sha256':hashlib.sha256(source.read_bytes()).hexdigest(),
    'lowir_sha256':hashlib.sha256(lowir).hexdigest(),'mir_sha256':hashlib.sha256(mir).hexdigest(),
    'object_sha256':hashlib.sha256(obj.read_bytes()).hexdigest(),
    'symbols_sha256':hashlib.sha256(symbols).hexdigest(),
    'frames_sha256':hashlib.sha256(frames).hexdigest(),
    'relocations_sha256':hashlib.sha256(relocs).hexdigest(),
    'verified_facts':['demanded calculate<11> specialization','typed EH clauses in production LowIR',
        'MIR host frame slots and landing block identity','zPLR CIE','epilogue CFI restore state',
        'MIR saved raw selector and outer region identities',
        'PLT32 call relocations','host personality reference','host resume reference','program exits zero']}
(out/'inspection.json').write_text(json.dumps(result,indent=2)+'\n')
print('Template -> production LowIR -> host MIR -> ELF trace passed')
