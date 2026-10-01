#!/usr/bin/env python3
"""Explicit GNU complex semantics, ABI and typed-adapter controls for PA29."""
import hashlib,json,pathlib,subprocess,sys
root=pathlib.Path(__file__).resolve().parents[2]
out=pathlib.Path(sys.argv[1]).resolve();out.mkdir(parents=True,exist_ok=True)
compiler=root/'dev/cppgm++';sources=root/'student.tests/pa29/controls187';records=[]
def run(args,ok=True):
 p=subprocess.run(list(map(str,args)),cwd=root,capture_output=True,text=True,timeout=90)
 records.append(dict(command=list(map(str,args)),status=p.returncode,stdout=p.stdout,stderr=p.stderr))
 (out/'checks.json').write_text(json.dumps(records,indent=2)+'\n')
 assert (p.returncode==0)==ok,(args,p.returncode,p.stderr)
 return p
for name in ['complex','constants','operations','arithmetic','adapter']:
 src=sources/(name+'.cpp');obj=out/(name+'.o');exe=out/name
 run([compiler,'-O0','-c',src,'-o',obj]);run(['g++',obj,'-o',exe]);run([exe])
 host=out/(name+'-host');run(['clang++','-std=c++11','-O0',src,'-o',host]);run([host])
 run(['readelf','-Ws',obj]);run(['objdump','-dr',obj])
 run([compiler,'-c','--emit-lowir','--validate-lowir',src,'-o',out/(name+'.lowir')])
for group in ['abi','variadic']:
 obj=out/(group+'.o');exe=out/group
 run([compiler,'-O0','-c',sources/(group+'-source.cpp'),'-o',obj])
 run([compiler,'-c','--emit-lowir','--validate-lowir',sources/(group+'-source.cpp'),'-o',out/(group+'.lowir')])
 for host in ['g++','clang++']:
  run([host,'-std=c++11',sources/(group+'-host.cpp'),obj,'-o',exe]);run([exe])
 run(['readelf','-Ws',obj]);run(['objdump','-dr',obj])
# This source has no host-runtime dependencies. Exercise the external adapter
# and native executable writer, including every complex parameter/result class.
lir=out/'adapter.lowir';canonical=out/'adapter.canonical';again=out/'adapter.again'
run([compiler,'--emit-lowir',sources/'adapter.cpp','-o',lir])
run([root/'dev/lowir',lir,'-o',canonical]);run([root/'dev/lowir',canonical,'-o',again])
assert canonical.read_bytes()==again.read_bytes()
for src in [lir,canonical]:
 run([root/'dev/lowir2native','--dump-machine-ir',out/(src.name+'.mir'),src,'-o',exe]);run([exe])
 mir=(out/(src.name+'.mir')).read_text()
 for expected in ['return c32 -> xmm0\n','return c64 -> xmm0,xmm1\n','return c80 -> st0,st1\n','-> xmm1 : f64\n','fld.f80']:
  assert expected in mir,expected
rejects={
 'arity0':'void f(){__builtin_complex();}',
 'arity1':'void f(){__builtin_complex(1.0);}',
 'arity3':'void f(){__builtin_complex(1.0,2.0,3.0);}',
 'mismatch':'void f(){__builtin_complex(1.0,2.0f);}',
 'integer':'void f(){__builtin_complex(1,2);}',
 'complex':'void f(_Complex double x){__builtin_complex(x,x);}',
 'const':'void f(const _Complex double& x){__real__ x=1;}',
 'ordering':'bool f(_Complex double x){return x<x;}',
 'modulus':'void f(_Complex double x){x%x;}',
 'bitwise':'void f(_Complex double x){x&x;}',
 'implicit':'void f(_Complex double x){double y=x;}',
 'void':'_Complex void x;',
 'unsigned':'unsigned _Complex float x;',
 'pointer':'void f(double* p){__real__ p;}',
 'imag_address':'void f(double x){double* p=&__imag__ x;}',
 'dangling':'template<class T> constexpr auto bad(T x)->decltype(__real__ x){return __real__ x;} constexpr _Complex double z=__builtin_complex(1.0,2.0);static_assert(bad(z)==1,"dangling");',
 'builtin_address':'void(*p)()=__builtin_complex;',
 'vector':'typedef _Complex double V __attribute__((vector_size(16)));'
}
for name,body in rejects.items():
 src=out/(name+'.cpp');src.write_text(body+'\n')
 run([compiler,'-c',src,'-o',out/'rejected.o'],False)
 run(['g++' if name in ('const','imag_address') else 'clang++','-std=c++11','-fsyntax-only',src],False)
print('passed',len(records),'commands;',len(rejects),'rejection inputs')
