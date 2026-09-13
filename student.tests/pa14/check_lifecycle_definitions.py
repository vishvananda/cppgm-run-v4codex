#!/usr/bin/env python3
"""N3485 [temp.inst]/1, [class.dtor]/8: declaration use versus definition work.
Two translation units keep Impl incomplete at the delete-expression. Definitions
must emit their subobject destruction even without a use in the defining TU.
"""
from pathlib import Path
import hashlib,json,subprocess,sys
ROOT=Path(__file__).resolve().parents[2]
B,WORK=map(lambda p:Path(p).resolve(),sys.argv[1:3]);WORK.mkdir(parents=True,exist_ok=True)
observe=len(sys.argv)>3 and sys.argv[3]=='observe'
header='''extern int count;
template<class T>struct Holder{T* ptr;~Holder(){count+=sizeof(T);}};
struct Owner{struct Impl;Holder<Impl> member;Owner();~Owner();};
void destroy(Owner*);
'''
CASES={
 'external':([header+'void destroy(Owner*p){delete p;}'],False),
 'owner':([header+'void destroy(Owner*p){delete p;}',header+'''int count;
struct Owner::Impl{int value;};Owner::Owner():member(){}Owner::~Owner(){}
int main(){Owner*p=new Owner;destroy(p);return count!=4;}
'''],True),
 'definition':(['''extern int count;struct Member{~Member(){++count;}};
struct Target{Member field;~Target();};Target::~Target(){}
''','''int count;struct Member{~Member(){++count;}};
struct Target{Member field;~Target();};int main(){{Target value;}return count!=1;}
'''],True),
 'defaulted-definition':(['''extern int count;struct Member{~Member(){++count;}};
struct Target{Member field;~Target();};Target::~Target()=default;
''','''int count;struct Member{~Member(){++count;}};
struct Target{Member field;~Target();};int main(){{Target value;}return count!=1;}
'''],True),
 'virtual-definition':(['''struct Base{virtual int f();Base();};Base::Base(){}
''','''struct Base{virtual int f();Base();};int Base::f(){return 7;}
int read(Base*p){return p->f();}int main(){Base b;return read(&b)!=7;}
'''],True),
}
rows=[]
def sha(p):return hashlib.sha256(Path(p).read_bytes()).hexdigest()
def run(cmd,name):
 p=subprocess.run(list(map(str,cmd)),cwd=ROOT,capture_output=True,text=True,timeout=180)
 log=WORK/(name+'.log');log.write_text(p.stdout+p.stderr)
 row=dict(command=list(map(str,cmd)),exit_code=p.returncode,log=str(log),log_sha256=sha(log));rows.append(row)
 (WORK/'checks.json').write_text(json.dumps(dict(binary=str(B),binary_sha256=sha(B),checks=rows),indent=2)+'\n')
 return p.returncode
for name,(texts,native) in CASES.items():
 paths=[]
 for i,text in enumerate(texts):
  p=WORK/f'{name}-{i}.cpp';p.write_text(text);paths.append(p)
 host=WORK/(name+'-host')
 # Syntax-only suffices for the external-only control; native controls also
 # verify the reducer's result independently of this compiler and its backend.
 hostcmd=['g++','-std=c++11',*paths]+(['-o',host] if native else ['-fsyntax-only'])
 assert run(hostcmd,name+'-host-build')==0
 if native:assert run([host],name+'-host-run')==0
 ir=WORK/(name+'.lowir');status=run([B,'--emit-lowir','-O0','--validate-lowir','-o',ir,*paths],name+'-compile')
 if not observe:assert status==0,(name,status)
 if not status and native:
  exe=WORK/(name+'-native');status=run([ROOT/'dev/lowir2native-ref','-O0','-o',exe,ir],name+'-backend')
  if not status:status=run([exe],name+'-run')
  if not observe:assert status==0,(name,status)
 print(name,status,flush=True)
