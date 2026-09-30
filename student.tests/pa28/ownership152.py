#!/usr/bin/env python3
import pathlib,subprocess,json,sys
root=pathlib.Path(__file__).resolve().parents[2]; here=root/'student.tests/pa28'
out=pathlib.Path(sys.argv[1] if len(sys.argv)>1 else '/tmp/pa28-152/ownership').resolve();out.mkdir(parents=True,exist_ok=True)
records=[]
def run(args):
 p=subprocess.run(list(map(str,args)),cwd=root,capture_output=True,timeout=30)
 records.append(dict(args=list(map(str,args)),status=p.returncode,stdout=p.stdout.decode(),stderr=p.stderr.decode()))
 (out/'checks.json').write_text(json.dumps(records,indent=2)+'\n')
 assert not p.returncode,records[-1]
 return p.stdout.decode()
for tool,src,obj in [('dev/cppgm++','ownership-client152.cpp','client.o'),('g++','ownership-host152.cpp','host.o'),('dev/cppgm++','lazy-client152.cpp','lazy.o'),('dev/cppgm++','lazy-owner152.cpp','owner.o')]:
 run([tool,'-std=c++11','-c',here/src,'-o',out/obj])
for name,objects in [('ownership',['client.o','host.o']),('lazy',['lazy.o','owner.o'])]:
 run(['g++',*[out/o for o in objects],'-o',out/name]);run([out/name])
symbols=run(['nm',out/'client.o'])
assert '.cppgm.view.' not in symbols and '__cppgm_construction_vtable_' not in symbols
for line in symbols.splitlines():
 if '_ZTV' in line or '_ZTT' in line: assert line.split()[0] == 'U',line
assert '_ZN11HostLeaf152D1Ev' in symbols and '_ZN11HostLeaf152D2Ev' not in symbols
print('ownership controls PASS:',len(records),'commands')
