#!/usr/bin/env python3
"""Explicit exception-region exit, lifetime and jump rejection controls."""
from pathlib import Path
import hashlib,json,subprocess,sys
ROOT=Path(__file__).resolve().parents[2]
CC,WORK=[Path(p).resolve() for p in sys.argv[1:]]
WORK.mkdir(parents=True,exist_ok=True)
cases={'break_try': 'int main(){for(int i=0;i<2;++i){try{break;}catch(...){return 1;}}return 0;}',
 'continue_try': 'int n;int main(){for(int i=0;i<2;++i){try{++n;continue;}catch(...){return 1;}}return '
                 'n!=2;}',
 'break_handler': 'int live;struct E{E(){++live;}~E(){--live;}};int main(){while(true){try{throw '
                  'E();}catch(...){break;}}return live;}',
 'continue_handler': 'int live;struct E{E(){++live;}~E(){--live;}};int main(){for(int i=0;i<3;++i){try{throw '
                     'E();}catch(...){continue;}}return live;}',
 'goto_handler': 'int live;struct E{E(){++live;}~E(){--live;}};int main(){try{throw E();}catch(...){goto '
                 'done;}done:return live;}',
 'inner_loop_handler': 'int live;struct E{E(){++live;}~E(){--live;}};int main(){try{throw '
                       'E();}catch(...){for(int i=0;i<2;++i){if(i)break;continue;}if(live!=1)return '
                       '1;}return live;}',
 'goto_within_try': 'int main(){try{goto label;return 1;label:throw 7;}catch(int n){return n!=7;}}',
 'goto_within_handler': 'int live;struct E{E(){++live;}~E(){--live;}};int main(){try{throw '
                        'E();}catch(...){goto label;return 1;label:if(live!=1)return 2;}return live;}',
 'outer_cleanup_throw_on_return': 'int live;struct G{~G()noexcept(false){throw 9;}};struct '
                                  'E{E(){++live;}~E(){--live;}};int f(){G g;try{throw E();}catch(...){return '
                                  '0;}}int main(){try{f();}catch(int n){return n!=9||live;}return 1;}',
 'conditional_glvalue_throw': 'int main(){int n=0;try{(false?n:throw 7);return 1;}catch(int x){return '
                              'x!=7;}}'}
cases.update({'break_try_outer_destructor': 'struct G{~G()noexcept(false){throw 9;}};int f(){while(true){G '
                               'g;try{break;}catch(int){return 2;}}return 3;}int main(){try{f();}catch(int '
                               'n){return n!=9;}return 1;}',
 'break_nested_handlers': 'int live;struct E{E(){++live;}~E(){--live;}};int main(){while(true){try{throw '
                          'E();}catch(...){try{throw E();}catch(...){break;}}}return live;}',
 'break_to_enclosing_handler': 'int live;struct E{E(){++live;}~E(){--live;}};int main(){try{throw '
                               'E();}catch(...){while(true){try{throw '
                               'E();}catch(...){break;}}if(live!=1)return 1;}return live;}',
 'switch_break_handler': 'int live;struct E{E(){++live;}~E(){--live;}};int main(){switch(1){case 1:try{throw '
                         'E();}catch(...){break;}}return live;}',
 'range_continue_handler': 'int live;struct E{E(){++live;}~E(){--live;}};int main(){int a[3]={1,2,3};for(int '
                           'n:a){try{throw E();}catch(...){continue;}}return live;}',
 'range_inside_handler': 'int live;struct E{E(){++live;}~E(){--live;}};int main(){int a[3]={1,2,3};try{throw '
                         'E();}catch(...){for(int n:a){if(n==2)break;continue;}if(live!=1)return 1;}return '
                         'live;}',
 'goto_nested_cleanup_order': 'int trace;struct G{int n;G(int x):n(x){}~G(){trace=trace*10+n;}};int '
                              'main(){try{G a(1);try{throw 7;}catch(int){G b(2);goto '
                              'done;}}catch(...){return 1;}done:return trace!=21;}',
 'lambda_template_jump': 'int live;struct E{E(){++live;}~E(){--live;}};template<int N>int f(){auto '
                         'l=[&](){for(int i=0;i<N;++i){try{throw E();}catch(...){continue;}}return '
                         'live;};return l();}int main(){return f<2>()||f<3>();}',
 'outer_early_label_in_handler': 'int main(){try{throw 7;}catch(int n){goto again;if(n){again:return '
                                 'n!=7;}}return 1;}'})
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
rejections = {'goto_into_try': 'int main(){try{goto ready;try{ready:throw 7;}catch(int n){return n!=7;}}catch(...){return 1;}return 2;}',
 'goto_into_handler': 'int main(){goto label;try{throw 7;}catch(...){label:return 0;}}',
 'switch_into_try': 'int main(){switch(1){try{case 1:break;}catch(...){}}}'}
for name,source in rejections.items():
 src=WORK/(name+'.cpp');src.write_text(source)
 command=run([CC,'--emit-lowir','-O0','-o',src.with_suffix('.lowir'),src])
 passed=command['exit'] not in (0,124) and command['exit']>0
 rows.append(dict(name=name,source=source,commands=[command],passed=passed,expected='reject'))
 print(name,'PASS' if passed else 'FAIL',flush=True)
(WORK/'results.json').write_text(json.dumps(dict(compiler_sha256=hashlib.sha256(CC.read_bytes()).hexdigest(),rows=rows),indent=2)+'\n')
sys.exit(0 if all(r['passed'] for r in rows) else 1)
