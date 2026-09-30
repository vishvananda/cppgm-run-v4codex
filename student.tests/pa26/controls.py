#!/usr/bin/env python3
"""Explicit PA26 host ABI controls; artifacts stay outside the checkout."""
import argparse, hashlib, json, os, pathlib, subprocess
p = argparse.ArgumentParser()
p.add_argument('--compiler', default='dev/cppgm++')
p.add_argument('--out', default='/tmp/pa26-142/controls')
a = p.parse_args()
out = pathlib.Path(a.out).resolve(); out.mkdir(parents=True, exist_ok=True)
compiler = str(pathlib.Path(a.compiler).resolve())
records = []
def run(args, expected=0):
    r = subprocess.run(args, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)
    records.append(dict(args=[str(x) for x in args], status=r.returncode, stdout=r.stdout, stderr=r.stderr))
    if r.returncode != expected: raise RuntimeError(records[-1])
    return r.stdout

def case(name, student, host, status=0, flags=()):
    source=out/(name+'.cpp'); helper=out/(name+'-host.cpp'); obj=out/(name+'.o'); exe=out/name
    source.write_text(student); helper.write_text(host)
    run([compiler,'-c',*flags,'-o',obj,source])
    run(['g++','-std=c++11',helper,obj,'-o',exe])
    run([exe],status)
    run(['readelf','-wf',obj])

case('incoming', '''extern "C" void fail(long);
int destroyed;
struct G { ~G(){ ++destroyed; } };
long test(long a,long b,long c,long d,long e,long f,long g,long h) {
  long sum=a+b+c+d+e+f+g+h;
  try { G guard; fail(sum); } catch(int) { return -1; } catch(long x) {
    return destroyed==1 && x==sum+3 ? sum : -2;
  }
  return -3;
}''', '''extern long test(long,long,long,long,long,long,long,long);
extern "C" void fail(long x) { throw x+3; }
int main(){return test(1,2,3,4,5,6,7,8)==36 ? 0:1;}''')
case('outgoing', '''struct Packet { long value; }; void emit(long x) { throw Packet{x}; }''',
'''struct Packet { long value; }; void emit(long);
int main(){try {emit(81);}catch(const Packet& p){return p.value==81?0:1;} return 2;}''')
case('unprotected-frame', '''extern "C" long barrier(long*);
long middle(long a,long b,long c,long d,long e,long f,long g,long h) {
 long data[67]; for(int i=0;i<67;++i) data[i]=a+b+c+d+e+f+g+h+i;
 return barrier(data)+data[4];
}''', '''long middle(long,long,long,long,long,long,long,long);
extern "C" long barrier(long* a){throw a[66];}
int main(){try{middle(1,2,3,4,5,6,7,8);}catch(long x){return x==102?0:1;}return 2;}''')
case('terminate', '''extern "C" void fail(); void boundary() noexcept { fail(); }''',
'''#include <exception>
#include <cstdlib>
void boundary(); extern "C" void fail(){throw 8;}
void stop(){std::_Exit(17);} int main(){std::set_terminate(stop);boundary();return 1;}''',17)
case('runtime-alias', '''extern "C" void* memcpy(void*,const void*,unsigned long) noexcept;
int main(){int* x=new int(27); int a[3]={*x,2,3}, b[3]={};
__builtin_memcpy(b,a,sizeof a); memcpy(a,b,sizeof a); delete x;
return a[0]==27 && b[2]==3 ? 0:1;}''','')
case('implicit-cleanup', '''int destroyed;
struct E { ~E(){} }; struct W { E e; }; struct R { bool ok; };
struct Live { ~Live(){++destroyed;} }; struct Needed { Live v; };
W make(){return W();} R use(const W& w){return R{true};}
int main(){for(int i=0;i<3;++i) if(!use(make()).ok) return 1;
{Needed n;} return destroyed==1?0:2;}''','')
classes=''.join('struct T%d { int n; };\n'%i for i in range(140))
handlers=''.join('catch(T%d& x) {return x.n==%d ? 0:2;}\n'%(i,i) for i in range(140))
case('large-actions',classes+'int main(){try {throw T139{139};}'+handlers+'return 1;}','')
# Independent local type tables must map back to their source selector IDs.
case('selectors', '''int one(){try{throw 9;}catch(long){return 1;}catch(int x){return x;}}
int two(){try{throw 17L;}catch(int){return 1;}catch(long x){return x;}}
int main(){return one()+two()==26?0:1;}''','')
# The two object formats are deliberate driver choices, not source-dependent.
source=out/'format.cpp'; source.write_text('int main(){return 0;}')
private=out/'legacy.obj'; elf=out/'explicit.obj'; executable=out/'private-main'
run([compiler,'-c','-o',private,source]); assert private.read_bytes()[:8]==b'CPPGMOBJ'
run([compiler,'-o',executable,private]); run([executable])
run([compiler,'-c','--object-format=elf','-o',elf,source]); assert elf.read_bytes()[:4]==b'\x7fELF'
run(['g++',elf,'-o',out/'host-main']); run([out/'host-main'])
for record in records:
    for stream in ['stdout','stderr']:
        value=record.pop(stream)
        record[stream+'_sha256']=hashlib.sha256(value.encode()).hexdigest()
manifest=dict(compiler_sha256=hashlib.sha256(pathlib.Path(compiler).read_bytes()).hexdigest(),
              cases=9, records=records,
              sources={p.name:hashlib.sha256(p.read_bytes()).hexdigest() for p in out.glob('*.cpp')})
(out/'results.json').write_text(json.dumps(manifest,indent=2)+'\n')
print('9 host ABI/format controls passed')
