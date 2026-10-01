#!/usr/bin/env python3
"""Bit-precise integer controls."""
import hashlib,json,pathlib,subprocess,sys
root=pathlib.Path(__file__).resolve().parents[2]
out=pathlib.Path(sys.argv[1]).resolve();out.mkdir(parents=True,exist_ok=True)
cc=pathlib.Path(sys.argv[2] if len(sys.argv)>2 else root/'dev/cppgm++').resolve()
rows=[]
def run(args):
 args=list(map(str,args));p=subprocess.run(args,capture_output=True,text=True,timeout=60)
 return dict(command=args,status=p.returncode,stdout=p.stdout,stderr=p.stderr)
for src in sorted((root/'student.tests/pa29/source185').glob('*.cpp')):
 ok='.reject.' not in src.name
 row=dict(source=str(src.relative_to(root)),sha256=hashlib.sha256(src.read_bytes()).hexdigest(),accept=ok,checks=[])
 for label,binary,standard in [('student',cc,'c++11'),('clang','clang++','c++11')]:
  obj=out/(src.stem+label+'.o');exe=out/(src.stem+label)
  r=run([binary,'-std='+standard,'-c',src,'-o',obj]);row['checks'].append(r)
  if (r['status']==0)!=ok:row['failure']=label+' acceptance'
  if ok and r['status']==0:
   helpers=[]
   if src.stem=='abi':
    helper=out/'abi.host.o';hr=run(['clang++','-std=c++11','-c',root/'student.tests/pa29/support185/abi.host.cpp','-o',helper]);row['checks'].append(hr);assert hr['status']==0;helpers=[helper]
   r=run(['g++',obj,*helpers,'-o',exe]);row['checks'].append(r)
   if r['status']:row['failure']=label+' link'
   else:
    r=run([exe]);row['checks'].append(r)
    if r['status']:row['failure']=label+' run'
 if ok:
  r=run([cc,'-c','--emit-lowir','--validate-lowir',src,'-o',out/(src.stem+'.lowir')]);row['checks'].append(r)
  if r['status']:row['failure']='LowIR validation'
 rows.append(row);print(src.name,row.get('failure','pass'),flush=True)
 (out/'controls.json').write_text(json.dumps(dict(compiler_sha256=hashlib.sha256(cc.read_bytes()).hexdigest(),cases=rows),indent=2)+'\n')
sys.exit(any('failure' in r for r in rows))
