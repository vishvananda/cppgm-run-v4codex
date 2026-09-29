#!/usr/bin/env python3
"""Explicit constant-conversion receiver/lifetime and demand controls."""
from pathlib import Path
import hashlib,json,subprocess,sys
ROOT=Path(__file__).resolve().parents[2]
CC,WORK=[Path(x).resolve() for x in sys.argv[1:3]]
WORK.mkdir(parents=True,exist_ok=True)
flag='''template<bool B>struct Flag{static const bool value=B;
constexpr operator bool()const{return B;}};
template<bool B>const bool Flag<B>::value;
'''
cases={}
def add(name,source,omitted=None): cases[name]=(source,omitted)
add('temporary-false',flag+'int main(){return Flag<false>()?1:0;}',1)
add('temporary-true',flag+'int main(){return Flag<true>()?0:1;}',1)
add('temporary-parentheses',flag+'int main(){return (((Flag<false>())))?1:0;}',1)
add('temporary-braces',flag+'int main(){return Flag<false>{}?1:0;}',0)
add('temporary-depth-fallback',flag+'int main(){return '+'('*12+'Flag<false>()'+')'*12+'?1:0;}',0)
add('inherited-empty',flag+'template<class T>struct F:Flag<false>{};int main(){return F<int>()?1:0;}',1)
add('nondependent', 'const bool no=false;struct F{operator bool()const{return no;}};int main(){return F()?1:0;}',1)
add('multiple-empty-bases',flag+'struct E{};struct F:E,Flag<false>{};int main(){return F()?1:0;}',1)
add('named-local',flag+'int main(){Flag<false> f;return f?1:0;}',0)
add('function-receiver',flag+'int count;Flag<false> make(){++count;return Flag<false>();}int main(){int x=make()?1:0;return x||count!=1;}',0)
add('comma-receiver',flag+'int count;int main(){int x=(++count,Flag<false>())?1:0;return x||count!=1;}',0)
add('conditional-receiver',flag+'volatile int condition=1;int main(){return (condition?Flag<false>():Flag<false>())?1:0;}',0)
add('constructor-effects',flag+'int count;struct F:Flag<false>{F(){++count;}};int main(){int x=F()?1:0;return x||count!=1;}',0)
add('base-constructor-effects',flag+'int count;struct E{E(){++count;}};struct F:E,Flag<false>{};int main(){int x=F()?1:0;return x||count!=1;}',0)
add('destructor-effects',flag+'int count;struct F:Flag<false>{~F(){++count;}};int main(){int x=F()?1:0;return x||count!=1;}',0)
add('base-destructor-effects',flag+'int count;struct E{~E(){++count;}};struct F:E,Flag<false>{};int main(){int x=F()?1:0;return x||count!=1;}',0)
add('argument-effects',flag+'int count;struct F:Flag<false>{F(int){}};int main(){int x=F(++count)?1:0;return x||count!=1;}',0)
add('default-argument-effects',flag+'int count;struct F:Flag<false>{F(int x=++count){}};int main(){int x=F()?1:0;return x||count!=1;}',0)
add('volatile-argument',flag+'volatile int count=0;struct F:Flag<false>{F(int){}};int main(){int x=F(++count)?1:0;return x||count!=1;}',0)
add('nonempty-receiver',flag+'struct F:Flag<false>{int value;};int main(){return F()?1:0;}',0)
add('escaping-this',flag+'void* saved;struct F:Flag<false>{F(){saved=this;}};int main(){int x=F()?1:0;return x||!saved;}',0)
add('conversion-effects','int count;struct F{operator bool()const{++count;return false;}};int main(){int x=F()?1:0;return x||count!=1;}',0)
add('lifetime-order',flag+'int count;int mark(){return count;}struct F:Flag<false>{~F(){++count;}};int main(){int x=F()?2:mark();return x||count!=1;}',0)
add('throwing-constructor',flag+'struct F:Flag<false>{F(){throw 7;}};int main(){try{int x=F()?1:0;return x+2;}catch(int v){return v!=7;}}',0)
add('throwing-argument',flag+'int fail(){throw 7;}struct F:Flag<false>{F(int){}};int main(){try{int x=F(fail())?1:0;return x+2;}catch(int v){return v!=7;}}',0)
add('unselected-arm-effects',flag+'int count;int main(){int x=Flag<false>()?++count:0;return x||count;}',1)
add('logical-use',flag+'int count;int main(){bool x=Flag<false>() && ++count;return x||count;}',1)
add('condition-use',flag+'int main(){if(Flag<false>())return 1;return 0;}',1)
add('explicit-bool',flag+'int main(){return bool(Flag<false>())?1:0;}',1)
add('scalar-use', 'const int answer=7;struct F{operator int()const{return answer;}};int main(){int v=F();return v!=7;}',1)
add('static-member-demand',flag+'const bool* p=&Flag<false>::value;int main(){return *p?1:0;}',0)
add('member-selection', (ROOT/'pa22/tests/general/300-structured-bool-conditional-member-pointer-dead-branch.t').read_text(),1)
rows=[]
for name,(source,omitted) in cases.items():
 src=WORK/(name+'.cpp');src.write_text(source);ir=src.with_suffix('.lowir');exe=src.with_suffix('.exe')
 command=[str(CC),'--emit-lowir','-O0','--validate-lowir','-o',str(ir),str(src)]
 p=subprocess.run(command,capture_output=True,text=True)
 row=dict(name=name,source=source,source_sha256=hashlib.sha256(source.encode()).hexdigest(),compile_exit=p.returncode,diagnostic=p.stderr,expected_omitted=omitted)
 if not p.returncode:
  raw=ir.read_bytes();stats=subprocess.run(command+['--stats'],capture_output=True,text=True)
  telemetry=[json.loads(x) for x in stats.stderr.splitlines() if x.startswith('{')]
  row.update(lowir_sha256=hashlib.sha256(raw).hexdigest(),stats_identical=stats.returncode==0 and raw==ir.read_bytes(),telemetry=telemetry)
  count=next((s.get('semantic_conversion_receivers_omitted',0) for s in telemetry if 'semantic_conversion_result_work' in s),0)
  row['shape_passed']=count==omitted
  if name=='static-member-demand':row['shape_passed'] &= 'global @' in ir.read_text()
  if name in ('temporary-false','temporary-true','inherited-empty'):row['shape_passed'] &= 'global @' not in ir.read_text()
  p=subprocess.run([str(ROOT/'dev/lowir2native-ref'),'-O0','-o',str(exe),str(ir)],capture_output=True,text=True)
  row.update(backend_exit=p.returncode,backend_diagnostic=p.stderr)
  if name.startswith('throwing-'):
   # Supplied standalone backend cannot merge fundamental RTTI declaration
   # and definition. Preserve that result; use the established PA21 hosted
   # object lane to check actual exception propagation, with unmodified IR.
   obj=ir.with_suffix('.o')
   p=subprocess.run([str(ROOT/'dev/cppgm++-ref'),'-c','-O0','-o',str(obj),str(ir)],capture_output=True,text=True)
   row.update(hosted_backend_exit=p.returncode,hosted_backend_diagnostic=p.stderr)
   if not p.returncode:
    p=subprocess.run(['g++','-no-pie',str(obj),'-o',str(exe)],capture_output=True,text=True)
    row.update(hosted_link_exit=p.returncode,hosted_link_diagnostic=p.stderr)
  if not p.returncode: row['runtime_exit']=subprocess.run([str(exe)],timeout=20).returncode
 row['behavior_passed']=row.get('runtime_exit')==0
 row['passed']=row['behavior_passed'] and row.get('stats_identical',False) and row.get('shape_passed',False)
 rows.append(row)
print(json.dumps(dict(compiler=str(CC),compiler_sha256=hashlib.sha256(CC.read_bytes()).hexdigest(),cases=rows),indent=2))
sys.exit(not all(r['passed'] for r in rows))
