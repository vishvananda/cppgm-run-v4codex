#!/usr/bin/env python3
"""Check ABI query/emission separation across independently analyzed TUs."""
from pathlib import Path
import re
import subprocess
import tempfile
ROOT=Path(__file__).resolve().parents[2]
header='''struct Base{void* first;void* second;Base(void* a,void* b):first(a),second(b){} Base(Base const&)=default;};
struct Pair:Base{Pair(void* a,void* b):Base(a,b){} Pair(Pair const& x):Base(x){}};
'''
def compile_source(work,name,source):
 path=work/(name+'.cpp');ir=work/(name+'.lowir');path.write_text(source)
 subprocess.run([str(x)for x in [ROOT/'dev/cppgm++','--emit-lowir','-O0','--validate-lowir','-o',ir,path]],check=True)
 return ir.read_text()
with tempfile.TemporaryDirectory(prefix='pa12-parameter-')as directory:
 work=Path(directory)
 declaration=compile_source(work,'declaration',header+'int accept(Pair);int main(){}')
 definition=compile_source(work,'definition',header+'int accept(Pair value){Pair copy(value);return copy.first==0;}')
 caller=compile_source(work,'caller',header+'int accept(Pair);int main(){Pair p(0,0);return accept(p)!=1;}')
 for text in [declaration,definition,caller]:
  signature=re.search(r'(?:declare )?function @accept\(([^\n]*)',text)
  assert signature and 'obj<16x8>'in signature[1],text
 assert '_ZN4PairC1ERKS_'not in declaration,declaration
 assert '_ZN4PairC1ERKS_'in definition and '_ZN4PairC1ERKS_'in caller
 assert re.search(r'copyobj 16x8 %[^,]+, %',definition),definition
 # A nontrivial scalar copy or a constructor observing destination identity
 # must keep the same indirect ABI even if its body is known in this TU.
 negative=compile_source(work,'negative',(ROOT/'student.tests/pa12/parameter-representation.cpp').read_text())
 for name in ['observe','identity','other','escaped']:
  signature=re.search(r'function @'+name+r'\(([^\n]*)',negative)
  assert signature and ': ptr [pass=by_address'in signature[1],name
 assert re.search(r'copy ptr 16',negative),negative
 # Link independently parsed caller/definition inputs through the course LowIR
 # path, then execute the source-generated program with the supplied backend.
 linked=work/'linked.lowir';exe=work/'linked'
 subprocess.run([str(x)for x in [ROOT/'dev/cppgm++','--emit-lowir','-O0','--validate-lowir','-o',linked,work/'definition.cpp',work/'caller.cpp']],check=True)
 subprocess.run([str(x)for x in [ROOT/'dev/lowir2native-ref','-O0','-o',exe,linked]],check=True)
 subprocess.run([str(exe)],check=True)
print('parameter representation and declaration-only emission properties: pass')
