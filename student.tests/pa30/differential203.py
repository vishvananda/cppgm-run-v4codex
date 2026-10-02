#!/usr/bin/env python3
import hashlib,json,pathlib,subprocess,sys,time
root=pathlib.Path(__file__).resolve().parents[2]
out=pathlib.Path(sys.argv[1]).resolve();out.mkdir(parents=True,exist_ok=True)
compiler=pathlib.Path(sys.argv[2]).resolve() if len(sys.argv)>2 else root/'dev/cppgm++'
r={'compiler_sha256':hashlib.sha256(compiler.read_bytes()).hexdigest(),'cases':[]}
for src in sorted((root/'student.tests/pa30/generated203').glob('*.cpp')):
 row={'source':str(src.relative_to(root)),'sha256':hashlib.sha256(src.read_bytes()).hexdigest(),'commands':[]}
 results={}
 for label,cc in [('host','g++'),('student',str(compiler))]:
  obj=out/(src.stem+label+'.o');exe=out/(src.stem+label)
  for cmd in [[cc,'-O0','-c',str(src),'-o',str(obj)],['g++',str(obj),'-o',str(exe)],[str(exe)],[str(exe),'vary']]:
   start=time.perf_counter();p=subprocess.run(cmd,capture_output=True,text=True,timeout=45)
   row['commands'].append(dict(label=label,args=cmd,status=p.returncode,stdout=p.stdout,stderr=p.stderr,wall_s=time.perf_counter()-start))
   if p.returncode:break
   if cmd[0]==str(exe):results.setdefault(label,[]).append(p.stdout)
 row['equal']=results.get('host')==results.get('student') and len(results.get('host',[]))==2
 r['cases'].append(row);(out/'differential.json').write_text(json.dumps(r,indent=2)+'\n')
 if not row['equal']:print(src.stem,'FAIL',row['commands'][-1],flush=True)
print(sum(x['equal'] for x in r['cases']),'/',len(r['cases']),flush=True)

assert hashlib.sha256(compiler.read_bytes()).hexdigest()==r["compiler_sha256"]
assert all(c["equal"] for c in r["cases"])
