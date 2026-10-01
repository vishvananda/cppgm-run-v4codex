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
for name in ['vector','bitcast','vector-many','vector-constant','bitcast-storage','sfinae']:
 for level in ['-O0','-O2']:
  obj=out/(name+'.o');exe=out/name
  if run([cc,level,'-c',src/(name+'.cpp'),'-o',obj]):
   if run(['g++',obj,'-o',exe]):run([exe])
 lir=out/(name+'.lowir')
 if run([cc,'-c','--emit-lowir','--validate-lowir',src/(name+'.cpp'),'-o',lir]):
  run([root/'dev/lowir',lir,'-o',out/(name+'.canonical')])
 if name=='sfinae':
  # Clang validates this extension but cannot encode BuiltinBitCastExpr in a symbol.
  run(['clang++','-std=c++11','-fsyntax-only',src/(name+'.cpp')])
 else:
  if run(['clang++','-std=c++11',src/(name+'.cpp'),'-o',out/'host']):run([out/'host'])
if run([cc,'-c',src/'abi-source.cpp','-o',out/'abi.o']):
 if run(['clang++','-std=c++11',src/'abi-host.cpp',out/'abi.o','-o',out/'abi']):run([out/'abi'])
 run(['objdump','-dr',out/'abi.o']);run(['readelf','-SWsrg',out/'abi.o']);run(['readelf','--debug-dump=frames',out/'abi.o'])
print(len(rows),'commands,',len(failed),'failures')
for r in failed: print(' '.join(r['command']),r['status'],r['stderr'])
sys.exit(bool(failed))
