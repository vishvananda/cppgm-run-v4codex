#!/usr/bin/env python3
"""Executed full-expression lifetime controls, including later-operand observations."""
from pathlib import Path
import hashlib,json,subprocess,sys
ROOT=Path(__file__).resolve().parents[2]
CC,WORK=[Path(p).resolve() for p in sys.argv[1:]]
WORK.mkdir(parents=True,exist_ok=True)
prefix='int trace,live;struct G{int id;G(int n):id(n){++live;}~G()noexcept{trace=trace*10+id;--live;}};bool use(const G&,bool b)noexcept{return b;}'
cases={}
for op in ('&&','||'):
 for a in ('true','false'):
  for b in ('true','false'):
   evaluated=(a=='true') if op=='&&' else (a=='false')
   trace=21 if evaluated else 1
   cases[f'condition_{op}_{a}_{b}']=prefix+'int main(){if(use(G(1),'+a+')'+op+'use(G(2),'+b+')){}return live||trace!='+str(trace)+';}'
   cases[f'value_{op}_{a}_{b}']=prefix+'int main(){bool v=use(G(1),'+a+')'+op+'use(G(2),'+b+');return live||trace!='+str(trace)+';}'
cases['nested_rhs']=prefix+'int main(){bool gate=true;if(gate&&(use(G(1),true)&&use(G(2),true)))return live||trace!=21;return 2;}'
cases['later_comma_operand']=prefix+'int main(){bool gate=true;int n=(gate&&use(G(1),true),live);return n!=1||live||trace!=1;}'
cases['later_call_argument']=prefix+'int f(bool,int n){return live==1&&n==1;}int main(){bool gate=true;int n=f(gate&&use(G(1),true),live);return n!=1||live||trace!=1;}'
cases['condition_then_throw']=prefix+'int main(){try{if(use(G(1),true)){if(live||trace!=1)return 2;throw 7;}}catch(int n){return n!=7||live||trace!=1;}return 3;}'
cases['rhs_throw']=prefix+'bool fail(const G&){throw 7;}int main(){try{bool gate=true;bool b=use(G(1),gate)&&fail(G(2));}catch(int n){return n!=7||live||trace!=21;}return 2;}'
cases['sibling_short_throw']=prefix+'bool fail(const G&){throw 7;}int main(){try{bool gate=false;bool b=gate&&fail(G(2));if(live||trace)return 2;throw 8;}catch(int n){return n!=8||live||trace;}return 3;}'
cases['nested_default']=prefix+'int f(const G&g=G(1)){return live;}int outer(int n=f()){return n;}int main(){int n=outer();return n!=1||live||trace!=1;}'
cases['static_once']=prefix+'int f(){static int n=use(G(1),true);return n;}int main(){return f()!=1||f()!=1||live||trace!=1;}'
cases['conditional_sibling']=prefix+'int main(){bool yes=false;int n=yes?use(G(1),true):use(G(2),true);return !n||live||trace!=2;}'
required=['200-hidden-eh-condition-call-argument-temporary-cleanup','200-hidden-eh-short-circuit-condition-rhs-temp-cleanup','200-hidden-eh-short-circuit-rhs-temp-cleanup','200-nested-logical-rhs-temporary-value-slot','200-nested-short-circuit-temporary-cleanup','100-range-for-iteration-temporary-lifetime','200-guarded-local-static-initializer-temporary-cleanup']
for name in required:cases[name]=(ROOT/'pa21/tests/general'/f'{name}.t').read_text()
rows=[]
for i,(name,source) in enumerate(cases.items()):
 src=WORK/f'case{i}.cpp';src.write_text(source)
 ir=src.with_suffix('.lowir');obj=src.with_suffix('.o');exe=src.with_suffix('')
 row=dict(name=name,source=source,commands=[])
 for cmd in ([CC,'--emit-lowir','-O0','--validate-lowir','-o',ir,src],[ROOT/'dev/cppgm++-ref','-c','-O0','-o',obj,ir],['g++','-no-pie',obj,'-o',exe],[exe]):
  try:
   p=subprocess.run(list(map(str,cmd)),capture_output=True,text=True,timeout=30)
   row['commands'].append(dict(argv=list(map(str,cmd)),exit=p.returncode,stdout=p.stdout,stderr=p.stderr))
  except subprocess.TimeoutExpired:
   row['commands'].append(dict(argv=list(map(str,cmd)),exit=124));break
  if p.returncode:break
 row['passed']=len(row['commands'])==4 and row['commands'][-1]['exit']==0
 rows.append(row);print(name,'PASS' if row['passed'] else 'FAIL',flush=True)
 (WORK/'results.json').write_text(json.dumps(dict(compiler_sha256=hashlib.sha256(CC.read_bytes()).hexdigest(),rows=rows),indent=2)+'\n')
sys.exit(0 if all(r['passed'] for r in rows) else 1)
