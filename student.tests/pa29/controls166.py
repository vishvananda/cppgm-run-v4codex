#!/usr/bin/env python3
"""Independent accumulated-audit reducers and cross-owner controls, run explicitly."""
import hashlib,json,pathlib,subprocess,sys
root=pathlib.Path(__file__).resolve().parents[2]
out=pathlib.Path(sys.argv[1]).resolve();out.mkdir(parents=True,exist_ok=True)
cc=pathlib.Path(sys.argv[2]).resolve() if len(sys.argv)>2 else root/'dev/cppgm++'
rows=[]
def run(name,args):
 p=subprocess.run(list(map(str,args)),cwd=root,capture_output=True,timeout=90)
 rows.append(dict(name=name,args=list(map(str,args)),status=p.returncode,passed=p.returncode==0,stdout=p.stdout.decode(errors='replace'),stderr=p.stderr.decode(errors='replace')))
 if p.returncode:print(name,p.returncode,p.stderr.decode(),flush=True)
 return p.returncode==0
for src in sorted((root/'student.tests/pa29/controls166').glob('*.cpp')):
 name=src.stem;obj=out/(name+'.o');exe=out/name
 if run(name+' compile',[cc,'-std=c++14','-O0','-c',src,'-o',obj]):
  if run(name+' link',['g++',obj,'-o',exe]):run(name+' runtime',[exe])
(out/'results.json').write_text(json.dumps(dict(compiler_sha256=hashlib.sha256(cc.read_bytes()).hexdigest(),inputs={str(p.relative_to(root)):hashlib.sha256(p.read_bytes()).hexdigest() for p in sorted((root/'student.tests/pa29/controls166').glob('*.cpp'))},checks=rows),indent=2)+'\n')
print('%d/%d checks passed'%(sum(r['passed'] for r in rows),len(rows)),flush=True)
sys.exit(any(not r['passed'] for r in rows))
