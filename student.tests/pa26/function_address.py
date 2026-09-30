#!/usr/bin/env python3
"""Imported function identity through a host DSO and default-PIE final link."""
import json,pathlib,subprocess,sys
out=pathlib.Path(sys.argv[1] if len(sys.argv)>1 else '/tmp/pa26-143/function-address').resolve();out.mkdir(parents=True,exist_ok=True)
root=pathlib.Path(__file__).resolve().parent;records=[]
def run(args):
 r=subprocess.run(list(map(str,args)),capture_output=True,text=True);records.append(dict(command=list(map(str,args)),status=r.returncode,stdout=r.stdout,stderr=r.stderr))
 (out/'results.json').write_text(json.dumps(records,indent=2)+'\n');assert r.returncode==0,records[-1];return r.stdout
run(['g++','-fPIC','-shared',root/'function_address-host.cpp','-o',out/'libaddress.so'])
run(['dev/cppgm++','-c',root/'function_address.cpp','-o',out/'source.o'])
relocs=run(['readelf','-rW',out/'source.o']);assert any('GOTPCREL' in s and 'imported' in s for s in relocs.splitlines())
assert not any('GOTPCREL' in s and 'local' in s for s in relocs.splitlines())
run(['g++',out/'source.o',out/'libaddress.so','-Wl,-rpath,'+str(out),'-o',out/'program']);run([out/'program'])
print('Imported function pointer identity, indirect calls, local address and PIE relocations pass')
