#!/usr/bin/env python3
"""Inspect actual LowIR, MIR, ELF/CFI and bounded forced-inline behavior."""
import hashlib,json,pathlib,subprocess,sys
root=pathlib.Path(__file__).resolve().parents[2];out=pathlib.Path(sys.argv[1]).resolve();out.mkdir(parents=True,exist_ok=True)
cc=root/'dev/cppgm++'
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
r=dict(compiler_sha256=sha(cc),commands=[],images={},views={})
def save():(out/'optimization-trace.json').write_text(json.dumps(r,indent=2)+'\n')
def run(args):
 args=list(map(str,args));p=subprocess.run(args,capture_output=True,text=True,timeout=45)
 r['commands'].append(dict(args=args,status=p.returncode,stdout=p.stdout,stderr=p.stderr));save();assert not p.returncode,r['commands'][-1]
 return p
for name,src in [('optimization',root/'student.tests/pa30/source198/optimization.cpp'),('flow-conditional',root/'student.tests/pa30/source202/flow-conditional.cpp')]:
 obj=out/(name+'.o');ir=out/(name+'.lowir');mir=out/(name+'.mir');exe=out/name
 run([cc,'-O0','-c',src,'-o',obj]);plain=sha(obj)
 run([cc,'-O0','-c','--stats',src,'-o',obj]);assert plain==sha(obj)
 run(['g++',obj,'-o',exe]);run([exe])
 run([cc,'--emit-lowir','--validate-lowir','--stats',src,'-o',ir])
 run([root/'dev/lowir2native','--stats','--dump-machine-ir',mir,'-o',out/(name+'.native'),ir]);run([out/(name+'.native')])
 r['images'][name]=dict(source_sha256=sha(src),object_sha256=sha(obj),executable_sha256=sha(exe),telemetry_identical=True)
 r['views'][name]=dict(lowir=ir.read_text(),mir=mir.read_text(),disassembly=run(['objdump','-dr',obj]).stdout,symbols=run(['readelf','-Ws',obj]).stdout,frames=run(['readelf','--debug-dump=frames',obj]).stdout)
 save()
print(len(r['commands']),'inspection commands passed')
