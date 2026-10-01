#!/usr/bin/env python3
"""Function-name identity, dependent queries, rendering and serialized IR controls."""
import hashlib,json,pathlib,subprocess,sys
root=pathlib.Path(__file__).resolve().parents[2]
out=pathlib.Path(sys.argv[1]).resolve();out.mkdir(parents=True,exist_ok=True)
cc=pathlib.Path(sys.argv[2]).resolve() if len(sys.argv)>2 else root/'dev/cppgm++'
rows=[]
def run(name,args,ok=True):
 p=subprocess.run(list(map(str,args)),cwd=root,capture_output=True,timeout=90)
 passed=(p.returncode==0)==ok
 rows.append(dict(name=name,args=list(map(str,args)),status=p.returncode,passed=passed,stdout=p.stdout.decode(errors='replace'),stderr=p.stderr.decode(errors='replace')))
 if not passed: print(name,p.returncode,p.stderr.decode(),flush=True)
 return p if passed else None
for src in sorted((root/'student.tests/pa29/controls164').glob('*.cpp')):
 name=src.stem;obj=out/(name+'.o');exe=out/name
 if run(name+' compile',[cc,'-std=c++14','-O0','-c',src,'-o',obj]):
  if run(name+' link',['g++',obj,'-o',exe]):
   p=run(name+' runtime',[exe])
   if p and name=='signatures':rows.append(dict(name='special function spelling',passed=p.stdout.decode()=='C::C()\nauto inferred()\nint C::operator()() const\nC::operator int() const\nC::~C()\n'))
  if run(name+' stats',[cc,'-std=c++14','--stats','-c',src,'-o',out/'stats.o']):
   rows.append(dict(name=name+' telemetry equality',passed=obj.read_bytes()==(out/'stats.o').read_bytes()))
  low=out/(name+'.lowir');rt=out/(name+'.roundtrip')
  if run(name+' IR',[cc,'-std=c++14','-c','--emit-lowir','--validate-lowir',src,'-o',low]):
   if run(name+' roundtrip',[root/'dev/lowir',low,'-o',rt]):rows.append(dict(name=name+' IR equality',passed=low.read_bytes()==rt.read_bytes()))
   if name!='signatures':
    if run(name+' native',[root/'dev/lowir2native','--dump-machine-ir',out/(name+'.mir'),low,'-o',out/(name+'-native')]):run(name+' native runtime',[out/(name+'-native')])
reject={
 'immutable':'int main(){__PRETTY_FUNCTION__[0]=0;}',
 'wrong-bound':'int main(){const char(&s)[1]=__func__;return s[0];}',
 'dependent-wrong-bound':'template<class T>int f(){const char(&s)[1]=__PRETTY_FUNCTION__;return s[0];}int main(){return f<int>();}',
}
for name,source in reject.items():
 src=out/(name+'.cpp');src.write_text(source+'\n');run(name,[cc,'-c',src,'-o',out/'reject.o'],False)
(out/'results.json').write_text(json.dumps(dict(compiler_sha256=hashlib.sha256(cc.read_bytes()).hexdigest(),checks=rows),indent=2)+'\n')
print('%d/%d checks passed'%(sum(r['passed'] for r in rows),len(rows)),flush=True)
sys.exit(any(not r['passed'] for r in rows))
