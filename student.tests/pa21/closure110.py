#!/usr/bin/env python3
"""PA21 complete handler/static/list ownership execution controls."""
from pathlib import Path
import hashlib,json,subprocess,sys
ROOT=Path(__file__).resolve().parents[2]
CC,WORK=[Path(p).resolve() for p in sys.argv[1:]]
WORK.mkdir(parents=True,exist_ok=True)
from list104 import LIB
cases={}
cases['handler_temporary_precedes_exception']='int live,bad;struct E{E(){++live;}~E(){--live;}};struct G{G(int){}~G(){if(live!=1)bad=1;}};int use(const G&){throw 7;}int f(){try{throw E();}catch(...){return use(G(1));}}int main(){try{f();}catch(int n){return n!=7||live||bad;}return 1;}'
cases['handler_constructor_failure']='int live,drops;struct E{E(){++live;}~E(){--live;}};struct G{G(int){throw 7;}~G(){++drops;}};int use(const G&){return 0;}int f(){try{throw E();}catch(...){return use(G(1));}}int main(){try{f();}catch(int n){return n!=7||live||drops;}return 1;}'
cases['handler_nested_mismatch_cleanup']='int live,bad;struct E{E(){++live;}~E(){--live;}};struct G{~G(){if(live!=1)bad=1;}};void f(){try{throw E();}catch(...){G g;try{throw 7L;}catch(int){}}}int main(){try{f();}catch(long n){return n!=7||live||bad;}return 1;}'
cases['handler_throwing_cleanup_order']='int live,trace;struct E{E(){++live;}~E(){--live;trace=trace*10+3;}};struct G{int n;G(int x):n(x){}~G()noexcept(false){trace=trace*10+n;if(n==2)throw 7;}};int f(){try{throw E();}catch(...){G a(1),b(2);return 0;}}int main(){try{f();}catch(int n){return n!=7||live||trace!=213;}return 1;}'

for count in (1,3,8,9,16):
 for fail in (1,count):
  values=','.join(str(i) for i in range(1,count+1))
  source=LIB+'int live,drops,bad,next='+str(count)+';struct M{int n;M(int x):n(x){++live;}~M()noexcept(false){--live;++drops;if(n!=next--)bad=1;if(n=='+str(fail)+')throw 7;}};'
  source+='int main(){try{std::initializer_list<M> a{'+values+'};}catch(int n){return n!=7||live||bad||next||drops!='+str(count)+';}return 1;}'
  cases['list_destructor_%d_%d'%(count,fail)]=source
for fail in (1,2,3):
 source='int fail='+str(fail)+';int live,temps,attempts;struct G{int n;G(int x):n(x){if(fail==n)throw n;++temps;}~G(){--temps;}};struct S{S(const G&,const G&){++attempts;if(fail==3)throw 3;++live;}};int f(){static S s(G(1),G(2));return live;}'
 source+='int main(){try{f();}catch(int n){if(n!=fail||live||temps)return 1;}fail=0;if(f()!=1||temps||f()!=1||temps||live!=1)return 2;return attempts!='+str(2 if fail==3 else 1)+';}'
 cases['static_retry_'+str(fail)]=source

for name,source in list(cases.items()):
 if name.startswith('list_destructor_'):
  cases[name.replace('list_', 'array_')]=source.replace('std::initializer_list<M> a{','M a[]{')
cases['static_temporary_destructor_retry']='int live;bool fail=true;struct G{~G()noexcept(false){if(fail)throw 7;}};struct S{S(const G&){++live;}~S(){--live;}};int f(){static S s{G()};return live;}int main(){try{f();}catch(int n){if(n!=7||live!=1)return 1;}fail=false;return f()!=1||f()!=1;}'
cases['static_list_temporary_destructor_retry']=LIB+'int live;bool fail=true;struct G{~G()noexcept(false){if(fail)throw 7;}};struct S{S(const G&){++live;}~S(){--live;}};int f(){static std::initializer_list<S> s{G()};return live;}int main(){try{f();}catch(int n){if(n!=7||live!=1)return 1;}fail=false;return f()!=1||f()!=1;}'
rows=[]
def run(cmd):
 try:
  p=subprocess.run(list(map(str,cmd)),capture_output=True,text=True,timeout=30)
  return dict(argv=list(map(str,cmd)),exit=p.returncode,stdout=p.stdout,stderr=p.stderr)
 except subprocess.TimeoutExpired:return dict(argv=list(map(str,cmd)),exit=124,stderr='timeout')
for name,source in cases.items():
 src=WORK/(name+'.cpp');src.write_text(source);ir=src.with_suffix('.lowir');obj=src.with_suffix('.o');exe=WORK/name
 commands=[]
 for cmd in ([CC,'--emit-lowir','-O0','--validate-lowir','-o',ir,src],[ROOT/'dev/cppgm++-ref','-c','-O0','-o',obj,ir],['g++','-no-pie',obj,'-o',exe],[exe]):
  commands.append(run(cmd))
  if commands[-1]['exit']:break
 passed=len(commands)==4 and commands[-1]['exit']==0
 rows.append(dict(name=name,source=source,commands=commands,passed=passed))
 print(name,'PASS' if passed else 'FAIL',commands[-1]['exit'],commands[-1]['stderr'].strip(),flush=True)
 (WORK/'results.json').write_text(json.dumps(dict(compiler_sha256=hashlib.sha256(CC.read_bytes()).hexdigest(),rows=rows),indent=2)+'\n')
sys.exit(0 if all(r['passed'] for r in rows) else 1)
