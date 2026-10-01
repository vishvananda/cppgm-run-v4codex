#!/usr/bin/env python3
"""Explicit hosted declaration identity controls, outside course discovery."""
import hashlib,json,pathlib,subprocess,sys
root=pathlib.Path(__file__).resolve().parents[2]
out=pathlib.Path(sys.argv[1]).resolve();out.mkdir(parents=True,exist_ok=True)
cc=pathlib.Path(sys.argv[2]).resolve() if len(sys.argv)>2 else root/'dev/cppgm++'
def run(args):
 p=subprocess.run(list(map(str,args)),cwd=root,capture_output=True,text=True,timeout=90)
 return dict(command=list(map(str,args)),status=p.returncode,stdout=p.stdout,stderr=p.stderr)
rows=[]
for source in sorted((root/'student.tests/pa29/source176').glob('*.cpp')):
 row=dict(source=str(source.relative_to(root)),sha256=hashlib.sha256(source.read_bytes()).hexdigest())
 obj=out/(source.stem+'.o');exe=out/source.stem
 row['compile']=run([cc,'-std=c++11','-O0','-c',source,'-o',obj])
 reject='.reject.' in source.name
 if reject: row['passed']=row['compile']['status']!=0
 elif not row['compile']['status']:
  row['symbols']=run(['nm','-C',obj])
  objects=[obj]
  peer=source.with_suffix('.peer')
  if peer.exists():
   peerobj=out/(source.stem+'-peer.o')
   row['peer_compile']=run([cc,'-std=c++11','-O0','-c',peer,'-o',peerobj]);objects.append(peerobj)
  row['link']=run(['g++',*objects,'-o',exe])
  if not row['link']['status']:row['run']=run([exe])
  row['passed']=all(v['status']==0 for v in row.values() if isinstance(v,dict))
 else:row['passed']=False
 rows.append(row);print(source.name,row['passed'],row['compile']['stderr'],flush=True)
(out/'controls.json').write_text(json.dumps(dict(compiler_sha256=hashlib.sha256(cc.read_bytes()).hexdigest(),rows=rows),indent=2)+'\n')
sys.exit(not all(r['passed'] for r in rows))
