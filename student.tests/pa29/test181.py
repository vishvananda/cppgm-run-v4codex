#!/usr/bin/env python3
"""GNU zero-extent controls, with checked executions and independent comparisons."""
import hashlib,json,pathlib,subprocess,sys
root=pathlib.Path(__file__).resolve().parents[2]
out=pathlib.Path(sys.argv[1]).resolve();out.mkdir(parents=True,exist_ok=True)
compiler=pathlib.Path(sys.argv[2] if len(sys.argv)>2 else root/'dev/cppgm++').resolve()
def run(args):
 args=list(map(str,args));p=subprocess.run(args,capture_output=True,text=True,timeout=60)
 return dict(command=args,status=p.returncode,stdout=p.stdout,stderr=p.stderr)
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
results=[]
for src in sorted((root/'student.tests/pa29/source181').glob('*.cpp')):
 ok='.reject.' not in src.name;name=src.stem;row=dict(name=name,accepted=ok,source_sha256=sha(src),checks=[])
 for label,binary,flags in [('student',compiler,['-std=c++11']),('host','g++' if name in ['zero-ctor','zero-ctor-new','zero-ctor-dynamic','zero-ctor-unwind','multidimensional-new'] else 'clang++',['-std=gnu++11'])]:
  obj=out/(name+label+'.o');exe=out/(name+label)
  r=run([binary,*flags,'-c',src,'-o',obj]);row['checks'].append(r)
  if (r['status']==0)!=ok:row['failure']=label+' acceptance'
  if ok and not r['status']:
   r=run(['g++',obj,'-o',exe]);row['checks'].append(r)
   if r['status']:row['failure']=label+' link'
   else:
    r=run([exe]);row['checks'].append(r)
    expected = 1 if label=='host' and name in ['zero-ctor','zero-ctor-new','zero-ctor-dynamic','zero-ctor-unwind','multidimensional-new'] else 0
    if expected:row['host_difference']='GNU host skips destructor effects for arrays of zero-size class; compiler preserves one call per element'
    if r['status']!=expected:row['failure']=label+' runtime'
 if ok:
  r=run([compiler,'-c','--emit-lowir','--validate-lowir',src,'-o',out/(name+'.lowir')]);row['checks'].append(r)
  if r['status']:row['failure']='LowIR audit'
 results.append(row)
 (out/'controls.json').write_text(json.dumps(dict(compiler_sha256=sha(compiler),cases=results),indent=2)+'\n')
 print(name,row.get('failure','pass'),flush=True)
assert not [r for r in results if 'failure' in r]
