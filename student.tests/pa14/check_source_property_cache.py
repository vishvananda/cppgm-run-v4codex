#!/usr/bin/env python3
"""Declaration properties precede lifetime actions and remain stable after use."""
from pathlib import Path
import hashlib,json,subprocess,sys
ROOT=Path(__file__).resolve().parents[2]
WORK,OBJECTS=map(lambda x:Path(x).resolve(),sys.argv[1:3]);WORK.mkdir(parents=True,exist_ok=True)
san=len(sys.argv)>3 and sys.argv[3]=='sanitized'
def sha(p):return hashlib.sha256(Path(p).read_bytes()).hexdigest()
def run(cmd):
 p=subprocess.run(list(map(str,cmd)),cwd=ROOT,capture_output=True,text=True,timeout=180)
 assert p.returncode==0,(cmd,p.returncode,p.stdout,p.stderr)
 return p
names=run(['make','-s','-C','dev','--eval=probe-objects:\n\t@echo $(FRONTEND_OBJ_BASENAMES_cppgm++)','probe-objects']).stdout.split()
flags=['-fsanitize=address,undefined','-fno-pie','-no-pie'] if san else []
probe=WORK/'probe';source=ROOT/'student.tests/pa14/source-obligations.cc'
run(['g++','-std=c++11','-O2',*flags,'-I'+str(ROOT/'dev/src'),source,*[OBJECTS/(n+'.o') for n in names],'-o',probe])
fixture=WORK/'properties.cpp';fixture.write_text('struct Leaf{int n=3;};template<class T>int f(){Leaf value;return value.n;}int main(){return f<int>()+f<long>()-6;}\n')
p=run([probe,fixture]);log=WORK/'probe.log';log.write_text(p.stdout+p.stderr)
assert '10000 source property queries without body/actions demand; 10000 stable concrete finish queries' in p.stdout
result=dict(harness_sha256=sha(__file__),probe_source_sha256=sha(source),binary=dict(path=str(probe),sha256=sha(probe)),source=dict(path=str(fixture),sha256=sha(fixture)),log=str(log),log_sha256=sha(log),exit_code=p.returncode)
(WORK/'checks.json').write_text(json.dumps(result,indent=2)+'\n');print(p.stdout,end='')
