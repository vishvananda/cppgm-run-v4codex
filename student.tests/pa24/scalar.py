#!/usr/bin/env python3
"""Deterministic differential integer cases; no reference compiler is used."""
import pathlib, random, subprocess, tempfile, sys
root = pathlib.Path(__file__).resolve().parents[2]
compiler = pathlib.Path(sys.argv[1]).resolve() if len(sys.argv)>1 else root/'dev/lowir2native'
rng = random.Random(240127)
types = [('i8',8,True),('u8',8,False),('i16',16,True),('u16',16,False),('i32',32,True),('u32',32,False),('i64',64,True)]
def norm(n,w,s):
    n &= (1<<w)-1
    return n-(1<<w) if s and n>>(w-1) else n
cases=[]
for t,w,s in types:
    for n in range(20):
        a=norm(rng.getrandbits(w),w,s); b=norm(rng.getrandbits(w),w,s)
        for op in ['add','sub','mul','and','or','xor','div','mod','udiv','umod','shl','shr','ushr']:
            k=b
            if op in ('shl','shr','ushr'): k=rng.randrange(w)
            sa=norm(a,w,True); sb=norm(k,w,True)
            ua=a&((1<<w)-1); ub=k&((1<<w)-1)
            if op in ('div','mod') and (not sb or (sa==-(1<<(w-1)) and sb==-1)): continue
            if op in ('udiv','umod') and not ub: continue
            q=(abs(sa)//abs(sb))*(-1 if (sa<0)!=(sb<0) else 1) if sb else 0
            result={'add':lambda:a+k,'sub':lambda:a-k,'mul':lambda:a*k,'and':lambda:a&k,'or':lambda:a|k,'xor':lambda:a^k,
                    'div':lambda:q,'mod':lambda:sa-q*sb,'udiv':lambda:ua//ub,'umod':lambda:ua%ub,
                    'shl':lambda:a<<k,'shr':lambda:sa>>k,'ushr':lambda:ua>>k}[op]()
            cases.append((t,op,a,k,norm(result,w,s)))
with tempfile.TemporaryDirectory(prefix='pa24-scalar-') as d:
    d=pathlib.Path(d)
    for batch in range(0,len(cases),100):
        rows=['function @main() -> i64 [role=entry] {']
        for j,(t,op,a,b,result) in enumerate(cases[batch:batch+100]):
            rows += [f'block ^b{j}:',f'%v{j} = binary {op} {t} {a}, {b}',f'%bad{j} = cmp ne {t} %v{j}, {result}',f'branch %bad{j}, ^fail{j}, ^b{j+1}',f'block ^fail{j}:',f'return i64 {j+1}']
        rows+= [f'block ^b{j+1}:','return i64 0','}']
        source=d/'case.lowir'; source.write_text('\n'.join(rows)+'\n')
        exe=d/'case'
        subprocess.run([str(compiler),'-o',str(exe),str(source)],check=True)
        result=subprocess.run([str(exe)])
        if result.returncode:
            case=cases[batch+result.returncode-1] if 0<result.returncode<=100 else result.returncode
            raise AssertionError(f'batch {batch}: {case}')
print(f'{len(cases)} checked integer arithmetic cases passed')
