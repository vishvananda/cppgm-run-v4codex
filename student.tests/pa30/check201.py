#!/usr/bin/env python3
"""Explicit semantic and object/LowIR controls for the PA30 function ownership and reachability owners."""
import hashlib,json,pathlib,subprocess,sys,time
root=pathlib.Path(__file__).resolve().parents[2]
out=pathlib.Path(sys.argv[1]).resolve();out.mkdir(parents=True,exist_ok=True)
compiler=pathlib.Path(sys.argv[2]).resolve() if len(sys.argv)>2 else root/'dev/cppgm++'
r=dict(compiler_sha256=hashlib.sha256(compiler.read_bytes()).hexdigest(),commands=[])
def run(args,reject=False):
 args=list(map(str,args));start=time.perf_counter();p=subprocess.run(args,cwd=root,capture_output=True,text=True,timeout=45)
 r['commands'].append(dict(args=args,status=p.returncode,wall_s=time.perf_counter()-start,stdout=p.stdout,stderr=p.stderr,expected_rejection=reject))
 (out/'controls.json').write_text(json.dumps(r,indent=2)+'\n')
 assert (p.returncode!=0)==reject,r['commands'][-1]
for source in sorted((root/'student.tests/pa30/source201').glob('*.cpp')):
 reject='.reject.' in source.name
 obj=out/(source.stem+'.o');exe=out/source.stem;ir=out/(source.stem+'.lowir')
 run([compiler,'-O0','-c',source,'-o',obj],reject)
 if reject:continue
 run(['g++',obj,'-o',exe]);run([exe])
 run([compiler,'-c','--emit-lowir','--validate-lowir',source,'-o',ir])
print(len(r['commands']),'controls passed',flush=True)
