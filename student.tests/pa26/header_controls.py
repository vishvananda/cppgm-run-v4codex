#!/usr/bin/env python3
"""Explicit host-header prerequisites, independent of the required string fixture."""
import argparse, hashlib, json, pathlib, subprocess
p=argparse.ArgumentParser(); p.add_argument('--compiler',default='dev/cppgm++'); p.add_argument('--out',default='/tmp/pa26-143/headers')
a=p.parse_args(); compiler=str(pathlib.Path(a.compiler).resolve()); out=pathlib.Path(a.out).resolve(); out.mkdir(parents=True,exist_ok=True)
records=[]
def run(args, success=True):
    r=subprocess.run(list(map(str,args)),capture_output=True,text=True)
    records.append(dict(args=list(map(str,args)),status=r.returncode,stdout=r.stdout,stderr=r.stderr))
    assert (r.returncode==0)==success, records[-1]
    return r.stdout

def case(name,source,helper='',flags=(),success=True):
    src=out/(name+'.cpp'); obj=out/(name+'.o'); exe=out/name; src.write_text(source)
    run([compiler,'-c',*flags,'-o',obj,src],success)
    if not success: return
    host=out/(name+'-host.cpp'); host.write_text(helper)
    run(['g++','-std=c++11',host,obj,'-o',exe]); run([exe])
    run(['readelf','-Ws',obj])

case('target-types','''#include <cstddef>
static_assert(sizeof(std::size_t)==8,"size_t");
static_assert(sizeof(std::ptrdiff_t)==8,"ptrdiff_t");
int main(){return 0;}
''')
case('wide-header','''#include <cwchar>
int main(){return std::wcslen(L"wide chars") == 10 ? 0:1;}
''')
case('variadic-shape','''#include <stdarg.h>
static_assert(sizeof(va_list)==24,"va_list size");
static_assert(alignof(va_list)==8,"va_list alignment");
extern "C" int consume(va_list);
int main(){va_list ap; ap[0].gp_offset=24; ap[0].fp_offset=80;
int x=7; ap[0].overflow_arg_area=&x; ap[0].reg_save_area=&x;
return consume(ap);}
''','''#include <cstring>
extern "C" int consume(__builtin_va_list ap) {
struct Layout { unsigned gp,fp; void *stack,*registers; } copy;
static_assert(sizeof copy==sizeof(__builtin_va_list),"host ABI size");
std::memcpy(&copy,ap,sizeof copy);
return copy.gp==24 && copy.fp==80 && *(int*)copy.stack==7 && copy.stack==copy.registers ? 0:1;
}''')
case('labels','''namespace named __attribute__((visibility("default"))) {
extern int value __asm__("shared_value");
int call([[maybe_unused]] int x) __asm__("shared_call");
int call(int x){return x+value;}
}
int main(){return named::call(9)==32?0:1;}
''','extern "C" { int shared_value=23; }')
case('block-linkage','''namespace owner {
int invoke() noexcept { int helper(int) noexcept; return helper(8); }
int indirect(){int helper(int) noexcept; int (*p)(int)=helper; return p(5);}
int helper(int n) noexcept { return n+7; }
}
int main(){return owner::invoke()+owner::indirect()==27?0:1;}
''')
case('block-hidden','''namespace owner {
int invoke(){int helper(int);return helper(8);}
int fail(){return helper(5);}
}''',success=False)
case('label-conflict','int f() asm("one"); int f() asm("two");',success=False)
case('label-null','int f() asm("one\\0two");',success=False)
(out/'override').mkdir(exist_ok=True)
(out/'override'/'cstddef').write_text('typedef int overridden;\n')
case('include-order','''#include <cstddef>
#ifndef OVERRIDE
#error missing user macro
#endif
static_assert(OVERRIDE==19,"user macro precedence");
int main(){overridden x=OVERRIDE; return x==19?0:1;}
''',flags=['-I',out/'override','-DOVERRIDE=19','-D__GNUC__=99'])
for name in ['first','second','third']:
    (out/name).mkdir(exist_ok=True)
(out/'first'/'wrapper.h').write_text('#define OUTER 3\n#include_next "wrapper.h"\n')
(out/'second'/'wrapper.h').write_text('#define MIDDLE 5\n#include "nested.h"\n')
(out/'second'/'nested.h').write_text('#include_next <terminal.h>\n')
(out/'second'/'terminal.h').write_text('#error resumed in same directory\n')
(out/'third'/'terminal.h').write_text('#define INNER 7\n')
case('include-next',"""#include <wrapper.h>
int main(){return OUTER+MIDDLE+INNER==15?0:1;}
""",flags=['-I',out/'first','-I',out/'first','-I',out/'second','-I',out/'third'])
(out/'results.json').write_text(json.dumps(dict(compiler_sha256=hashlib.sha256(pathlib.Path(compiler).read_bytes()).hexdigest(),records=records),indent=2)+'\n')
print('10 host header prerequisites passed')
