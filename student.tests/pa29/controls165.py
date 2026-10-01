#!/usr/bin/env python3
"""Explicit PA29 invocation/evaluation-context behavioral controls."""
import hashlib,json,pathlib,subprocess,sys
root=pathlib.Path(__file__).resolve().parents[2]
out=pathlib.Path(sys.argv[1]).resolve();out.mkdir(parents=True,exist_ok=True)
cc=pathlib.Path(sys.argv[2]).resolve() if len(sys.argv)>2 else root/'dev/cppgm++'
rows=[]
def run(name,args,ok=True):
 p=subprocess.run(list(map(str,args)),cwd=root,capture_output=True,timeout=90)
 passed=(p.returncode==0)==ok
 rows.append(dict(name=name,args=list(map(str,args)),status=p.returncode,passed=passed,stdout=p.stdout.decode(errors='replace'),stderr=p.stderr.decode(errors='replace')))
 if not passed:print(name,p.returncode,p.stderr.decode(),flush=True)
 return passed
for src in sorted((root/'student.tests/pa29/controls165').glob('*.cpp')):
 name=src.stem;obj=out/(name+'.o');exe=out/name
 if run(name+' compile',[cc,'-std=c++14','-O0','-c',src,'-o',obj]):
  if run(name+' link',['g++',obj,'-o',exe]):run(name+' runtime',[exe])
reject={'required-false':'([](){static_assert(!__builtin_is_constant_evaluated(),"");return 0;})()', 'address':'bool(&__builtin_is_constant_evaluated)', 'decay':'bool(__builtin_is_constant_evaluated)', 'arity':'__builtin_is_constant_evaluated(1)','qualification':'nested::__builtin_is_constant_evaluated()'}
for name,expr in reject.items():
 src=out/(name+'.cpp');src.write_text('namespace nested {}\nint main(){return '+expr+';}\n')
 run(name,[cc,'-c',src,'-o',out/'reject.o'],False)
(out/'results.json').write_text(json.dumps(dict(compiler_sha256=hashlib.sha256(cc.read_bytes()).hexdigest(),checks=rows),indent=2)+'\n')
print('%d/%d checks passed'%(sum(r['passed'] for r in rows),len(rows)),flush=True)
sys.exit(any(not r['passed'] for r in rows))
