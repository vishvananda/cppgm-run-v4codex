#!/usr/bin/env python3
"""Independent type-query rejection and ABI checks; successful programs use check_functions.py."""
from pathlib import Path
import subprocess, tempfile
root=Path(__file__).resolve().parents[2]
with tempfile.TemporaryDirectory(prefix='pa14-queries-') as directory:
 work=Path(directory)
 out=work/'names'
 r=subprocess.run([root/'dev/abimangle','-o',out,root/'student.tests/pa14/query-names.abi'],capture_output=True,text=True)
 assert r.returncode==0,r.stderr
 # doc/itanium-mangling.txt: cl ... E, sr qualifier+ E simple-id,
 # fp_ for the first function parameter, and DT expression E.
 assert out.read_text()=='DTclsr2nsE1ffp_EE\nDTcl1fIiEfp_EE\n'
 cases=[
  'template<class T> auto bad(T p)->decltype(absent_value) { return 0; }',
  'template<class T> auto bad(T p)->decltype(T) { return 0; }',
  'template<class T> auto bad(T p)->decltype(absent_call(p)); int main(){return bad(1);}',
  'template<class T> auto bad(T p)->decltype(*p); int main(){return bad(1);}',
  'template<class T> auto bad(T p)->decltype(p&1); int main(){return bad(1.5);}',
  'template<class T> auto bad(T p)->decltype(p<nullptr); int main(){return bad((int*)0);}',
  'int f(int*); template<class T> auto bad(T p)->decltype(f(false));',
 ]
 for i,source in enumerate(cases):
  path=work/f'reject-{i}.cpp';path.write_text(source)
  r=subprocess.run([root/'dev/cppgm++','--emit-lowir','-O0','-o',work/'output',path],capture_output=True,text=True)
  assert r.returncode==1,(i,r.returncode,r.stderr)
 print('two unresolved-name ABI cases and seven type-query rejections PASS')
