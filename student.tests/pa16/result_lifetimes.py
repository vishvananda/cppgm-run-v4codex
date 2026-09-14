#!/usr/bin/env python3
"""Explicit final-result lifetime/ABI controls; supplied backend executes LowIR."""
from pathlib import Path
import hashlib,json,re,subprocess,sys
ROOT=Path(__file__).resolve().parents[2]
CC=Path(sys.argv[1]).resolve();WORK=Path(sys.argv[2]);WORK.mkdir(parents=True,exist_ok=True)
CASES={
'empty_template':(ROOT/'student.tests/pa16/initialization/empty_result.cpp').read_text(),
'final_noexcept': 'int c,d;struct X{X(){++c;}~X(){++d;}};X f(){return X();}int main(){f();(f());return c==2&&d==2?0:1;}',
'argument_prefix': 'int alive;struct X{X(){++alive;}~X(){--alive;}};X f(X const&){return X();}int main(){f(X());return alive;}',
'comma_results': 'int alive;struct X{X(){++alive;}~X(){--alive;}};X f(){return X();}int main(){(f(),f());return alive;}',
'conditional_results': 'int alive;struct X{X(){++alive;}~X(){--alive;}};X f(){return X();}int main(){volatile int b=1;b?f():f();b=0;b?f():f();return alive;}',
'nonempty_result': 'int alive;struct X{int n;X(int n):n(n){++alive;}~X(){--alive;}};X f(int n){return X(n);}int main(){int v=f(7).n;return v==7&&alive==0?0:1;}',
# PA12 supports cleanup IR; C++ catch/throw execution is a later stage. Test
# noexcept(false) cleanup structurally, with normal execution checked natively.
'potentially_throwing': 'int alive;struct X{X(){++alive;}~X()noexcept(false){--alive;}};X f(X const&){return X();}int main(){f(X());return alive;}',
}
rows=[]
for name,source in CASES.items():
 src=WORK/(name+'.cpp');src.write_text(source);ir=src.with_suffix('.lowir');exe=src.with_suffix('.exe')
 for command in [[CC,'--emit-lowir','-O0','--validate-lowir','-o',ir,src],[ROOT/'dev/lowir2native-ref','-O0','-o',exe,ir],[exe]]:
  p=subprocess.run(command,capture_output=True,text=True,timeout=30)
  assert p.returncode==0,(name,command,p.returncode,p.stderr)
 text=ir.read_text()
 if name=='empty_template':
  assert re.search(r'function @[^\n]+\[pass=indirect_result, object_bytes=1\]',text)
  assert text.count('eh_try')==1,text
 if name=='potentially_throwing':
  assert 'unwind=may' in text or 'unwind=no' not in re.search(r'function @[^\n]*D1Ev[^\n]*',text).group(0)
  assert text.count('eh_try')>=2,text
 rows.append(dict(name=name,source=source,source_sha256=hashlib.sha256(src.read_bytes()).hexdigest(),lowir_sha256=hashlib.sha256(ir.read_bytes()).hexdigest(),native_exit=0,eh_regions=text.count('eh_try')))
 print(name,'PASS',flush=True)
(WORK/'results.json').write_text(json.dumps(dict(compiler_sha256=hashlib.sha256(CC.read_bytes()).hexdigest(),rows=rows),indent=2)+'\n')
