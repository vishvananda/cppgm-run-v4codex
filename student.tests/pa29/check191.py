#!/usr/bin/env python3
"""Explicit PA29 vector and representation builtin controls, outside course discovery."""
import json,pathlib,subprocess,sys
root=pathlib.Path(__file__).resolve().parents[2]
out=pathlib.Path(sys.argv[1]).resolve();out.mkdir(parents=True,exist_ok=True)
cc=root/'dev/cppgm++';src=root/'student.tests/pa29/controls191';rows=[];failed=[]
def run(args,ok=True):
 args=list(map(str,args));p=subprocess.run(args,cwd=root,capture_output=True,text=True,timeout=90)
 row=dict(command=args,status=p.returncode,expected_success=ok,stdout=p.stdout,stderr=p.stderr);rows.append(row)
 (out/'checks.json').write_text(json.dumps(rows,indent=2)+'\n')
 if (p.returncode==0)!=ok:failed.append(row);return False
 return True
for name in ['vector','bitcast','vector-many','vector-constant','bitcast-storage','sfinae','vector-static','bitcast-deleted','bitcast-array','builtin-effects','bitcast-special','vector-mask-types']:
 for level in ['-O0','-O2']:
  obj=out/(name+'.o');exe=out/name
  if run([cc,level,'-c',src/(name+'.cpp'),'-o',obj]):
   if run(['g++',obj,'-o',exe]):run([exe])
 lir=out/(name+'.lowir')
 if run([cc,'-c','--emit-lowir','--validate-lowir',src/(name+'.cpp'),'-o',lir]):
  canonical=out/(name+'.canonical')
  if run([root/'dev/lowir',lir,'-o',canonical]):
   if run([root/'dev/lowir2native','--dump-machine-ir',out/(name+'.mir'),canonical,'-o',out/'roundtrip']):run([out/'roundtrip'])
 if name=='sfinae':
  # Clang validates this extension but cannot encode BuiltinBitCastExpr in a symbol.
  run(['clang++','-std=c++11','-fsyntax-only',src/(name+'.cpp')])
 else:
  if run(['clang++','-std=c++11',src/(name+'.cpp'),'-o',out/'host']):run([out/'host'])
if run([cc,'-c',src/'abi-source.cpp','-o',out/'abi.o']):
 if run(['clang++','-std=c++11',src/'abi-host.cpp',out/'abi.o','-o',out/'abi']):run([out/'abi'])
 run(['objdump','-dr',out/'abi.o']);run(['readelf','-SWsrg',out/'abi.o']);run(['readelf','--debug-dump=frames',out/'abi.o'])
for p in sorted(src.glob('*.reject.cpp')):
 for compiler in [cc,'clang++']:run([compiler,'-std=c++11','-c',p,'-o',out/'reject.o'],False)
if run([cc,'-c',src/'gnu-abi-source.cpp','-o',out/'gnu.o']):
 if run(['g++','-std=c++11','-mno-avx',src/'gnu-abi-host.cpp',out/'gnu.o','-o',out/'gnu']):run([out/'gnu'])
if run([cc,'-c',src/'qualification.cpp','-o',out/'qualification.o']):
 if run(['g++',out/'qualification.o','-o',out/'qualification']):run([out/'qualification'])
sets=(root/'dev/frontend_source_sets.mk').read_text().splitlines()
names=next(s for s in sets if s.startswith('FRONTEND_OBJ_BASENAMES_abimangle :=')).split(':=')[1].split()
objects=[root/'obj/dev'/(name+'.o') for name in names]
if run(['g++','-std=c++11','-I'+str(root/'dev/src'),src/'abi-adapter.cpp',*objects,'-o',out/'adapter']):run([out/'adapter'])
print(len(rows),'commands,',len(failed),'failures')
for r in failed: print(' '.join(r['command']),r['status'],r['stderr'])
sys.exit(bool(failed))
