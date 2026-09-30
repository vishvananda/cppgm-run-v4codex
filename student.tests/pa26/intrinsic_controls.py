#!/usr/bin/env python3
"""Run typed header/variadic controls explicitly; never participates in course gates."""
import argparse,json,pathlib,subprocess
p=argparse.ArgumentParser();p.add_argument('--compiler',default='dev/cppgm++');p.add_argument('--out',default='/tmp/pa26-143/intrinsic-controls');a=p.parse_args()
compiler=pathlib.Path(a.compiler).resolve();out=pathlib.Path(a.out).resolve();out.mkdir(parents=True,exist_ok=True)
root=pathlib.Path(__file__).resolve().parent; records=[]
def run(cmd,success=True):
    r=subprocess.run(list(map(str,cmd)),capture_output=True,text=True)
    records.append(dict(command=list(map(str,cmd)),status=r.returncode,stdout=r.stdout,stderr=r.stderr))
    (out/'results.json').write_text(json.dumps(records,indent=2)+'\n')
    assert (r.returncode==0)==success,records[-1]
for name in ['type_traits','header_intrinsics','variadic','noexcept_parameter','integer_pack','constexpr_if']:
    obj=out/(name+'.o');exe=out/name
    run([compiler,'-c','-o',obj,root/(name+'.cpp')])
    run(['g++',obj,*([root/'variadic-host.cpp'] if name=='variadic' else []),'-o',exe]);run([exe])
negative={
'negative-pack':'template<int N>struct A;template<int...>struct S{};template<int N>struct A{typedef S<__integer_pack(N)...>type;};A<-1>::type x;',
'va-fixed':'void f(int x){__builtin_va_list ap;__builtin_va_start(ap,x);}',
'va-wrong-last':'void f(int x,int y,...){__builtin_va_list ap;__builtin_va_start(ap,x);}',
'va-wrong-type':'void f(int x,...){int ap;__builtin_va_start(ap,x);}',
'va-arity':'void f(int x,...){__builtin_va_list ap;__builtin_va_start(ap);}',
'va-void':'void f(__builtin_va_list ap){__builtin_va_arg(ap,void);}',
'address-rvalue':'int main(){__builtin_addressof(1);}',
'address-bitfield':'struct B{int x:3;};int main(){B b;__builtin_addressof(b.x);}',
'function-global':'const char* p=__func__;',
'looser-throw':'struct B{virtual void f()throw();};struct D:B{void f();};',
}
for name,code in negative.items():
    src=out/(name+'.cpp');src.write_text(code+'\n');run([compiler,'-c','-o',out/(name+'.o'),src],False)
src=out/'throw.cpp';src.write_text('void f()throw();static_assert(noexcept(f()),"empty throw");struct B{virtual void f()throw();};struct D:B{void f()noexcept;};int main(){}\n')
run([compiler,'-c','-o',out/'throw.o',src]);run(['g++',out/'throw.o','-o',out/'throw']);run([out/'throw'])
print('17 intrinsic/trait controls passed')
