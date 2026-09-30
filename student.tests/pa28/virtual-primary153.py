#!/usr/bin/env python3
"""Bidirectional host ABI layout, VTT, covariance and lifetime controls."""
import hashlib,json,pathlib,subprocess,sys
root=pathlib.Path(__file__).resolve().parents[2]
out=pathlib.Path(sys.argv[1]).resolve();out.mkdir(parents=True,exist_ok=True)
compiler=pathlib.Path(sys.argv[2]).resolve() if len(sys.argv)>2 else root/'dev/cppgm++'
records=[]
def run(args):
 p=subprocess.run(list(map(str,args)),cwd=root,capture_output=True,timeout=60)
 records.append(dict(args=list(map(str,args)),status=p.returncode,stdout=p.stdout.decode(),stderr=p.stderr.decode()))
 (out/'controls.json').write_text(json.dumps(dict(commands=records),indent=2)+'\n')
 assert not p.returncode,records[-1]
 return p.stdout
for tool,cc in [('H','g++'),('S',compiler)]:
 for unit in ['producer','consumer']:
  args=[cc,'-c',root/f'student.tests/pa28/virtual-primary153-{unit}.cpp','-o',out/f'{tool}-{unit}.o']
  if tool=='H':args[1:1]=['-std=c++11','-O0']
  run(args)
expected=None
for pair in ['HH','SH','HS','SS']:
 exe=out/pair
 run(['g++',out/f'{pair[0]}-producer.o',out/f'{pair[1]}-consumer.o','-o',exe])
 actual=run([exe])
 if expected is None:expected=actual
 assert actual==expected,(pair,actual.decode(),expected.decode())
print('virtual-primary153 PASS:',len(records),'commands')
