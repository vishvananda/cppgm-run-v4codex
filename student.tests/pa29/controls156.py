#!/usr/bin/env python3
"""Explicit PA29 scalar/runtime builtin controls, including independent numeric checks."""
import hashlib,json,pathlib,subprocess,sys
root=pathlib.Path(__file__).resolve().parents[2]
cc=root/'dev/cppgm++'
work=pathlib.Path(sys.argv[1] if len(sys.argv)>1 else '/tmp/pa29-156/controls').resolve();work.mkdir(parents=True,exist_ok=True)
rows=[]
def run(name,args,ok=True):
 p=subprocess.run(list(map(str,args)),capture_output=True,timeout=90,cwd=root)
 good=(p.returncode==0)==ok
 rows.append(dict(name=name,args=list(map(str,args)),status=p.returncode,passed=good,stdout=p.stdout.decode(errors='replace'),stderr=p.stderr.decode(errors='replace')))
 if not good: print(name,p.returncode,p.stderr.decode(errors='replace')[-1000:],flush=True)
 return good
for src in sorted((root/'student.tests/pa29/controls156').glob('*.cpp')):
 obj=work/(src.stem+'.o');exe=work/src.stem
 if run(src.stem+' compile',[cc,'-c',src,'-o',obj]):
  if run(src.stem+' link',['g++',obj,'-lm','-o',exe]):run(src.stem+' execute',[exe])
negative=['__builtin_clz()', '__builtin_clz(1,2)', '__builtin_bswap64(1,2)',
 '__builtin_ctzg(1u,2,3)', '__builtin_popcountg(1u,2)', '__builtin_clzg(1)',
 '__builtin_popcountg(true)', '__builtin_sqrt(1,2)', '__builtin_frexp(1.0,3)',
 '__builtin_remquo(1.0,2.0)', '__builtin_modf(1.0,(int*)0)', '__builtin_memcpy(0,0)',
 '__builtin_prefetch()', 'wrong::__builtin_sqrt(4.0)']
for i,expression in enumerate(negative):
 src=work/('bad%d.cpp'%i);src.write_text('namespace wrong {}\nint main(){'+expression+';}\n')
 run(expression,[cc,'-c',src,'-o',work/'bad.o'],False)
for n in ['__builtin_clz(0u)','__builtin_ctzg(0u)']:
 src=work/'badconstant.cpp';src.write_text('constexpr int x='+n+';\n')
 run('not constant '+n,[cc,'-c',src,'-o',work/'bad.o'],False)
# Python arbitrary-precision arithmetic supplies an independent overflow oracle.
import random
rng=random.Random(156)
kinds=[('signed char',8,True),('unsigned char',8,False),('long long',64,True),('unsigned long long',64,False),('__int128',128,True),('unsigned __int128',128,False)]
def literal(v,t):
 bits=v%2**128
 return '('+t+')(((unsigned __int128)'+str(bits>>64)+'ULL<<64)|'+str(bits%(2**64))+'ULL)'
checks=[]
for operation in ['add','sub','mul']:
 for ta,wa,sa in kinds:
  for tb,wb,sb in kinds:
   for tr,wr,sr in kinds:
    def samples(w,sign):
     low=-(2**(w-1)) if sign else 0;high=2**(w-int(sign))-1
     return [low,high,rng.randint(low,high)]
    for av,bv in zip(samples(wa,sa),samples(wb,sb)):
     value=av+bv if operation=='add' else av-bv if operation=='sub' else av*bv
     low=-(2**(wr-1)) if sr else 0;high=2**(wr-int(sr))-1
     overflow=value<low or value>high
     checks.append('if(!check_'+operation+'<'+ta+','+tb+','+tr+'>('+literal(av,ta)+','+literal(bv,tb)+','+literal(value,tr)+','+str(overflow).lower()+'))return '+str(len(checks)%250+1)+';')
source=''.join('template<class A,class B,class R> bool check_'+op+'(A a,B b,R expected,bool overflow){R result=0; bool actual=__builtin_'+op+'_overflow(a,b,&result);return actual==overflow && result==expected;}\n' for op in ['add','sub','mul'])
source+='int main(){\n'+'\n'.join(checks)+'\nreturn 0;}\n'
p=work/'overflow.cpp';p.write_text(source)
if run('overflow matrix compile',[cc,'-c',p,'-o',work/'overflow.o']):
 if run('overflow matrix link',['g++',work/'overflow.o','-o',work/'overflow']):run('overflow matrix execute '+str(len(checks))+' cases',[work/'overflow'])
for expression in ['__builtin_add_overflow(1,2,(bool*)0)','__builtin_sub_overflow(1.0,2,(int*)0)','__builtin_mul_overflow(1,2,(const int*)0)','__builtin_add_overflow(1,2)', '__builtin_prefetch((int*)0,2)','__builtin_prefetch((int*)0,0,4)']:
 p=work/'badextension.cpp';p.write_text('int main(){'+expression+';}\n')
 run('invalid '+expression,[cc,'-c',p,'-o',work/'bad.o'],False)
(work/'results.json').write_text(json.dumps(dict(compiler_sha256=hashlib.sha256(cc.read_bytes()).hexdigest(),checks=rows),indent=2)+'\n')
print('%d/%d explicit checks passed'%(sum(r['passed'] for r in rows),len(rows)),flush=True)
sys.exit(any(not r['passed'] for r in rows))
