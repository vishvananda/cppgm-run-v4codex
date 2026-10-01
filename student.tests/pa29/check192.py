#!/usr/bin/env python3
"""Explicit extended-format correctness/ABI/inspection controls."""
import json,pathlib,subprocess,sys
root=pathlib.Path(__file__).resolve().parents[2]
out=pathlib.Path(sys.argv[1]).resolve();out.mkdir(parents=True,exist_ok=True)
cc=root/'dev/cppgm++';src=root/'student.tests/pa29/source192';rows=[];failed=[]
def run(args,ok=True):
 args=list(map(str,args));p=subprocess.run(args,cwd=root,capture_output=True,text=True,timeout=90)
 row=dict(command=args,status=p.returncode,expected_success=ok,stdout=p.stdout,stderr=p.stderr);rows.append(row)
 (out/'checks.json').write_text(json.dumps(rows,indent=2)+'\n')
 if (p.returncode==0)!=ok:failed.append(row);return False
 return True
objects=subprocess.check_output(['make','-s','-C',str(root/'dev'),'--eval','print-objects: ; @echo $(call frontend_objs,cppgm++) $(call frontend_objs,lowir)','print-objects'],text=True).split()
objects=list(dict.fromkeys(str((root/'dev'/p).resolve()) for p in objects))
assert run(['g++','-std=c++11','-I'+str(root/'dev/src'),src/'lowir-host.cpp',*objects,'-o',out/'lowir-host'])
for name in ['extended','template','quad-token','legacy']:
 for level in ['-O0','-O2']:
  obj=out/(name+level+'.o');exe=out/(name+level)
  if run([cc,level,'-c',src/(name+'.cpp'),'-o',obj]):
   if run(['g++',obj,'-o',exe]):run([exe])
 lir=out/(name+'.lowir');canonical=out/(name+'.canonical.lowir')
 if run([cc,'-c','--emit-lowir','--validate-lowir',src/(name+'.cpp'),'-o',lir]):
  if run([root/'dev/lowir',lir,'-o',canonical]):
   if run([out/'lowir-host',canonical,out/'roundtrip.o']):
    if run(['g++',out/'roundtrip.o','-o',out/'roundtrip']):run([out/'roundtrip'])
   run([root/'dev/lowir2native','--dump-machine-ir',out/(name+'.mir'),canonical])
 if run(['g++','-std=gnu++11',src/(name+'.cpp'),'-o',out/'host']):run([out/'host'])
for level in ['-O0','-O2']:
 if run([cc,level,'-c',src/'abi.cpp','-o',out/'abi.o']):
  if run(['g++','-std=gnu++11',src/'abi-host.cpp',out/'abi.o','-lquadmath','-o',out/'abi']):run([out/'abi'])
  run(['objdump','-dr',out/'abi.o']);run(['readelf','-SWsrg',out/'abi.o']);run(['readelf','--debug-dump=frames',out/'abi.o'])
for p in sorted(src.glob('*.reject.cpp')):
 run([cc,'-std=gnu++11','-c',p,'-o',out/'reject.o'],False)
 if not p.name.startswith('narrow-'):run(['g++','-std=gnu++11','-c',p,'-o',out/'reject.o'],False)
run([cc,'-E',src/'quad-token.t'])
if run([out/'lowir-host',src/'truth.lowir',out/'truth.o']):
 if run(['g++','-std=gnu++11',src/'truth-host.cpp',out/'truth.o','-o',out/'truth']):run([out/'truth'])
print(len(rows),'commands,',len(failed),'failures')
for r in failed:print(' '.join(r['command']),r['status'],r['stderr'])
sys.exit(bool(failed))
