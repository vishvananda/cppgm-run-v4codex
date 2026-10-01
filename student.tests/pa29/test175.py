#!/usr/bin/env python3
"""Explicit source invocation controls; no course discovery or oracle changes."""
import hashlib,json,pathlib,subprocess,sys
root=pathlib.Path(__file__).resolve().parents[2]
out=pathlib.Path(sys.argv[1]).resolve();out.mkdir(parents=True,exist_ok=True)
cc=pathlib.Path(sys.argv[2]).resolve() if len(sys.argv)>2 else root/'dev/cppgm++'
rows=[]
def run(args):
 p=subprocess.run(list(map(str,args)),cwd=root,capture_output=True,text=True,timeout=90)
 return dict(command=list(map(str,args)),status=p.returncode,stdout=p.stdout,stderr=p.stderr)
for source in sorted((root/'student.tests/pa29/source175').glob('*.cpp')):
 row=dict(source=str(source.relative_to(root)),sha256=hashlib.sha256(source.read_bytes()).hexdigest())
 obj=out/(source.stem+'.o'); exe=out/source.stem
 row['compile']=run([cc,'-std=c++11','-O0','-c',source,'-o',obj])
 reject='.reject.' in source.name
 if not reject and row['compile']['status']==0:
  row['link']=run(['g++',obj,'-o',exe])
  if row['link']['status']==0:row['run']=run([exe])
  if source.name!='columns.cpp':
   row['host_compile']=run(['g++','-std=c++11','-O0',source,'-o',str(exe)+'-host'])
   if row['host_compile']['status']==0:row['host_run']=run([str(exe)+'-host'])
  row['passed']=all(v['status']==0 for v in row.values() if isinstance(v,dict)) and ('host_run' not in row or row['host_run']['stdout']==row['run']['stdout'])
 else:row['passed']=reject and row['compile']['status']!=0
 rows.append(row);print(source.name,row['passed'],row.get('run',row['compile'])['stdout'] or row['compile']['stderr'],flush=True)
(out/'controls.json').write_text(json.dumps(dict(compiler_sha256=hashlib.sha256(cc.read_bytes()).hexdigest(),rows=rows),indent=2)+'\n')
sys.exit(0 if all(r['passed'] for r in rows) else 1)
