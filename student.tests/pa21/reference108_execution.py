#!/usr/bin/env python3
"""Execute original/revised fixture IR and the independent source reducer."""
from pathlib import Path
import hashlib,json,subprocess,sys
ROOT=Path(__file__).resolve().parents[2]
ENTRY,FINAL,WORK,OUT=[Path(p).resolve() for p in sys.argv[1:]]
WORK.mkdir(parents=True,exist_ok=True)
def run(cmd):
 p=subprocess.run(list(map(str,cmd)),capture_output=True,text=True,timeout=30)
 return dict(argv=list(map(str,cmd)),exit=p.returncode,stdout=p.stdout,stderr=p.stderr)
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
rows=[]
for name,cc in [('original',None),('revised',None),('entry',ENTRY),('final',FINAL)]:
 ir=WORK/(name+'.lowir');obj=ir.with_suffix('.o');exe=WORK/name
 commands=[]
 if cc:
  commands.append(run([cc,'--emit-lowir','-O0','--validate-lowir','-o',ir,ROOT/'student.tests/pa21/aggregate_prefix_reducer108.cpp']))
  assert commands[-1]['exit']==0,commands
 else:
  text=(subprocess.check_output(['git','show','ac988ea33d4997b44e82baaca5a86623fff3127a:pa21/tests/general/200-indirect-param-prologue-copy.ref'],cwd=ROOT,text=True) if name=='original' else (ROOT/'pa21/tests/general/200-indirect-param-prologue-copy.ref').read_text())
  text=text.replace('function @main() -> i32 [role=entry, binding=strong, keep_alias=yes]', 'function @construction_probe() -> i32 [binding=strong, linkage=c, object=construction_probe]')
  ir.write_text(text)
 for cmd in ([ROOT/'dev/cppgm++-ref','-c','-O0','-o',obj,ir],['g++','-std=c++11','-O0','-no-pie',obj,ROOT/'student.tests/pa21/aggregate_prefix_probe108.cpp','-o',exe],[exe]):
  commands.append(run(cmd))
  if commands[-1]['exit']:break
 expected=1 if name in ('original','entry') else 0
 assert len(commands)==(4 if cc else 3) and commands[-1]['exit']==expected,commands
 rows.append(dict(name=name,expected_exit=expected,commands=commands,lowir_sha256=sha(ir),compiler_sha256=sha(cc) if cc else None))
 print(name,commands[-1]['exit'],flush=True)
 OUT.write_text(json.dumps(dict(revision='pa21-aggregate-prefix-108',rows=rows),indent=2)+'\n')
