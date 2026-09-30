#!/usr/bin/env python3
"""Shared-subobject identity, final overriders and inherited virtual declarations."""
from pathlib import Path
import hashlib,json,subprocess,sys
ROOT=Path(__file__).resolve().parents[2]
def sha(p): return hashlib.sha256(p.read_bytes()).hexdigest()
base='struct R{virtual int f(){return 1;}};'
abstract='struct R{virtual int f()=0;};'
cases=[]
def add(name,source,accepted=True): cases.append((name,source,accepted))
add('ambiguous-shared',base+'struct A:virtual R{int f(){return 2;}};struct B:virtual R{int f(){return 3;}};struct D:A,B{};',False)
add('repeated-same-overrider',base+'struct A:virtual R{int f(){return 2;}};struct B:A{};struct C:A{};struct D:B,C{};',False)
add('repeated-nonvirtual-roots',base+'struct A:R{int f(){return 2;}};struct B:A{};struct C:A{};struct D:B,C{};')
add('one-shared-overrider',abstract+'struct A:virtual R{int f(){return 2;}};struct B:virtual R{};struct D:A,B{};void use(){D d;}')
add('shared-remains-abstract',abstract+'struct A:virtual R{};struct B:virtual R{};struct D:A,B{};void use(){D d;}',False)
add('leaf-resolves',base+'struct A:virtual R{int f(){return 2;}};struct B:virtual R{int f(){return 3;}};struct D:A,B{int f(){return 4;}};')
add('leaf-resolves-repeated-member',base+'struct A:virtual R{int f(){return 2;}};struct B:A{};struct C:A{};struct D:B,C{int f(){return 4;}};')
add('leaf-makes-abstract',base+'struct A:virtual R{};struct B:virtual R{};struct D:A,B{int f()=0;};void use(){D d;}',False)
add('shared-overrider-occurrence',base+'struct A:virtual R{int f(){return 2;}};struct B:virtual A{};struct C:virtual A{};struct D:B,C{};')
add('dominating-shared-overrider',base+'struct A:virtual R{int f(){return 2;}};struct B:virtual A{int f(){return 3;}};struct D:B,virtual A{};')
add('nonvirtual-receiver-does-not-dominate-sibling',base+'struct A:virtual R{int f(){return 2;}};struct B:A{int f(){return 3;}};struct C:A{};struct D:B,C{};',False)
add('distinct-root-return-types','struct A{virtual int f(){return 1;}};struct B{virtual void f(){}};struct D:A,B{};')
add('nonvirtual-abstract-sibling',abstract+'struct A:R{int f(){return 2;}};struct B:R{};struct D:A,B{};void use(){D d;}',False)
add('virtual-plus-nonvirtual-abstract',abstract+'struct A:virtual R{int f(){return 2;}};struct B:R{};struct D:A,B{};void use(){D d;}',False)
add('virtual-plus-nonvirtual-independent',base+'struct A:virtual R{int f(){return 2;}};struct B:R{int f(){return 3;}};struct D:A,B{};')
add('nested-nonvirtual-inside-shared','struct R{virtual int f(){return 1;}};struct P:R{};struct A:virtual P{int f(){return 2;}};struct B:virtual P{};struct D:A,B{};')
add('nested-shared-ambiguous','struct R{virtual int f(){return 1;}};struct P:R{};struct A:virtual P{int f(){return 2;}};struct B:virtual P{int f(){return 3;}};struct D:A,B{};',False)
add('overloaded-roots','struct R{virtual int f(int){return 1;}virtual int f(double){return 2;}};struct A:virtual R{int f(int){return 3;}};struct B:virtual R{int f(double){return 4;}};struct D:A,B{};')
add('same-name-unrelated-signature',base+'struct A:virtual R{int f(){return 2;}};struct B:virtual R{int f(int){return 3;}};struct D:A,B{};')
add('final-through-shared',base+'struct A:virtual R{int f()final{return 2;}};struct B:virtual R{};struct D:A,B{int f(){return 3;}};',False)
add('covariant-shared','struct X{};struct Y:X{};struct R{virtual X* f(){return 0;}};struct A:virtual R{Y* f(){return 0;}};struct B:virtual R{};struct D:A,B{};')
add('template-shared',base+'template<class T>struct A:virtual T{int f(){return 2;}};template<class T>struct B:virtual T{};struct D:A<R>,B<R>{};')
add('template-ambiguous',base+'template<int I>struct A:virtual R{int f(){return I;}};struct D:A<2>,A<3>{};',False)
add('virtual-inheritance-is-not-polymorphism','struct V{};struct B:virtual V{};struct D:B{};D*cast(B*b){return dynamic_cast<D*>(b);}',False)
# Actual edge occurrence matters even if two derived types share an override name.
for reverse in (False,True):
 branches='B,A' if reverse else 'A,B'
 add('late-maximum-'+str(reverse),base+'struct A:virtual R{};struct B:virtual R{int f(){return 2;}};struct D:'+branches+'{};')
def check(cc,work):
 work.mkdir(parents=True,exist_ok=True);rows=[]
 for name,source,accepted in cases:
  src=work/(name+'.cpp');src.write_text(source);ir=src.with_suffix('.lowir')
  p=subprocess.run([str(cc),'--emit-lowir','-O0','--validate-lowir','--stats','-o',str(ir),str(src)],capture_output=True,text=True)
  row=dict(name=name,source=source,expected_accept=accepted,exit=p.returncode,passed=(p.returncode==0)==accepted,diagnostics=[x for x in p.stderr.splitlines() if not x.startswith('{')],telemetry=[json.loads(x) for x in p.stderr.splitlines() if x.startswith('{')])
  if not p.returncode: row['lowir_sha256']=sha(ir)
  rows.append(row);print(name,'PASS' if row['passed'] else 'FAIL',file=sys.stderr,flush=True)
 return dict(compiler=str(cc),compiler_sha256=sha(cc),cases=rows)
if __name__=='__main__':
 cc,work=map(lambda x:Path(x).resolve(),sys.argv[1:3]);result=check(cc,work)
 print(json.dumps(result,indent=2));sys.exit(not all(r['passed'] for r in result['cases']))
