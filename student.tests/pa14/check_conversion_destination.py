#!/usr/bin/env python3
"""Destination storage belongs to the initialized object; call arguments retain temporaries.
N3485 [dcl.init]/16-17, [class.temporary]/3-5, [class.copy]/31-32.
"""
from pathlib import Path
import hashlib,json,subprocess,sys
ROOT=Path(__file__).resolve().parents[2]
BINARY=Path(sys.argv[1]).resolve();WORK=Path(sys.argv[2]).resolve();WORK.mkdir(parents=True,exist_ok=True)
objects=Path(sys.argv[3]).resolve() if len(sys.argv)>3 else ROOT/'obj/dev'
san=len(sys.argv)>4 and sys.argv[4]=='sanitized'
def sha(p):return hashlib.sha256(Path(p).read_bytes()).hexdigest()
def run(cmd):
 p=subprocess.run(list(map(str,cmd)),capture_output=True,text=True,cwd=ROOT,timeout=180)
 assert p.returncode==0,(cmd,p.returncode,p.stdout,p.stderr)
 assert 'Sanitizer' not in p.stderr and 'runtime error:' not in p.stderr
 return p.stdout
cases={
'ownership':'struct C{int n;C(int x):n(x){}};void take(C){}template<class T>int f(int n){C a=n;C b=a;take(n);return b.n;}int main(){return f<int>(3)!=3||f<long>(4)!=4;}',
'default-cleanup':'int live,seen;struct G{G(){++live;}~G(){--live;}};struct C{int n;C(int x,const G& g=G()):n(x){seen+=live;}};template<class T>int f(int x){C c=x;struct A{C c;};A a{x+1};return c.n+a.c.n+live;}int main(){return f<int>(2)!=5||f<long>(3)!=7||live!=0||seen!=4;}',
'conversion-result':'int live;struct C{int n;C(int x):n(x){++live;}C(const C& c):n(c.n){++live;}~C(){--live;}};struct F{operator C(){return C(7);}};template<class T>int f(){F f;C a=f;return a.n+live;}int main(){return f<int>()!=8||f<long>()!=8||live!=0;}',
'borrowed-result':'int live;struct C{int n;C(int x):n(x){++live;}C(const C& c):n(c.n){++live;}~C(){--live;}};struct F{C c;F():c(9){}operator C&(){return c;}};template<class T>int f(){F f;C a=f;return a.n+live;}int main(){return f<int>()!=11||f<long>()!=11||live!=0;}',
'derived-result':'int live;struct C{int n;C(int x):n(x){++live;}C(const C& c):n(c.n){++live;}~C(){--live;}};struct D:C{D():C(9){}};struct F{operator D(){return D();}};template<class T>int f(){F f;C a=f;return a.n+live;}int main(){return f<int>()!=10||f<long>()!=10||live!=0;}',
'polymorphic':'int live;struct C{int n;C(int x):n(x){++live;}virtual ~C(){--live;}};template<class T>int f(){C a=3;struct A{C c;};A b{4};return a.n+b.c.n+live;}int main(){return f<int>()!=9||f<long>()!=9||live!=0;}',
}
rows=[]
for name,source in cases.items():
 path=WORK/(name+'.cpp');path.write_text(source);ir=WORK/(name+'.lowir');exe=WORK/name
 run([BINARY,'--emit-lowir','-O0','--validate-lowir','-o',ir,path]);run([ROOT/'dev/lowir2native-ref','-O0','-o',exe,ir]);run([exe])
 rows.append(dict(name=name,source_path=str(path),source_sha256=sha(path),native=dict(path=str(exe),sha256=sha(exe),exit_code=0)));print(name,'PASS',flush=True)
names=run(['make','-s','-C','dev','--eval=probe-objects:\n\t@echo $(FRONTEND_OBJ_BASENAMES_cppgm++)','probe-objects']).split()
flags=['-fsanitize=address,undefined','-fno-pie','-no-pie'] if san else []
probe=WORK/'probe';src=ROOT/'student.tests/pa14/conversion-destination.cc'
run(['g++','-std=c++11','-O2',*flags,'-I'+str(ROOT/'dev/src'),src,*[objects/(n+'.o') for n in names],'-o',probe])
output=run([probe,WORK/'ownership.cpp'])+run([probe,WORK/'borrowed-result.cpp','user']);log=WORK/'probe.log';log.write_text(output);print(output,flush=True)
(WORK/'checks.json').write_text(json.dumps(dict(harness_sha256=sha(__file__),probe_source_sha256=sha(src),binary=dict(path=str(BINARY),sha256=sha(BINARY)),checks=rows,probe_log=str(log),probe_log_sha256=sha(log)),indent=2)+'\n')
