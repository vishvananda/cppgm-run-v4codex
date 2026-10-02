#!/usr/bin/env python3
"""Typed LowIR/object trace, telemetry invariance and ELF inspection."""
import hashlib,json,pathlib,subprocess,sys
root=pathlib.Path(__file__).resolve().parents[2]
out=pathlib.Path(sys.argv[1]).resolve(); out.mkdir(parents=True,exist_ok=True)
cc=root/'dev/cppgm++'
def sha(p): return hashlib.sha256(p.read_bytes()).hexdigest()
r=dict(compiler_sha256=sha(cc),commands=[],images={},views={})
def save(): (out/'trace.json').write_text(json.dumps(r,indent=2)+'\n')
def run(args):
 p=subprocess.run(list(map(str,args)),capture_output=True,text=True,timeout=45)
 r['commands'].append(dict(args=list(map(str,args)),status=p.returncode,stdout=p.stdout,stderr=p.stderr));save()
 assert p.returncode==0,r['commands'][-1]
 return p
objects=[p for p in sorted((root/'obj/dev').rglob('*.o')) if 'entry' not in p.parts and not p.name.startswith('test_runner')]
adapter=out/'adapter'
run(['g++','-std=c++11','-O2','-I'+str(root/'dev/src'),root/'student.tests/pa29/ir-object161.cpp',*objects,'-o',adapter])
for name in ['flow-conditional','flow-conditional-effects','new-converted','new-query-converted','interactions']:
 src=root/f'student.tests/pa30/source202/{name}.cpp';obj=out/(name+'.o');ir=out/(name+'.lowir');exe=out/name
 run([cc,'-O0','-c',src,'-o',obj]);plain=sha(obj)
 run([cc,'-O0','-c','--stats',src,'-o',obj]); assert plain==sha(obj)
 run([cc,'-c','--emit-lowir','--validate-lowir',src,'-o',ir])
 run([root/'dev/lowir',ir,'-o',out/(name+'.roundtrip')]); assert ir.read_bytes()==(out/(name+'.roundtrip')).read_bytes()
 rebuilt=out/(name+'.roundtrip.o');run([adapter,ir,rebuilt]);run(['g++',rebuilt,'-o',exe]);run([exe])
 r['images'][name]=dict(source_sha256=sha(src),direct_object_sha256=plain,roundtrip_object_sha256=sha(rebuilt),telemetry_identical=True)
 r['views'][name]=dict(lowir=ir.read_text(),symbols=run(['readelf','-Ws',obj]).stdout,disassembly=run(['objdump','-dr',obj]).stdout,frames=run(['readelf','--debug-dump=frames',obj]).stdout)
 save()
print(len(r['commands']),'trace commands pass')
