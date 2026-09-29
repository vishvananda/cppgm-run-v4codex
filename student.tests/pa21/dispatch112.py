#!/usr/bin/env python3
"""Explicit PA21 exhaustive dispatch, raw cleanup and array composition controls."""
from pathlib import Path
import hashlib,json,subprocess,sys
ROOT=Path(__file__).resolve().parents[2]
CC,WORK=[Path(p).resolve() for p in sys.argv[1:]];WORK.mkdir(parents=True,exist_ok=True)
cases={}
guard='int trace;struct G{int n;G(int x):n(x){}~G(){trace=trace*10+n;}};'
for templated in (False,True):
 prefix='template<class T>' if templated else ''
 call='run<int>' if templated else 'run'
 for action in ('throw 9L;','fail();','return fail_value();'):
  name=('template_' if templated else '')+('throw' if action[0]=='t' else 'call' if action[0]=='f' else 'return')
  cases['exhaustive_'+name]=guard+'void fail(){throw 9L;}int fail_value(){throw 9L;}'+prefix+'int run(){try{G a(1);try{throw 7;}catch(...){G b(2);'+action+'}}catch(long n){return n!=9||trace!=21;}return 1;}int main(){return '+call+'();}'
  # Only exception objects remain live; the raw active-handler pads must end
  # each catch even when there is no lexical destructor to create a call pad.
  cases['raw_'+name]='int live,drops;struct E{E(){++live;}~E(){--live;++drops;}};void fail(){throw 9L;}int fail_value(){throw 9L;}'+prefix+'int run(){try{try{throw E();}catch(...){try{throw E();}catch(...){'+action+'}}}catch(long n){return n!=9||live||drops!=2;}return 1;}int main(){return '+call+'();}'
for caught in ('int','...'):
 cases['typed_nested_'+caught.replace('.','all')]='int live,bad;struct E{E(){++live;}~E(){--live;}};struct G{~G(){if(live!=1)bad=1;}};void fail(){throw 9L;}int main(){try{try{throw E();}catch(...){G g;try{throw 7;}catch('+caught+'){fail();}}}catch(long n){return n!=9||live||bad;}return 1;}'
for thrown,expected in (('7',12),('7L',13),('7.0',14)):
 cases['mixed_'+str(expected)]=guard+'int main(){try{G a(1);try{throw '+thrown+';}catch(int){trace=trace*10+2;}catch(long){trace=trace*10+3;}catch(...){trace=trace*10+4;}}catch(...){return 1;}return trace!='+str((expected%10)*10+1)+';}'
cases['rethrow_outer_kept']=guard+'int main(){try{G a(1);try{try{throw 7;}catch(...){G b(2);throw;}}catch(long){return 1;}}catch(int n){return n!=7||trace!=21;}return 2;}'
cases['catchall_goto']=guard+'int main(){try{G a(1);try{throw 7;}catch(...){G b(2);goto done;}}catch(...){return 1;}done:return trace!=21;}'
cases['array_local_specializations']='template<class T>void bump(T*p){*p+=sizeof(T);}template<class T>int run(){struct C{T*first;T*last;~C(){while(first!=last){bump(first);++first;}}};T a[2]={0,0};T b[2]={0,0};{C c={a,a+2};}return a==b||a[0]!=sizeof(T)||a[1]!=sizeof(T)||b[0]||b[1];}int main(){return run<int>()||run<long>();}'
cases['array_distinct_mixed']='int main(){int a[2]={0,0},b[2]={0,0};long c[2]={0,0},d[2]={0,0};a[1]=9;c[1]=1L<<35;return a==b||c==d||a[1]!=9||b[1]||c[1]!=(1L<<35)||d[1];}'
cases['array_in_handler']=guard+'int main(){try{throw 7;}catch(...){long a[2]={0,0},b[2]={0,0};G g(2);a[1]=9;if(a==b||b[1]||a[1]!=9)return 1;}return trace!=2;}'
rows=[]
for name,source in cases.items():
 src=WORK/(name+'.cpp');src.write_text(source);ir=src.with_suffix('.lowir');obj=src.with_suffix('.o');exe=WORK/name
 commands=[]
 for cmd in ([CC,'--emit-lowir','-O0','--validate-lowir','-o',ir,src],[ROOT/'dev/cppgm++-ref','-c','-O0','-o',obj,ir],['g++','-no-pie',obj,'-o',exe],[exe]):
  try:
   p=subprocess.run(list(map(str,cmd)),capture_output=True,text=True,timeout=30)
   commands.append(dict(argv=list(map(str,cmd)),exit=p.returncode,stdout=p.stdout,stderr=p.stderr))
  except subprocess.TimeoutExpired:commands.append(dict(argv=list(map(str,cmd)),exit=124,stderr='timeout'))
  if commands[-1]['exit']:break
 passed=len(commands)==4 and commands[-1]['exit']==0
 rows.append(dict(name=name,source=source,commands=commands,passed=passed))
 print(name,'PASS' if passed else 'FAIL',commands[-1]['exit'],commands[-1]['stderr'].strip(),flush=True)
 (WORK/'results.json').write_text(json.dumps(dict(compiler_sha256=hashlib.sha256(CC.read_bytes()).hexdigest(),rows=rows),indent=2)+'\n')
sys.exit(not all(r['passed'] for r in rows))
