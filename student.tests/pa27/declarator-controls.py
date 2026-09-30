#!/usr/bin/env python3
"""GNU attributes between pointer/reference operators and their declarators."""
import hashlib,json,pathlib,subprocess,sys
root=pathlib.Path(__file__).resolve().parents[2]
out=pathlib.Path(sys.argv[1]).resolve();out.mkdir(parents=True,exist_ok=True)
compiler=root/'dev/cppgm++';records=[]
cases={
'handler':'typedef int (*__attribute__((deprecated)) Handler)(int);int f(int x){return x+1;}int main(){Handler p=f;return p(6)==7?0:1;}',
'qualified':'int main(){int n=7;int* __attribute__((unused)) const __attribute__((deprecated)) p=&n;return *p==7?0:1;}',
'reference':'typedef int (&__attribute__((deprecated)) Ref);int main(){int n=7;Ref p=n;return &p==&n?0:1;}',
'nested':'typedef int (**__attribute__((deprecated)) Table)(int);int f(int n){return n+2;}int main(){int(*p)(int)=f;Table t=&p;return (*t)(5)==7?0:1;}',
'array':'typedef int (*__attribute__((deprecated)) Row)[2];int main(){int a[2]={3,4};Row p=&a;return (*p)[0]+(*p)[1]==7?0:1;}',
'member':'struct C{int x;};typedef int C::* __attribute__((deprecated)) Member;int main(){C c={7};Member p=&C::x;return c.*p==7?0:1;}',
}
for name,body in cases.items():
 source=out/(name+'.cpp');source.write_text(body+'\n');obj=out/(name+'.o');exe=out/name
 for args in ([compiler,'-c',source,'-o',obj],['g++',obj,'-o',exe],[exe]):
  p=subprocess.run(list(map(str,args)),capture_output=True,timeout=30)
  records.append(dict(command=list(map(str,args)),status=p.returncode,stdout=p.stdout.decode(),stderr=p.stderr.decode()))
  assert p.returncode==0,records[-1]
(out/'declarator-controls.json').write_text(json.dumps(dict(binary_sha256=hashlib.sha256(compiler.read_bytes()).hexdigest(),commands=records),indent=2)+'\n')
print('PA27 declarator controls PASS:',len(records),'commands')
