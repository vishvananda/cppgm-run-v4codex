#!/usr/bin/env python3
import hashlib,json,pathlib,subprocess,sys,time
root=pathlib.Path(__file__).resolve().parents[2]
out=pathlib.Path(sys.argv[1]).resolve();out.mkdir(parents=True,exist_ok=True)
r={'compiler_sha256':hashlib.sha256((root/'dev/cppgm++').read_bytes()).hexdigest(),'commands':[]}
def run(cmd,reject=False):
 cmd=list(map(str,cmd));start=time.perf_counter();p=subprocess.run(cmd,capture_output=True,text=True,timeout=45)
 r['commands'].append(dict(args=cmd,status=p.returncode,stdout=p.stdout,stderr=p.stderr,expected_rejection=reject,wall_s=time.perf_counter()-start))
 (out/'controls.json').write_text(json.dumps(r,indent=2)+'\n');assert (p.returncode!=0)==reject,r['commands'][-1]
for src in sorted((root/'student.tests/pa30/source203').glob('*.cpp')):
 obj=out/(src.stem+'.o');exe=out/src.stem;ir=out/(src.stem+'.lowir');reject='.reject.' in src.name
 run([root/'dev/cppgm++','-c',src,'-o',obj],reject)
 if reject:continue
 run(['g++',obj,'-o',exe]);run([exe]);run([root/'dev/cppgm++','--emit-lowir','--validate-lowir',src,'-o',ir])
print(len(r['commands']),'controls pass',flush=True)
