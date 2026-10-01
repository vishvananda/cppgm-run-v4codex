#!/usr/bin/env python3
"""Explicit audit reducers, including cross-handoff storage/conversion recipes."""
import hashlib,json,pathlib,subprocess,sys
root=pathlib.Path(__file__).resolve().parents[2]
out=pathlib.Path(sys.argv[1]).resolve();out.mkdir(parents=True,exist_ok=True)
cc=pathlib.Path(sys.argv[2]).resolve() if len(sys.argv)>2 else root/'dev/cppgm++'
rows=[]
def run(name,args,ok=True):
 p=subprocess.run(list(map(str,args)),cwd=root,capture_output=True,timeout=90)
 good=(p.returncode==0)==ok
 rows.append(dict(name=name,args=list(map(str,args)),status=p.returncode,passed=good,stdout=p.stdout.decode(errors='replace'),stderr=p.stderr.decode(errors='replace')))
 if not good:print(name,p.returncode,p.stderr.decode(errors='replace'),flush=True)
 return good
for src in sorted((root/'student.tests/pa29/controls162').glob('*.cpp')):
 reject=src.stem.endswith('-reject');obj=out/(src.stem+'.o');exe=out/src.stem
 if run(src.stem+' compile',[cc,'-c',src,'-o',obj],not reject) and not reject:
  if run(src.stem+' link',['g++',obj,'-latomic','-o',exe]):run(src.stem+' run',[exe])
result=dict(compiler_sha256=hashlib.sha256(cc.read_bytes()).hexdigest(),checks=rows)
(out/'results.json').write_text(json.dumps(result,indent=2)+'\n')
print('%d/%d controls passed'%(sum(r['passed'] for r in rows),len(rows)),flush=True)
sys.exit(any(not r['passed'] for r in rows))
