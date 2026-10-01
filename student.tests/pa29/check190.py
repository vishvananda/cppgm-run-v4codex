#!/usr/bin/env python3
"""Explicit complex constant/RTTI and cross-handoff audit controls."""
import hashlib,json,pathlib,subprocess,sys
root=pathlib.Path(__file__).resolve().parents[2]
out=pathlib.Path(sys.argv[1]).resolve();out.mkdir(parents=True,exist_ok=True)
cc=pathlib.Path(sys.argv[2]).resolve() if len(sys.argv)>2 else root/'dev/cppgm++'
src=root/'student.tests/pa29/controls190';rows=[]
def run(args,ok=True):
    args=list(map(str,args))
    p=subprocess.run(args,cwd=root,capture_output=True,text=True,timeout=90)
    rows.append(dict(command=args,status=p.returncode,expected_success=ok,stdout=p.stdout,stderr=p.stderr))
    (out/'checks.json').write_text(json.dumps(rows,indent=2)+'\n')
    assert (p.returncode==0)==ok,(args,p.returncode,p.stderr)
    return p
for name in ['complex-special','complex-rtti','complex-identity','fixed-lists','integrated']:
    obj=out/(name+'.o');exe=out/name;lir=out/(name+'.lowir')
    for level in ['-O0','-O2']:
        run([cc,level,'-c',src/(name+'.cpp'),'-o',obj])
        run(['g++',obj,'-o',exe]);run([exe])
    run(['clang++','-std=c++11','-include','typeinfo',src/(name+'.cpp'),'-o',out/'host']);run([out/'host'])
    symbols=run(['nm',obj]).stdout
    if name=='complex-rtti':
        for spelling in ['_ZTICf','_ZTICd','_ZTICe','_ZTIPCd']:
            assert any(spelling in l and ' V ' in l for l in symbols.splitlines()),(spelling,symbols)
    assert 'unused' not in symbols
    run([cc,'-c','--emit-lowir','--validate-lowir',src/(name+'.cpp'),'-o',lir])
    run([root/'dev/lowir',lir,'-o',out/(name+'.canonical')])
    run(['readelf','-SWsrg',obj]);run(['readelf','--debug-dump=frames',obj]);run(['objdump','-dr',obj])
obj=out/'abi.o'
run([cc,'-c',src/'abi-source.cpp','-o',obj])
for host in ['g++','clang++']:
    run([host,'-std=c++11',src/'abi-host.cpp',obj,'-o',out/'abi']);run([out/'abi'])
run(['readelf','-SWsrg',obj]);run(['readelf','--debug-dump=frames',obj])
# A finite overflow or division by zero must still fail constant evaluation.
for name,expr in [('overflow','z*z'),('zero-divisor','z/zero')]:
    p=out/(name+'.cpp')
    p.write_text('constexpr auto z=__builtin_complex(1e308,1e308);constexpr auto zero=__builtin_complex(0.,0.);constexpr auto bad='+expr+';\n')
    run([cc,'-c',p,'-o',out/'reject.o'],False)
for name,body in {
    'false':'struct C{int n;constexpr explicit operator bool()const{return n==3;}};template<class T>void f(){static_assert(C{2},"fixed false");}',
    'narrow':'struct C{int n;constexpr explicit operator bool()const{return true;}};template<class T>void f(){static_assert(C{1.5},"narrow");}',
    'lifetime':'struct C{int n;~C(){} constexpr explicit operator bool()const{return true;}};template<class T>void f(){static_assert(C{3},"lifetime");}',
    'nonconstant':'struct C{int n;explicit operator bool()const{return true;}};template<class T>void f(){static_assert(C{3},"nonconstant");}',
    'access':'class C{int n;constexpr C(int x):n(x){} public:constexpr explicit operator bool()const{return true;}};template<class T>void f(){static_assert(C{3},"private");}',
}.items():
    p=out/(name+'-list.cpp');p.write_text(body+'\nint main(){f<int>();}\n')
    for compiler in [cc,'clang++']:
        run([compiler,'-std=c++11','-c',p,'-o',out/'reject.o'],False)
print(len(rows),'commands passed')
