#!/usr/bin/env python3
"""Seeded independent finite-domain interpreter versus optimized affine loops."""
import pathlib,random,subprocess,tempfile
ROOT=pathlib.Path(__file__).resolve().parents[2]
def run(*args):
 r=subprocess.run(list(map(str,args)),capture_output=True,text=True,timeout=60)
 assert r.returncode==0,(args,r.returncode,r.stderr[-2000:]);return r
rng=random.Random(212)
functions=[];expected=[];guards=[]
ops=['eq','ne','lt','le','gt','ge','ult','ule','ugt','uge']
for n in range(500):
 width=rng.choice([8,16,32,64]);ty='i'+str(width);mask=(1<<width)-1;sign=1<<(width-1)
 def signed(v):return v-(1<<width) if v&sign else v
 values=[0,1,2,3,4,sign-4,sign-1,sign,sign+1,mask-3,mask]
 start=rng.choice(values);limit=rng.choice(values);step=rng.choice([-3,-2,-1,0,1,2,3]);op=rng.choice(ops)
 def compare(a,b):
  if op[0]!='u':a,b=signed(a),signed(b)
  return {'eq':a==b,'ne':a!=b,'lt':a<b,'le':a<=b,'gt':a>b,'ge':a>=b}[op[1:] if op[0]=='u' else op]
 v=start;count=0
 while count<300 and compare(v,limit):v=(v+step)&mask;count+=1
 name='f'+str(n)
 body=f'''function @{name}() -> i64 [no_inline=yes] {{
block ^entry: jump ^head
block ^head:
 %i = phi {ty} [^entry: {signed(start)}, ^body: %next]
 %c = cmp {op} {ty} %i, {signed(limit)}
 branch %c, ^body, ^exit
block ^body:
 %next = binary add {ty} %i, {step}
 jump ^head
block ^exit:
 %r = copy i64 %i
 return i64 %r
}}'''
 if count==300:guards.append((name,body));continue
 functions.append(body);expected.append((name,signed(v)))
main=['function @main() -> i32 [role=entry] {block ^entry:']
for n,(name,value) in enumerate(expected):
 main += [f'%v{n} = call i64 @{name}()',f'%c{n} = cmp eq i64 %v{n}, {value}']
 if n:main.append(f'%ok{n} = binary and i64 '+('%c0' if n==1 else f'%ok{n-1}')+f', %c{n}')
main += [f'%bad = cmp eq i64 %ok{len(expected)-1}, 0','%r = convert trunc i32 i64 %bad','return i32 %r','}']
with tempfile.TemporaryDirectory(prefix='pa32-trip-properties-') as tmp:
 d=pathlib.Path(tmp);source=d/'finite.lowir';source.write_text('\n'.join(functions+main))
 for level in range(4):
  ir=d/f'o{level}.lowir';run(ROOT/'dev/lowiropt',f'-O{level}','-o',ir,source)
  exe=d/f'e{level}';run(ROOT/'dev/lowir2native','-o',exe,ir);run(exe)
 # Unknown large counts include genuine infinite loops. Don't execute them;
 # finite proofs may erase some, but literal-false exit guards cannot disappear.
 source=d/'guards.lowir';source.write_text('\n'.join(body for _,body in guards))
 run(ROOT/'dev/lowiropt','-O3','-o',d/'guards-out.lowir',source)
 run(ROOT/'dev/cppgm++','-c','-O0','-o',d/'guards.o',d/'guards-out.lowir')
print(f'{len(expected)} finite-domain interpreter cases x four levels PASS; {len(guards)} long/infinite candidates remain valid')
