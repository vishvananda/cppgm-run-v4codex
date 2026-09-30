#!/usr/bin/env python3
"""Explicit audit of typed runtime IR, native ABI, source facts and final ELF."""
import hashlib, json, pathlib, subprocess, sys
root=pathlib.Path(__file__).resolve().parents[2]
out=pathlib.Path(sys.argv[1]).resolve();out.mkdir(parents=True,exist_ok=True)
objects=[]
for folder in ['preprocess','lowir','native','toolchain']:
    for p in (root/'obj/dev'/folder).glob('*.o'):
        if folder=='preprocess' and p.stem not in ['source','identifier_table']:continue
        if folder=='toolchain' and p.stem=='driver':continue
        if folder=='lowir' and p.stem=='exercises':continue
        objects.append(p)
commands=[['g++','-std=c++11','-I'+str(root/'dev/src'),root/'student.tests/pa25/runtime-api.cc',*objects,'-o',out/'runtime-api'],[out/'runtime-api',out/'runtime']]
src=root/'student.tests/pa25/class-trace.cc'; cxx=root/'dev/cppgm++'
commands += [[cxx,'--emit-ast','-o',out/'class.ast',src],
 [cxx,'--emit-lowir','--validate-lowir','--stats','-o',out/'class.lowir',src],
 [root/'dev/lowir2native','--dump-machine-ir',out/'class.mir',out/'class.lowir'],
 [cxx,'--stats','-c','-o',out/'class.obj',src],
 [cxx,'--stats','-o',out/'class.elf',out/'class.obj'],[out/'class.elf'],
 [cxx,'--stats','-o',out/'class-direct.elf',src],[out/'class-direct.elf'],
 ['readelf','-l',out/'class.elf']]
results=[]
for i,args in enumerate(commands):
    p=subprocess.run(list(map(str,args)),capture_output=True,text=True,timeout=60)
    (out/f'{i}.stdout').write_text(p.stdout);(out/f'{i}.stderr').write_text(p.stderr)
    results.append({'command':list(map(str,args)),'exit_code':p.returncode})
    assert p.returncode==0,(args,p.stderr)
assert (out/'class.elf').read_bytes()==(out/'class-direct.elf').read_bytes()
mir=(out/'runtime.mir').read_text()
assert 'param %native_arg -> rdi : i64' in mir
assert 'syscall [args=(' in mir and 'r10' in mir
source_ir=(out/'class.lowir').read_text()
assert 'function @__cppgm_init' not in source_ir # all object facts are static
assert 'ptr addr @item' in source_ir # self-pointer relocation, no anonymous storage
stats=[json.loads(x) for x in (out/'6.stderr').read_text().splitlines() if x.startswith('{')][-1]
assert stats['runtime_functions']==2 and stats['runtime_instructions']>0
results.append({'checks':['direct/separate ELF identity','native parameter/syscall ABI','static construction and self relocation','finite runtime work counters'],'passed':True})
manifest={str(p):hashlib.sha256(p.read_bytes()).hexdigest() for p in [cxx,src,*out.glob('class.*'),out/'runtime.lowir',out/'runtime.mir']}
(out/'results.json').write_text(json.dumps({'commands':results,'manifest':manifest},indent=2)+'\n')
print('11 trace commands and 4 ABI/data-flow assertions passed')
