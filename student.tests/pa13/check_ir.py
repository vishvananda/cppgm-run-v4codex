#!/usr/bin/env python3
"""Raw lifecycle order, bounded cleanup, demand and multi-TU object identity."""
from pathlib import Path
import json,re,subprocess,tempfile,sys
ROOT=Path(__file__).resolve().parents[2]
compiler=Path(sys.argv[1]).resolve() if len(sys.argv)>1 else ROOT/'dev/cppgm++'
def run(cmd):
 r=subprocess.run(list(map(str,cmd)),capture_output=True,text=True,timeout=60)
 assert r.returncode==0,(list(map(str,cmd)),r.returncode,r.stderr)
 return r
with tempfile.TemporaryDirectory(prefix='pa13-ir-') as tmp:
 work=Path(tmp)
 ir=work/'result.lowir'
 source=ROOT/'student.tests/pa13/dispatch-lifetimes.cpp'
 run([compiler,'--emit-lowir','-O0','--validate-lowir','-o',ir,source])
 entries=re.findall(r'^function .*?object=([^,\]]+)',ir.read_text(),re.M)
 for cls in ('4Base','7Derived'):
  seq=[entries.index('_ZN'+cls+'D'+str(k)+'Ev') for k in (2,0,1)]
  assert seq==sorted(seq),(cls,seq)
 # Slot demand must not instantiate an unused unrelated member body.
 source=work/'unused.cpp';source.write_text('struct B{virtual int f(){return 7;} int unused(){return 99;} }; int main(){B b;return b.f()-7;}')
 run([compiler,'--emit-lowir','-O0','--validate-lowir','-o',ir,source])
 assert '_ZN1B6unusedEv' not in ir.read_text()
 counts=[]
 for n in (8,64):
  source=work/f'cleanup-{n}.cpp'
  fields=''.join(f'M m{i};' for i in range(n))
  source.write_text('int count;struct M{~M()noexcept{++count;}};struct B{'+fields+'virtual ~B()noexcept{}};int main(){B* p=new B;delete p;return count!='+str(n)+';}')
  stats=run([compiler,'--emit-lowir','-O0','--stats','--validate-lowir','-o',ir,source])
  counts.append(json.loads(stats.stderr.splitlines()[-1])['instructions'])
  exe=work/'exec';run([ROOT/'dev/lowir2native-ref','-O0','-o',exe,ir]);run([exe])
 # Eightfold source growth allows at most ninefold IR, a diagnostic work bound.
 assert counts[1] <= 9*counts[0],counts
 header='struct A{virtual int f()const{return 1;}virtual ~A(){}};struct B:A{int f()const override{return 7;}};'
 a=work/'a.cpp';b=work/'b.cpp'
 a.write_text(header+'int other(){B b;A& a=b;return a.f();}')
 b.write_text(header+'int other();int main(){B b;A& a=b;return a.f()!=7 || other()!=7;}')
 run([compiler,'--emit-lowir','-O0','--validate-lowir','-o',ir,a,b])
 exe=work/'multi';run([ROOT/'dev/lowir2native-ref','-O0','-o',exe,ir]);run([exe])
 print('lifecycle order, lazy bodies, bounded cleanup and multi-TU controls passed',counts)
