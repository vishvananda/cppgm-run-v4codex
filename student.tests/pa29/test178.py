#!/usr/bin/env python3
"""Explicit audit controls; optional observation of the frozen pre-fix compiler."""
import hashlib,json,pathlib,subprocess,sys
root=pathlib.Path(__file__).resolve().parents[2]
out=pathlib.Path(sys.argv[1]).resolve();out.mkdir(parents=True,exist_ok=True)
cc=pathlib.Path(sys.argv[2]).resolve() if len(sys.argv)>2 else root/'dev/cppgm++'
observe='--observe' in sys.argv
def run(args):
    p=subprocess.run(list(map(str,args)),cwd=root,capture_output=True,text=True,timeout=90)
    return dict(command=list(map(str,args)),status=p.returncode,stdout=p.stdout,stderr=p.stderr)
rows=[]
for src in sorted((root/'student.tests/pa29/source178').glob('*.cpp')):
    row=dict(source=str(src.relative_to(root)),sha256=hashlib.sha256(src.read_bytes()).hexdigest())
    reject='.reject.' in src.name
    host='clang++' if src.stem=='integrated' else 'g++'
    for label,binary,mode in [('student',cc,'c++11'),('host',host,'c++17')]:
        obj=out/(src.stem+label+'.o');exe=out/(src.stem+label)
        checks=[run([binary,'-std='+mode,'-O0','-c',src,'-o',obj])]
        if not reject and not checks[-1]['status']:
            checks.append(run(['g++',obj,'-o',exe]))
            if not checks[-1]['status']:checks.append(run([exe]))
        row[label]=checks
        row[label+'_passed']=checks[0]['status']==1 if reject else len(checks)==3 and all(c['status']==0 for c in checks)
    rows.append(row);print(src.name,row['student_passed'],row['host_passed'],flush=True)
    (out/'controls.json').write_text(json.dumps(dict(compiler_sha256=hashlib.sha256(cc.read_bytes()).hexdigest(),rows=rows),indent=2)+'\n')
assert all(r['host_passed'] for r in rows)
if not observe:assert all(r['student_passed'] for r in rows)
