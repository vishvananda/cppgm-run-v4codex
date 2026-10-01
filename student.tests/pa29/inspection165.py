#!/usr/bin/env python3
"""Validate actual typed IR, native behavior, and telemetry transparency."""
import hashlib,json,pathlib,subprocess,sys
root=pathlib.Path(__file__).resolve().parents[2]
out=pathlib.Path(sys.argv[1]).resolve();out.mkdir(parents=True,exist_ok=True)
cc=root/'dev/cppgm++';rows=[]
def run(name,args):
 p=subprocess.run(list(map(str,args)),cwd=root,capture_output=True,timeout=90)
 rows.append(dict(name=name,args=list(map(str,args)),status=p.returncode,passed=p.returncode==0,stdout=p.stdout.decode(errors='replace'),stderr=p.stderr.decode(errors='replace')))
 if p.returncode:print(name,p.stderr.decode(),flush=True)
 return p.returncode==0
for src in sorted((root/'student.tests/pa29/controls165').glob('*.cpp')):
 name=src.stem;low=out/(name+'.lowir');rt=out/(name+'.roundtrip')
 if run(name+' typed IR',[cc,'-std=c++14','-c','--emit-lowir','--validate-lowir',src,'-o',low]):
  if run(name+' roundtrip',[root/'dev/lowir',low,'-o',rt]):rows.append(dict(name=name+' IR equality',passed=low.read_bytes()==rt.read_bytes()))
  if run(name+' native MIR',[root/'dev/lowir2native','--dump-machine-ir',out/(name+'.mir'),low,'-o',out/(name+'-native')]):run(name+' native execution',[out/(name+'-native')])
 obj=out/(name+'.o');stats=out/(name+'-stats.o')
 if run(name+' object',[cc,'-std=c++14','-O0','-c',src,'-o',obj]) and run(name+' telemetry',[cc,'-std=c++14','--stats','-O0','-c',src,'-o',stats]):
  rows.append(dict(name=name+' telemetry byte equality',passed=obj.read_bytes()==stats.read_bytes()))
(out/'inspection.json').write_text(json.dumps(dict(compiler_sha256=hashlib.sha256(cc.read_bytes()).hexdigest(),checks=rows),indent=2)+'\n')
print('%d/%d checks passed'%(sum(r['passed'] for r in rows),len(rows)),flush=True)
sys.exit(any(not r['passed'] for r in rows))
