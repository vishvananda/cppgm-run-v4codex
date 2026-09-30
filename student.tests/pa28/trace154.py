#!/usr/bin/env python3
"""Trace an attributed virtual declaration and demanded class/function templates.
Pass an audit150-view binary built by inspection153.py from current objects.
"""
import hashlib,json,pathlib,subprocess,sys
root=pathlib.Path(__file__).resolve().parents[2]
out=pathlib.Path(sys.argv[1]).resolve();out.mkdir(parents=True,exist_ok=True)
view=pathlib.Path(sys.argv[2]).resolve()
source=root/'student.tests/pa28/trace154.cpp';rows=[]
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def run(args):
 p=subprocess.run(list(map(str,args)),cwd=root,capture_output=True,text=True,timeout=60)
 rows.append(dict(command=list(map(str,args)),status=p.returncode,stdout=p.stdout,stderr=p.stderr))
 assert p.returncode==0,rows[-1]
 return p.stdout
stats=json.loads(run([view,source,out/'trace']))
run(['dev/cppgm++','-c',source,'-o',out/'ordinary.o'])
assert sha(out/'trace.o')==sha(out/'ordinary.o')
run(['g++',out/'trace.o','-o',out/'trace']);run([out/'trace'])
names=run(['nm',out/'trace.o']);ir=(out/'trace.lowir').read_text();mir=(out/'trace.mir').read_text()
assert 'B5audit' in names and '_Z9sample154I7Root154B5auditEiPKT_' in names
assert 'unused' not in names
assert 'effects=readonly' in ir and '__dynamic_cast' in ir and 'adjustor_thunk' in ir
assert 'function ' in mir
run(['readelf','-rW',out/'trace.o']);run(['readelf','-wf',out/'trace.o'])
(out/'trace.json').write_text(json.dumps(dict(status='pass',source_sha256=sha(source),binary_sha256=sha(root/'dev/cppgm++'),statistics=stats,
 hashes={name:sha(out/name) for name in ['trace.o','ordinary.o','trace.lowir','trace.mir']},checks=rows),indent=2)+'\n')
print('typed template trace PASS:',stats)
