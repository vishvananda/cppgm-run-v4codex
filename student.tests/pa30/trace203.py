#!/usr/bin/env python3
"""Inspect the shared typed LowIR/MIR/ELF path and external-input validation."""
import hashlib,json,pathlib,re,subprocess,sys
root=pathlib.Path(__file__).resolve().parents[2];out=pathlib.Path(sys.argv[1]).resolve();out.mkdir(parents=True,exist_ok=True)
cc=root/'dev/cppgm++'
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
r=dict(compiler_sha256=sha(cc),commands=[],images={},views={})
def save():(out/'trace.json').write_text(json.dumps(r,indent=2)+'\n')
def run(cmd,reject=False):
 cmd=list(map(str,cmd));p=subprocess.run(cmd,capture_output=True,text=True,timeout=45)
 r['commands'].append(dict(args=cmd,status=p.returncode,stdout=p.stdout,stderr=p.stderr,expected_rejection=reject));save();assert (p.returncode!=0)==reject,r['commands'][-1]
 return p
objects=[p for p in sorted((root/'obj/dev').rglob('*.o')) if 'entry' not in p.parts and not p.name.startswith('test_runner')]
adapter=out/'adapter';run(['g++','-std=c++11','-O2','-I'+str(root/'dev/src'),root/'student.tests/pa29/ir-object161.cpp',*objects,'-o',adapter])
for name in ['vector-storage','vector-volatile-bitcast','vector-template-assignment','vector-shuffle','packed-boundaries','sse-state','sse-memory']:
 src=root/f'student.tests/pa30/source203/{name}.cpp';obj=out/(name+'.o');ir=out/(name+'.lowir');mir=out/(name+'.mir');exe=out/name
 run([cc,'-O0','-c',src,'-o',obj]);plain=sha(obj)
 run([cc,'-O0','-c','--stats',src,'-o',obj]);assert plain==sha(obj)
 run([cc,'--emit-lowir','--validate-lowir',src,'-o',ir])
 # The external reader canonicalizes floating literal suffixes. Require a
 # stable canonical text and exercise the reparsed form through both backends.
 canonical=out/(name+'.roundtrip');run([root/'dev/lowir',ir,'-o',canonical])
 run([root/'dev/lowir',canonical,'-o',out/(name+'.roundtrip2')]);assert canonical.read_bytes()==(out/(name+'.roundtrip2')).read_bytes()
 rebuilt=out/(name+'.roundtrip.o');run([adapter,canonical,rebuilt]);run(['g++',rebuilt,'-o',exe]);run([exe])
 run([root/'dev/lowir2native','--stats','--dump-machine-ir',mir,'-o',out/(name+'.native'),canonical]);run([out/(name+'.native')])
 r['images'][name]=dict(source_sha256=sha(src),direct_object_sha256=plain,roundtrip_object_sha256=sha(rebuilt),telemetry_identical=True)
 r['views'][name]=dict(lowir=ir.read_text(),mir=mir.read_text(),disassembly=run(['objdump','-dr',obj]).stdout,symbols=run(['readelf','-Ws',obj]).stdout,frames=run(['readelf','--debug-dump=frames',obj]).stdout);save()
# External LowIR consumers must reject malformed target operation facts.
original=(out/'sse-state.lowir').read_text()
for name,pattern,replacement in [('bad-id',r'x86 \d+,','x86 4294967295,'),('bad-record',r'(x86 \d+, )[^,\n]+,',r'\g<1>%v1,'),('bad-predicate',r'(x86 \d+, [^,\n]+, )\d+',r'\g<1>32')]:
 text,count=re.subn(pattern,replacement,original,count=1);assert count
 src=out/(name+'.lowir');src.write_text(text);run([root/'dev/lowir',src,'-o',out/'bad.out'],True)
assert sha(cc)==r['compiler_sha256']
print(len(r['commands']),'trace and validation commands pass',flush=True)
