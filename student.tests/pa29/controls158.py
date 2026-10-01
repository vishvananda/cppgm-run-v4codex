#!/usr/bin/env python3
"""Explicit checkpoint audit controls; never discovered by the course harness."""
import hashlib,json,pathlib,subprocess,sys
root=pathlib.Path(__file__).resolve().parents[2]
out=pathlib.Path(sys.argv[1] if len(sys.argv)>1 else '/tmp/pa29-158/controls').resolve();out.mkdir(parents=True,exist_ok=True)
cc=root/'dev/cppgm++';rows=[]
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def run(name,args,ok=True):
 args=list(map(str,args));p=subprocess.run(args,capture_output=True,timeout=90,cwd=root)
 good=(p.returncode==0)==ok
 rows.append(dict(name=name,args=args,status=p.returncode,passed=good,stdout=p.stdout.decode(errors='replace'),stderr=p.stderr.decode(errors='replace')))
 if not good:print(name,p.returncode,p.stderr.decode(errors='replace'),flush=True)
 return good
for src in sorted((root/'student.tests/pa29/controls158').glob('*.cpp')):
 reject=src.stem.endswith('-reject');obj=out/(src.stem+'.o');exe=out/src.stem
 if run(src.stem+' compile',[cc,'-c',src,'-o',obj],not reject) and not reject:
  if run(src.stem+' link',['g++',obj,'-o',exe]):run(src.stem+' run',[exe])
for name in ['storage','storage-flow']:
 src=root/'student.tests/pa29/controls158'/(name+'.cpp');low=out/(name+'.lowir');rt=out/(name+'-roundtrip.lowir');exe=out/(name+'-native')
 if run(name+' typed LowIR',[cc,'-c','--emit-lowir','--validate-lowir',src,'-o',low]):
  if run(name+' reader',[root/'dev/lowir',low,'-o',rt]):
   rows.append(dict(name=name+' roundtrip bytes',passed=low.read_bytes()==rt.read_bytes()))
   if run(name+' native MIR',[root/'dev/lowir2native','--dump-machine-ir',out/(name+'.mir'),rt,'-o',exe]):run(name+' roundtrip execute',[exe])
 obj=out/(name+'.o');stats=out/(name+'-stats.o')
 if run(name+' stats',[cc,'-c','--stats',src,'-o',stats]):rows.append(dict(name=name+' telemetry bytes',passed=obj.read_bytes()==stats.read_bytes()))
manifest=dict(compiler_sha256=sha(cc),checks=rows,files=[dict(path=str(p),sha256=sha(p),bytes=p.stat().st_size) for p in sorted(out.iterdir()) if p.is_file() and p.name!='results.json'])
(out/'results.json').write_text(json.dumps(manifest,indent=2)+'\n')
print('%d/%d audit controls passed'%(sum(r['passed'] for r in rows),len(rows)),flush=True)
sys.exit(any(not r['passed'] for r in rows))
