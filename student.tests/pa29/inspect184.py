#!/usr/bin/env python3
"""Cast fact ownership and source/serialized LowIR/native inspection."""
import pathlib,subprocess,sys,json,hashlib
root=pathlib.Path(__file__).resolve().parents[2];out=pathlib.Path(sys.argv[1]).resolve();out.mkdir(parents=True,exist_ok=True)
cc=root/'dev/cppgm++';rows=[];cases=[]
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def save():(out/'inspection.json').write_text(json.dumps(dict(compiler_sha256=sha(cc),rows=rows,cases=cases),indent=2)+'\n')
def run(args):
 args=list(map(str,args));p=subprocess.run(args,cwd=root,text=True,capture_output=True,timeout=120)
 rows.append(dict(command=args,status=p.returncode,stdout=p.stdout,stderr=p.stderr));save();assert not p.returncode,rows[-1]
 return p.stdout
objects=[p for p in sorted((root/'obj/dev').rglob('*.o')) if 'entry' not in p.parts and not p.name.startswith('test_runner')]
for source,name in [('inspection184-facts.cpp','facts'),('inspection180-object.cpp','adapter')]:
 run(['g++','-std=c++11','-O2','-I'+str(root/'dev/src'),root/'student.tests/pa29'/source,*objects,'-o',out/name])
for src in sorted((root/'student.tests/pa29/source184').glob('*.cpp')):
 if '.reject.' in src.name:continue
 name=src.stem;ir=out/(name+'.lowir');obj=out/(name+'.o');ao=out/(name+'.adapter.o')
 facts=json.loads(run([out/'facts',src]));casts=facts['casts']
 if name=='qualification':assert casts and all(not c['constant_forbidden'] for c in casts)
 if name=='base-adjustments':assert any(c['derived'] and c['offset'] for c in casts)
 if name=='reinterpret-runtime':assert any(c['constant_forbidden'] for c in casts)
 if name=='dependent':assert any(c['context'] for c in casts)
 run([cc,'--emit-ast',src,'-o',out/(name+'.ast')])
 run([cc,'-c','--emit-lowir','--validate-lowir',src,'-o',ir])
 run([root/'dev/lowir',ir,'-o',out/(name+'.roundtrip')]);assert ir.read_bytes()==(out/(name+'.roundtrip')).read_bytes()
 run([out/'adapter',ir,out/(name+'.prepared.lowir'),ao])
 run([cc,'-O0','--stats','-c',src,'-o',obj]);metrics={}
 for line in rows[-1]['stderr'].splitlines():
  if line.startswith('{'):metrics.update(json.loads(line))
 run([cc,'-O0','-c',src,'-o',out/(name+'.plain.o')]);assert obj.read_bytes()==(out/(name+'.plain.o')).read_bytes()
 direct=run(['objdump','-dr',obj]).splitlines()[3:];adapter=run(['objdump','-dr',ao]).splitlines()[3:];assert direct==adapter
 assert sorted(run(['nm',obj]).splitlines())==sorted(run(['nm',ao]).splitlines())
 run(['g++',ao,'-o',out/(name+'.exe')]);run([out/(name+'.exe')])
 run([root/'dev/lowir2native','--dump-machine-ir',out/(name+'.mir'),ir]);run(['readelf','-rSW',obj]);run(['readelf','--debug-dump=frames',obj])
 cases.append(dict(source=str(src.relative_to(root)),source_sha256=sha(src),facts=facts,metrics=metrics,object_sha256=sha(obj),adapter_sha256=sha(ao),lowir_sha256=sha(ir)));save()
 print(name,'pass',flush=True)
print(len(rows),'commands passed',flush=True)
