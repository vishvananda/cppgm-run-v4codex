#!/usr/bin/env python3
"""ABI-compatible vector bounds assertions on declaration lifecycle consumers."""
import hashlib,json,pathlib,shlex,subprocess,sys
root=pathlib.Path(__file__).resolve().parents[2]
out=pathlib.Path(sys.argv[1]).resolve();out.mkdir(parents=True,exist_ok=True)
rows=[]
def run(args,cwd=root):
    p=subprocess.run(list(map(str,args)),cwd=cwd,capture_output=True,text=True,timeout=90)
    rows.append(dict(command=list(map(str,args)),status=p.returncode,stdout=p.stdout,stderr=p.stderr))
    (out/'bounds.json').write_text(json.dumps(rows,indent=2)+'\n')
    assert not p.returncode,rows[-1]
    return p.stdout
dry=run(['make','-n','-C','dev','V=1','-W','src/lowering/destruction.cpp','cppgm++'])
link=next(shlex.split(l) for l in dry.splitlines() if l.startswith('g++ ') and 'cppgm++.tmp' in l)
for unit in ['destruction','symbols']:
    obj=out/(unit+'.o')
    run(['g++','-std=gnu++11','-O2','-D_GLIBCXX_ASSERTIONS','-Idev/src','-c','dev/src/lowering/'+unit+'.cpp','-o',obj])
    link=[str(obj) if x=='../obj/dev/lowering/'+unit+'.o' else x for x in link]
link[link.index('-o')+1]=str(out/'compiler')
run(link,root/'dev')
for name in ['source-storage','inline-context','inline-bound-effects','selection-source']:
    run([out/'compiler','-c',root/('student.tests/pa29/source178/'+name+'.cpp'),'-o',out/(name+'.o')])
    run(['g++',out/(name+'.o'),'-o',out/name]);run([out/name])
print('bounds assertions passed',len(rows),flush=True)
