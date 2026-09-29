#!/usr/bin/env python3
"""Reduced observable handler lifetime using the actual reference continuation."""
from pathlib import Path
import hashlib,json,re,subprocess,sys
from reference110 import original,revised
ROOT=Path(__file__).resolve().parents[2]
WORK=Path(sys.argv[1]).resolve();WORK.mkdir(parents=True,exist_ok=True)
def run(cmd):
 p=subprocess.run(list(map(str,cmd)),capture_output=True,text=True,timeout=30)
 return dict(argv=list(map(str,cmd)),exit=p.returncode,stdout=p.stdout,stderr=p.stderr)
host='''#include <cstdio>
extern int trace asm("_Z5trace");
int choose(bool);
struct E { ~E() { trace = trace * 10 + 3; } };
extern "C" int injected(const int *value) asm("_Z4readRK5Value");
extern "C" int injected(const int *value) {
  if (*value == 7) throw E();
  throw 9L;
}
int main() {
  try { choose(true); } catch(long n) { std::printf("%d\\n",trace); return n != 9 || trace != 813; }
  return 2;
}
'''
driver=WORK/'driver.cpp';driver.write_text(host);rows=[]
for name,text in [('original',original),('revised',revised)]:
 # Only make the potentially throwing read call externally controllable and
 # move the fixture entry aside. The entire choose/cleanup body stays intact.
 pattern=r'function @read\([^\n]*\n.*?\n}'
 match=re.search(pattern,text,re.S);assert match
 header=match.group(0).splitlines()[0]
 replacement='declare '+header[:-2]
 text=text[:match.start()]+replacement+text[match.end():]
 text=text.replace('function @main() -> i32 [role=entry, binding=strong, keep_alias=yes]',
                   'function @fixture_main() -> i32 [binding=strong, object=fixture_main]')
 ir=WORK/(name+'.lowir');ir.write_text(text);obj=ir.with_suffix('.o');exe=WORK/name
 commands=[]
 for cmd in ([ROOT/'dev/cppgm++-ref','-c','-O0','-o',obj,ir],['g++','-no-pie',driver,obj,'-o',exe],[exe]):
  commands.append(run(cmd))
  if commands[-1]['exit']:break
 row=dict(name=name,lowir_sha256=hashlib.sha256(text.encode()).hexdigest(),commands=commands);rows.append(row)
 print(name,commands[-1]['exit'],commands[-1]['stderr'],flush=True)
 assert len(commands)==3 and commands[-1]['exit']==(1 if name=='original' else 0),row
(WORK/'results.json').write_text(json.dumps(dict(host=host,rows=rows),indent=2)+'\n')
