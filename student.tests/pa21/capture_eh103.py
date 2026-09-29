#!/usr/bin/env python3
"""External throwing callees exercise student capture cleanup via host unwinding.

Only the harness is host-compiled. The supplied backend consumes student LowIR.
"""
from pathlib import Path
import hashlib,json,subprocess,sys
ROOT=Path(__file__).resolve().parents[2]
CC,WORK=[Path(x).resolve() for x in sys.argv[1:]]
WORK.mkdir(parents=True,exist_ok=True)
common='extern "C" void copied(int);extern "C" void destroyed(int);extern "C" int next_id();'
obj='struct S{int id;S(int n=next_id()):id(n){}S(const S&s):id(s.id+100){copied(id);}~S(){destroyed(id);}};'
cases={
 'second_capture':(common+obj+'extern "C" void run(){S a(1),b(2);auto f=[a,b](){};}',102,[101,2,1]),
 'first_capture':(common+obj+'extern "C" void run(){S a(1),b(2);auto f=[a,b](){};}',101,[2,1]),
 'complete':(common+obj+'extern "C" void run(){S a(1),b(2);auto f=[a,b](){};}',0,[102,101,2,1]),
 'array_small':(common+obj+'extern "C" void run(){S a[3];auto f=[a](){};}',103,[102,101,3,2,1]),
 'array_large':(common+obj+'extern "C" void run(){S a[12];auto f=[a](){};}',106,list(range(105,100,-1))+list(range(12,0,-1))),
 'array_then_scalar':(common+obj+'extern "C" void run(){S a[3];S b(9);auto f=[a,b](){};}',109,[103,102,101,9,3,2,1]),
 'nested_capture':(common+obj+'extern "C" void run(){S a(1);auto f=[a](){auto g=[a](){};};f();}',201,[101,1]),
}
cases.update({
 'closure_copy_failure':(common+obj+'extern "C" void run(){S a(1),b(2);auto f=[a,b](){};auto g=f;}',202,[201,102,101,2,1]),
 'closure_array_copy_failure':(common+obj+'extern "C" void run(){S a[3];auto f=[a](){};auto g=f;}',203,[202,201,103,102,101,3,2,1]),
})
cases.update({
 'closure_array_large_failure':(common+obj+'extern "C" void run(){S a[12];auto f=[a](){};auto g=f;}',206,list(range(205,200,-1))+list(range(112,100,-1))+list(range(12,0,-1))),
 'empty_copy_prefix':(common+obj+'struct E{~E(){destroyed(50);}};extern "C" void run(){E e;S s(1);auto f=[e,s](){};auto g=f;}',201,[50,101,50,1,50]),
})
g='struct G{G(){}~G(){destroyed(50);}};'
s='struct S{int id;S(int n=next_id()):id(n){}S(const S&s,const G&=G()):id(s.id+100){copied(id);}~S(){destroyed(id);}};'
cases.update({
 'default_temp':(common+g+s+'extern "C" void run(){S a(1),b(2);auto f=[a,b](){};}',102,[50,101,50,2,1]),
 'array_defaults':(common+g+s+'extern "C" void run(){S a[3];auto f=[a](){};}',103,[50,50,50,102,101,3,2,1]),
})
def run(cmd):return subprocess.run([str(x) for x in cmd],capture_output=True,text=True,timeout=60)
def sha(p):return hashlib.sha256(Path(p).read_bytes()).hexdigest()
rows=[]
for name,(source,fail,expected) in cases.items():
 src=WORK/(name+'.cpp');src.write_text(source.replace('const G&=','const G& ='))
 ir=src.with_suffix('.lowir');objfile=src.with_suffix('.o');driver=WORK/(name+'-host.cpp');exe=WORK/name
 driver.write_text('extern "C" void run();int seq[100],used,serial;extern "C" int next_id(){return ++serial;}extern "C" void copied(int n){if(n=='+str(fail)+')throw n;}extern "C" void destroyed(int n){seq[used++]=n;}int main(){bool caught=false;try{run();}catch(int n){caught=n=='+str(fail)+';}int expected[]={'+','.join(map(str,expected))+'};if(caught!='+str(bool(fail)).lower()+'||used!='+str(len(expected))+')return 1;for(int i=0;i<used;++i)if(seq[i]!=expected[i])return 2;return 0;}')
 row=dict(name=name,source=src.read_text(),harness=driver.read_text(),expected_destructions=expected,commands=[])
 commands=[[CC,'--emit-lowir','-O0','--validate-lowir','-o',ir,src],[ROOT/'dev/cppgm++-ref','-c','-O0','-o',objfile,ir],['g++','-no-pie',driver,objfile,'-o',exe],[exe]]
 for cmd in commands:
  p=run(cmd);row['commands'].append(dict(argv=list(map(str,cmd)),exit=p.returncode,stdout=p.stdout,stderr=p.stderr))
  if p.returncode:break
 row['passed']=len(row['commands'])==4 and p.returncode==0
 if ir.exists():row['lowir_sha256']=sha(ir)
 rows.append(row);print(name,'PASS' if row['passed'] else 'FAIL',p.stderr.strip(),flush=True)
(WORK/'results.json').write_text(json.dumps(dict(compiler_sha256=sha(CC),object_backend_sha256=sha(ROOT/'reference-binaries/cppgm++'),rows=rows),indent=2)+'\n')
sys.exit(0 if all(r['passed'] for r in rows) else 1)
