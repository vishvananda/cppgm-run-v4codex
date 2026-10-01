#!/usr/bin/env python3
"""Assembly recipe semantics, templates, operand effects, native/IR and rejection controls."""
import hashlib,json,pathlib,subprocess,sys
root=pathlib.Path(__file__).resolve().parents[2]
out=pathlib.Path(sys.argv[1]).resolve();out.mkdir(parents=True,exist_ok=True)
cc=pathlib.Path(sys.argv[2]).resolve() if len(sys.argv)>2 else root/'dev/cppgm++'
rows=[]
def run(name,args,ok=True):
 p=subprocess.run(list(map(str,args)),cwd=root,capture_output=True,timeout=90)
 passed=(p.returncode==0)==ok
 rows.append(dict(name=name,args=list(map(str,args)),status=p.returncode,passed=passed,stdout=p.stdout.decode(errors='replace'),stderr=p.stderr.decode(errors='replace')))
 if not passed:print(name,p.returncode,p.stderr.decode(),flush=True)
 return passed
for name in ['assembly','assembly-lifetimes','assembly-thread']:
 src=root/'student.tests/pa29/controls163'/(name+'.cpp');obj=out/(name+'.o');exe=out/name
 host=[src.parent/'thread-host.cpp','-pthread'] if name=='assembly-thread' else []
 if run(name+' compile',[cc,'-O0','-c',src,'-o',obj]):
  if run(name+' link',['g++',obj,*host,'-o',exe]):run(name+' runtime',[exe])
  if run(name+' stats',[cc,'--stats','-c',src,'-o',out/'stats.o']):
   rows.append(dict(name=name+' telemetry equality',passed=obj.read_bytes()==(out/'stats.o').read_bytes()))
  low=out/(name+'.lowir');rt=out/(name+'.roundtrip')
  if run(name+' IR',[cc,'-c','--emit-lowir','--validate-lowir',src,'-o',low]):
   if run(name+' roundtrip',[root/'dev/lowir',low,'-o',rt]):
    rows.append(dict(name=name+' IR equality',passed=low.read_bytes()==rt.read_bytes()))
   if name=='assembly':
    if run(name+' MIR',[root/'dev/lowir2native','--dump-machine-ir',out/'assembly.mir',low,'-o',out/'native']):run(name+' native runtime',[out/'native'])
    run(name+' disassembly',['objdump','-drC',obj])
reject={
 'unknown-operand':'int x=0;asm("incl %1":"+r"(x));',
 'duplicate-name':'int x=0;asm("incl %[x]":[x]"+r"(x),[x]"+r"(x));',
 'write-input':'int x=0;asm("incl %0"::"r"(x));',
 'const-output':'const int x=0;asm("incl %0":"+r"(x));',
 'rvalue-output':'asm("incl %0":"+r"(1));',
 'rvalue-memory':'asm("nop"::"m"(1));',
 'uninitialized':'int x;asm("incl %0":"=r"(x));',
 'missing-constraint':'int x=0;asm("incl %0":"+"(x));',
 'bad-match':'int x=0;asm("incl %0":"=r"(x):"4"(x));',
 'bad-width':'long x=0;asm("incl %0":"+r"(x));',
 'bad-lock':'int x=0;asm("lock incl %0":"+r"(x));',
 'lock-move':'int x=0;asm("lock movl $1,%0":"=m"(x));',
 'lock-hint':'asm("lock nop");',
 'immediate-nonconstant':'int x=0;asm("addl %1,%0":"+r"(x):"i"(x));',
 'memory-pair':'int x=0,y=1;asm("movl %1,%0":"=m"(x):"m"(y));',
 'unwritten-exchange':'int x=0,y=1;asm("xchgl %1,%0":"+m"(x):"r"(y));',
 'narrow-bswap':'short x=0;asm("bswap %0":"+r"(x));',
 'float':'double x=0;asm("bswap %0":"+r"(x));',
 'trailing':'asm("nop junk");',
 'unknown-op':'asm("unsupported_instruction");',
 'bad-clobber':'asm("nop":::"unknown_clobber");',
 'ordinary-lookup':'asm("incl %0":"+r"(missing));',
 'embedded-null':'asm("nop\\0pause");',
}
for name,body in reject.items():
 src=out/(name+'.cpp');src.write_text('int main(){'+body+'}\n');run(name,[cc,'-c',src,'-o',out/'reject.o'],False)
for name,source in {
 'template-lookup':'template<class T>void f(T x){asm("addl %1,%0":"+r"(x):"r"(missing));}int main(){}',
 'template-const':'template<class T>void f(T& x){asm("incl %0":"+r"(x));}int main(){const int x=1;f(x);}',
 'constant-evaluation':'constexpr int f(){asm("nop");return 1;} static_assert(f()==1,"");',
}.items():
 src=out/(name+'.cpp');src.write_text(source+'\n');run(name,[cc,'-c',src,'-o',out/'reject.o'],False)
(out/'results.json').write_text(json.dumps(dict(compiler_sha256=hashlib.sha256(cc.read_bytes()).hexdigest(),checks=rows),indent=2)+'\n')
print('%d/%d checks passed'%(sum(r['passed'] for r in rows),len(rows)),flush=True)
sys.exit(any(not r['passed'] for r in rows))
