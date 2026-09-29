#!/usr/bin/env python3
"""Scalar body proofs must preserve declaration noexcept and real unwind ownership."""
from pathlib import Path
import hashlib,json,subprocess,sys
ROOT=Path(__file__).resolve().parents[2]
CC,WORK=[Path(p).resolve() for p in sys.argv[1:]]
WORK.mkdir(parents=True,exist_ok=True)
from list104 import LIB
OBJ='int live,drops;struct M{int n;M(int x):n(x){++live;}~M(){--live;++drops;}};'
cases={
 'declaration_noexcept':LIB+OBJ+'int get(int n){return n+1;}int main(){std::initializer_list<M> a{1,2};static_assert(!noexcept(get(2)),"declaration");return get(a.size())!=3||live!=2;}',
 'scalar_template_body':LIB+OBJ+'template<class T>T get(T n){return n+1;}int main(){std::initializer_list<M> a{1,2};return get<int>(a.size())!=3||live!=2;}',
 'throwing_call':LIB+OBJ+'int get(){throw 7;}int f(){std::initializer_list<M> a{1,2};return get();}int main(){try{f();}catch(int n){return n!=7||live||drops!=2;}return 1;}',
 'throwing_argument':LIB+OBJ+'int arg(){throw 7;}int get(int n){return n;}int f(){std::initializer_list<M> a{1,2};return get(arg());}int main(){try{f();}catch(int n){return n!=7||live||drops!=2;}return 1;}',
 'virtual_override':LIB+OBJ+'struct B{virtual int get(){return 3;}};struct D:B{int get(){throw 7;}};int f(B& b){std::initializer_list<M>a{1,2};return b.get();}int main(){D d;try{f(d);}catch(int n){return n!=7||live||drops!=2;}return 1;}',
 'indirect_call':LIB+OBJ+'int get(){throw 7;}int f(int(*p)()){std::initializer_list<M>a{1,2};return p();}int main(){try{f(get);}catch(int n){return n!=7||live||drops!=2;}return 1;}',
 'throwing_conversion':LIB+OBJ+'struct X{operator int(){throw 7;}};int get(int n){return n;}int f(){std::initializer_list<M>a{1,2};X x;return get(x);}int main(){try{f();}catch(int n){return n!=7||live||drops!=2;}return 1;}',
 'temporary_destructor':LIB+OBJ+'struct X{~X()noexcept(false){throw 7;}};int get(const X&){return 1;}int f(){std::initializer_list<M>a{1,2};return get(X());}int main(){try{f();}catch(int n){return n!=7||live||drops!=2;}return 1;}',
 'scalar_body_owning_parameter':LIB+OBJ+'bool fail=true;struct X{X(){}X(const X&){}~X()noexcept(false){if(fail){fail=false;throw 7;}}};int get(X){return 1;}int f(){std::initializer_list<M>a{1,2};return get(X());}int main(){try{f();}catch(int n){return n!=7||live||drops!=2;}return 1;}',
 'allocation_failure':LIB+OBJ+'void*operator new(unsigned long){throw 7;}void operator delete(void*){}int get(){return 1;}int f(){std::initializer_list<M>a{1,2};return get()+*new int(2);}int main(){try{f();}catch(int n){return n!=7||live||drops!=2;}return 1;}',
 'active_handler':LIB+OBJ+'int get(int n){return n+1;}int main(){try{throw 7;}catch(int n){std::initializer_list<M>a{1,2};if(get(n)!=8||live!=2)return 1;}return live||drops!=2;}',
}
for name in ('200-initializer-list-backing-array-lifetime','100-source-nested-catch-miss-cleans-active-handler'):
 cases[name]=(ROOT/'pa21/tests/general'/(name+'.t')).read_text()
rows=[]
for name,source in cases.items():
 src=WORK/(name+'.cpp');src.write_text(source);ir=src.with_suffix('.lowir');obj=src.with_suffix('.o');exe=WORK/name
 commands=[]
 for cmd in ([CC,'--emit-lowir','-O0','--validate-lowir','-o',ir,src],[ROOT/'dev/cppgm++-ref','-c','-O0','-o',obj,ir],['g++','-no-pie',obj,'-o',exe],[exe]):
  try:
   p=subprocess.run(list(map(str,cmd)),capture_output=True,text=True,timeout=30)
   r=dict(argv=list(map(str,cmd)),exit=p.returncode,stdout=p.stdout,stderr=p.stderr)
  except subprocess.TimeoutExpired:r=dict(argv=list(map(str,cmd)),exit=124,stderr='timeout')
  commands.append(r)
  if r['exit']:break
 passed=len(commands)==4 and not commands[-1]['exit']
 rows.append(dict(name=name,source=source,commands=commands,passed=passed))
 print(name,'PASS' if passed else 'FAIL',r['stderr'].strip(),flush=True)
(WORK/'results.json').write_text(json.dumps(dict(compiler_sha256=hashlib.sha256(CC.read_bytes()).hexdigest(),rows=rows),indent=2)+'\n')
sys.exit(not all(r['passed'] for r in rows))
