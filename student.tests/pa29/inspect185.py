#!/usr/bin/env python3
"""Bit-integer identity, ABI adapters and native inspection."""
import pathlib,subprocess,sys,json,hashlib,re
root=pathlib.Path(__file__).resolve().parents[2];out=pathlib.Path(sys.argv[1]).resolve();out.mkdir(parents=True,exist_ok=True)
cc=root/'dev/cppgm++';rows=[];cases=[]
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def save():(out/'inspection.json').write_text(json.dumps(dict(compiler_sha256=sha(cc),rows=rows,cases=cases),indent=2)+'\n')
def run(args):
 args=list(map(str,args));p=subprocess.run(args,cwd=root,text=True,capture_output=True,timeout=120)
 rows.append(dict(command=args,status=p.returncode,stdout=p.stdout,stderr=p.stderr));save();assert not p.returncode,rows[-1]
 return p.stdout
objects=[p for p in sorted((root/'obj/dev').rglob('*.o')) if 'entry' not in p.parts and not p.name.startswith('test_runner')]
for source,name in [('inspection185-facts.cpp','facts'),('inspection180-object.cpp','adapter')]:
 run(['g++','-std=c++11','-O2','-I'+str(root/'dev/src'),root/'student.tests/pa29'/source,*objects,'-o',out/name])
for src in sorted((root/'student.tests/pa29/source185').glob('*.cpp')):
 if '.reject.' in src.name:continue
 name=src.stem;ir=out/(name+'.lowir');obj=out/(name+'.o');ao=out/(name+'.adapter.o')
 facts=json.loads(run([out/'facts',src]));assert facts['concrete_types'] and facts['abi_adapter_cases']
 run([cc,'--emit-ast',src,'-o',out/(name+'.ast')])
 run([cc,'-c','--emit-lowir','--validate-lowir',src,'-o',ir])
 run([root/'dev/lowir',ir,'-o',out/(name+'.roundtrip')]);assert ir.read_bytes()==(out/(name+'.roundtrip')).read_bytes()
 run([out/'adapter',ir,out/(name+'.prepared.lowir'),ao])
 run([cc,'-O0','--stats','-c',src,'-o',obj]);metrics={}
 for line in rows[-1]['stderr'].splitlines():
  if line.startswith('{'):metrics.update(json.loads(line))
 run([cc,'-O0','-c',src,'-o',out/(name+'.plain.o')]);assert obj.read_bytes()==(out/(name+'.plain.o')).read_bytes()
 # Serialized symbol order may differ; compare every complete named section.
 def sections(path):
  chunks=re.split(r'Disassembly of section ',run(['objdump','-dr',path]))[1:]
  return sorted(c.strip() for c in chunks)
 assert sections(obj)==sections(ao),(name,'native section mismatch')
 assert sorted(run(['nm',obj]).splitlines())==sorted(run(['nm',ao]).splitlines())
 helpers=[]
 if name=='abi':
  helper=out/'abi.host.o';run(['clang++','-std=c++11','-c',root/'student.tests/pa29/support185/abi.host.cpp','-o',helper]);helpers=[helper]
 run(['g++',ao,*helpers,'-o',out/(name+'.exe')]);run([out/(name+'.exe')])
 if name in ['abi','abi-template']:
  clang=out/(name+'.clang.o');run(['clang++','-std=c++11','-c',src,'-o',clang])
  def symbols(path):return sorted(l.split()[-1] for l in run(['nm','--defined-only',path]).splitlines() if l.split() and l.split()[-1].startswith('_Z') and (name!='abi' or l.split()[-2]=='T'))
  assert symbols(obj)==symbols(clang),(name,'host symbol mismatch')
 run([root/'dev/lowir2native','--dump-machine-ir',out/(name+'.mir'),ir]);run(['readelf','-rSW',obj]);run(['readelf','--debug-dump=frames',obj])
 cases.append(dict(source=str(src.relative_to(root)),source_sha256=sha(src),facts=facts,metrics=metrics,object_sha256=sha(obj),adapter_sha256=sha(ao),lowir_sha256=sha(ir)));save()
 print(name,'pass',flush=True)
print(len(rows),'commands passed',flush=True)
