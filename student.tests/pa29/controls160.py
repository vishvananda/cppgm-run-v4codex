#!/usr/bin/env python3
"""Explicit invoke semantics, separate from the unchanged course suite."""
import hashlib,json,pathlib,subprocess,sys
root=pathlib.Path(__file__).resolve().parents[2]
out=pathlib.Path(sys.argv[1] if len(sys.argv)>1 else '/tmp/pa29-160/controls').resolve();out.mkdir(parents=True,exist_ok=True)
cc=root/'dev/cppgm++';rows=[]
def run(name,args,ok=True):
 p=subprocess.run(list(map(str,args)),capture_output=True,timeout=90,cwd=root)
 good=(p.returncode==0)==ok
 rows.append(dict(name=name,args=list(map(str,args)),status=p.returncode,passed=good,stdout=p.stdout.decode(errors='replace'),stderr=p.stderr.decode(errors='replace')))
 if not good:print(name,p.returncode,p.stderr.decode(errors='replace'),flush=True)
 return good
for src in sorted((root/'student.tests/pa29/controls160').glob('*.cpp')):
 obj=out/(src.stem+'.o');exe=out/src.stem
 if run(src.stem+' compile',[cc,'-c',src,'-o',obj]):
  if run(src.stem+' link',['g++',obj,'-o',exe]):run(src.stem+' run',[exe])
cases={
 'missing':'__builtin_invoke()',
 'noncallable':'__builtin_invoke(4)',
 'missing-receiver':'__builtin_invoke(&Base::x)',
 'extra-data':'__builtin_invoke(&Base::x,b,1)',
 'bad-receiver':'__builtin_invoke(&Base::x,4)',
 'private-base':'__builtin_invoke(&Base::x,Private{})',
 'ambiguous-base':'__builtin_invoke(&Base::x,Ambiguous{})',
 'deleted-dereference':'__builtin_invoke(&Base::x,Deleted{})',
 'private-dereference':'__builtin_invoke(&Base::x,Hidden{})',
 'bad-argument':'__builtin_invoke(&Base::fn,b,Unrelated{})',
 'rvalue-qualified':'__builtin_invoke(&Base::rv,b)',
 'const-receiver':'__builtin_invoke(&Base::fn,cb,1)',
 'extra-qualified':'ns::__builtin_invoke(&Base::x,b)',
}
for name,expr in cases.items():
 src=out/(name+'-reject.cpp');src.write_text('''struct Base {int x; int fn(int); int rv() &&;};
struct Private:private Base{}; struct L:Base{}; struct R:Base{}; struct Ambiguous:L,R{};
struct Deleted {Base& operator*()=delete;}; class Hidden {Base& operator*();};
struct Unrelated{}; namespace ns{}; Base b; const Base cb={0};
int main(){(void)'''+expr+';}\n')
 run(name,[cc,'-c',src,'-o',out/'reject.o'],False)
result=dict(compiler_sha256=hashlib.sha256(cc.read_bytes()).hexdigest(),checks=rows)
(out/'results.json').write_text(json.dumps(result,indent=2)+'\n')
print('%d/%d controls passed'%(sum(r['passed'] for r in rows),len(rows)),flush=True)
sys.exit(any(not r['passed'] for r in rows))
