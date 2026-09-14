#!/usr/bin/env python3
"""N3485 [expr.cond]/1,3-6: conversions occur on the selected branch only."""
from pathlib import Path
import hashlib,json,subprocess,sys
ROOT=Path(__file__).resolve().parents[2]
BINARY,WORK=map(lambda x:Path(x).resolve(),sys.argv[1:3]);WORK.mkdir(parents=True,exist_ok=True)
def sha(p):return hashlib.sha256(Path(p).read_bytes()).hexdigest()
cases={
 'scalar': 'int calls;struct C{operator int(){++calls;return 3;}};template<class T>int f(bool pick){C c;return pick?c:5;}int main(){return f<int>(true)!=3||calls!=1||f<long>(false)!=5||calls!=1;}',
 'reference': 'int n=3,calls;struct C{operator int&(){++calls;return n;}};template<class T>int f(bool pick){C c;int other=10;int& r=pick?c:other;++r;return r;}int main(){return f<int>(true)!=4||calls!=1||f<long>(false)!=11||calls!=1||n!=4;}',
 'distinct': 'int left,right;struct C{operator int(){++left;return 3;}};struct D{operator double(){++right;return 4;}};template<class T>int f(bool pick){C c;D d;return int(pick?c:d);}int main(){return f<int>(true)!=3||left!=1||right||f<long>(false)!=4||left!=1||right!=1;}',
 'class': 'int live;struct C{int n;C(int x):n(x){++live;}C(const C& x):n(x.n){++live;}~C(){--live;}};template<class T>int f(bool pick){C c=pick?C(3):4;return c.n;}int main(){return f<int>(true)!=3||live||f<long>(false)!=4||live;}',
}
rows=[]
for name,source in cases.items():
 path=WORK/(name+'.cpp');path.write_text(source);ir=WORK/(name+'.lowir');exe=WORK/name
 commands=[[BINARY,'--emit-lowir','-O0','--validate-lowir','-o',ir,path],[ROOT/'dev/lowir2native-ref','-O0','-o',exe,ir],[exe]]
 runs=[]
 for i,command in enumerate(commands):
  p=subprocess.run(list(map(str,command)),cwd=ROOT,capture_output=True,text=True,timeout=60)
  log=WORK/(name+'-'+str(i)+'.log');log.write_text(p.stdout+p.stderr)
  runs.append(dict(command=list(map(str,command)),exit_code=p.returncode,log=str(log),log_sha256=sha(log)))
  assert p.returncode==0,(name,i,p.stderr)
  assert 'Sanitizer' not in p.stderr and 'runtime error:' not in p.stderr,(name,i,p.stderr)
 rows.append(dict(name=name,source_path=str(path),source_sha256=sha(path),runs=runs,native=dict(path=str(exe),sha256=sha(exe))))
 print(name,'PASS',flush=True)
(WORK/'checks.json').write_text(json.dumps(dict(binary=dict(path=str(BINARY),sha256=sha(BINARY)),harness_sha256=sha(__file__),checks=rows),indent=2)+'\n')
