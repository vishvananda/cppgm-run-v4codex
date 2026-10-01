#!/usr/bin/env python3
"""Inspect zero storage across explicit adapters, native ABI and empty-span guards."""
import hashlib,json,pathlib,subprocess,sys
root=pathlib.Path(__file__).resolve().parents[2];out=pathlib.Path(sys.argv[1]).resolve();out.mkdir(parents=True,exist_ok=True)
cc=root/'dev/cppgm++';src=root/'student.tests/pa29/source181';rows=[]
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def run(args,ok=True):
 args=list(map(str,args));p=subprocess.run(args,capture_output=True,text=True,timeout=120)
 rows.append(dict(command=args,status=p.returncode,stdout=p.stdout,stderr=p.stderr));(out/'inspection.json').write_text(json.dumps(dict(compiler_sha256=sha(cc),rows=rows),indent=2)+'\n')
 assert (p.returncode==0)==ok,rows[-1]
 return p.stdout
objects=[p for p in sorted((root/'obj/dev').rglob('*.o')) if 'entry' not in p.parts and not p.name.startswith('test_runner')]
run(['g++','-std=c++11','-O2','-I'+str(root/'dev/src'),root/'student.tests/pa29/inspection180-object.cpp',*objects,'-o',out/'ir-object'])
for name in ['identity','layout','lifetime','nested','zero-parameter','zero-ctor-new','query','static-template','overaligned','zero-ctor-unwind']:
 ir=out/(name+'.lowir');obj=out/(name+'.o')
 run([cc,'--emit-ast',src/(name+'.cpp'),'-o',out/(name+'.ast')])
 run([cc,'-c','--emit-lowir','--validate-lowir',src/(name+'.cpp'),'-o',ir])
 run([root/'dev/lowir',ir,'-o',out/(name+'.roundtrip')]);assert ir.read_bytes()==(out/(name+'.roundtrip')).read_bytes()
 run([out/'ir-object',ir,out/(name+'.prepared'),out/(name+'.adapter.o')]);assert ir.read_bytes()==(out/(name+'.prepared')).read_bytes()
 run(['g++',out/(name+'.adapter.o'),'-o',out/(name+'.adapter')]);run([out/(name+'.adapter')])
 run([cc,'-O0','--stats','-c',src/(name+'.cpp'),'-o',obj]);run([cc,'-O0','-c',src/(name+'.cpp'),'-o',out/(name+'.plain.o')]);assert obj.read_bytes()==(out/(name+'.plain.o')).read_bytes()
 assert run(['objdump','-dr',obj]).splitlines()[3:]==run(['objdump','-dr',out/(name+'.adapter.o')]).splitlines()[3:]
 assert sorted(run(['nm',obj]).splitlines())==sorted(run(['nm',out/(name+'.adapter.o')]).splitlines())
 run(['readelf','-rSW',obj]);run(['readelf','--debug-dump=frames',obj])
 # The source path's host libc calls can remain unresolved in native executable
 # mode; machine IR still shows the selected frame and actual zero-object homes.
 run([root/'dev/lowir2native','--dump-machine-ir',out/(name+'.mir'),ir])
for library,client in [(cc,'g++'),('g++',cc)]:
 run([library,'-c',src/'boundary.cc','-o',out/'boundary.o'])
 run([client,'-c',src/'boundary-main.cc','-o',out/'client.o'])
 run(['g++',out/'boundary.o',out/'client.o','-o',out/'boundary']);run([out/'boundary'])
 symbols=run(['nm',out/'boundary.o']);assert '_Z4pickPA0_i' in symbols and '_Z4pickPA_i' in symbols
# Empty representation does not relax positive alignment or bulk-span rules.
for name,body in [('alignment','slot $x : obj<0x0>'),('copy','slot $x : obj<0x4>\n block ^entry:\n copyobj 0x4 $x, $x'),('zero','slot $x : obj<0x4>\n block ^entry:\n zeroinit 0x4 $x')]:
 if name=='alignment':body+='\n block ^entry:'
 path=out/(name+'.lowir');path.write_text('function @main() -> i32 {\n '+body+'\n return i32 0\n}\n')
 run([root/'dev/lowir',path],ok=False)
path=out/'untyped-empty.lowir';path.write_text('global @empty = {}\n')
run([root/'dev/lowir',path],ok=False)
print(len(rows),'inspection commands passed')
