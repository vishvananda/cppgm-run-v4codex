#!/usr/bin/env python3
"""Independent two-word payload, arithmetic, comparison and ABI tests."""
import pathlib, random, subprocess, tempfile
root=pathlib.Path(__file__).resolve().parents[2]
cc=root/'dev/lowir2native'; rng=random.Random(129); mask=(1<<128)-1; checks=0
with tempfile.TemporaryDirectory(prefix='pa24-wide-') as directory:
 d=pathlib.Path(directory)
 def run(name,source):
  global checks
  path=d/'input.lowir'; exe=d/'program'; path.write_text(source)
  c=subprocess.run([str(cc),'-o',str(exe),str(path)],capture_output=True,text=True)
  assert c.returncode==0,(name,c.stderr,source)
  r=subprocess.run([str(exe)],timeout=10)
  assert r.returncode==0,(name,r.returncode,source)
  checks+=1
 def main(body,extra='',slots=''):
  return extra+'\nfunction @main() -> i64 [role=entry] {\n'+slots+'\nblock ^entry:\n'+body+'\n}\n'
 def signed(x): return x-(1<<128) if x>>127 else x
 values=[0,1,mask,1<<64,(1<<64)-1,1<<127,(1<<127)-1]+[rng.getrandbits(128) for _ in range(15)]
 for k,a in enumerate(values):
  b=values[(k*7+3)%len(values)]
  extra=f'global @a : i128 = {a}\nglobal @b : i128 = {b}\n'
  inputs='%a = load i128 @a\n%b = load i128 @b\n'
  for op,expected in [('add',a+b),('sub',a-b),('mul',a*b),('and',a&b),('or',a|b),('xor',a^b)]:
   run(f'{op}-{k}',main(inputs+f'%x = binary {op} i128 %a, %b\n%bad = cmp ne i128 %x, {expected&mask}\nreturn i64 %bad',extra))
  for op,expected in [('neg',-a),('bitnot',~a)]:
   run(f'{op}-{k}',main(inputs+f'%x = unary {op} i128 %a\n%bad = cmp ne i128 %x, {expected&mask}\nreturn i64 %bad',extra))
  for pred,truth in [('eq',a==b),('ne',a!=b),('lt',signed(a)<signed(b)),('le',signed(a)<=signed(b)),('gt',signed(a)>signed(b)),('ge',signed(a)>=signed(b)),('ult',a<b),('ule',a<=b),('ugt',a>b),('uge',a>=b)]:
   run(f'{pred}-value-{k}',main(inputs+f'%x = cmp {pred} i128 %a, %b\n%bad = cmp ne i64 %x, {int(truth)}\nreturn i64 %bad',extra))
   run(f'{pred}-branch-{k}',main(inputs+f'%x = cmp {pred} i128 %a, %b\nbranch %x, ^yes, ^no\nblock ^yes:\nreturn i64 {int(not truth)}\nblock ^no:\nreturn i64 {int(truth)}',extra))
  for gp in range(8):
   params=', '.join([*(f'%p{n} : i64' for n in range(gp)),'%wide : i128','%tail : i64'])
   args=', '.join([*(str(n) for n in range(gp)),str(a),'37'])
   extra=f'function @forward({params}) -> i128 {{\nblock ^entry:\nreturn i128 %wide\n}}\n'
   run(f'abi-{k}-{gp}',main(f'%x = call i128 @forward({args})\n%bad = cmp ne i128 %x, {a}\nreturn i64 %bad',extra))
 for a in values[:7]:
  for n in [0,1,31,63,64,65,95,127]:
   for op,expected in [('shl',a<<n),('ushr',a>>n),('shr',signed(a)>>n)]:
    for variable in [False,True]:
     extra=f'global @a : i128 = {a}\nglobal @n : i128 = {n}\n'
     body='%a = load i128 @a\n'+('%n = load i128 @n\n' if variable else '')
     count='%n' if variable else str(n)
     run(f'{op}-{a}-{n}-{variable}',main(body+f'%x = binary {op} i128 %a, {count}\n%bad = cmp ne i128 %x, {expected&mask}\nreturn i64 %bad',extra))
 for a in values[:7]:
  b=mask-a
  extra=f'global @wide : i128 = {a}\n'
  run(f'atomic-{a}',main(f'''%ptr = addr @wide
%x = atomic_load i128 %ptr, 5
%bad0 = cmp ne i128 %x, {a}
%old = atomic_exchange i128 %ptr, {b}, 5
%bad1 = cmp ne i128 %old, {a}
%added = atomic_add_fetch i128 %ptr, 7, 5
%bad2 = cmp ne i128 %added, {(b+7)&mask}
store i128 0, $expected
%ep = addr $expected
%fail = atomic_compare_exchange i128 %ptr, %ep, 2, 5, 2
%updated = load i128 $expected
%bad3 = cmp ne i128 %updated, {(b+7)&mask}
%ok = atomic_compare_exchange i128 %ptr, %ep, 2, 5, 2
%bad4 = cmp ne i64 %ok, 1
%now = atomic_load i128 %ptr, 5
%bad5 = cmp ne i128 %now, 2
atomic_store i128 {a}, %ptr, 5
%last = atomic_load i128 %ptr, 5
%bad6 = cmp ne i128 %last, {a}
%s0 = binary or i64 %bad0, %bad1
%s1 = binary or i64 %s0, %bad2
%s2 = binary or i64 %s1, %bad3
%s3 = binary or i64 %s2, %bad4
%s4 = binary or i64 %s3, %bad5
%s5 = binary or i64 %s4, %bad6
%s6 = binary or i64 %s5, %fail
return i64 %s6''',extra,'slot $expected : i128'))
 for width in [8,16,32,64]:
  for n in [-1,-(1<<(width-1)),0,1,(1<<(width-1))-1]:
   for op in ['sext','zext']:
    expected=n&mask if op=='sext' else n&((1<<width)-1)
    run(f'{op}-{width}-{n}',main(f'%x = convert {op} i128 i{width} {n}\n%bad = cmp ne i128 %x, {expected}\nreturn i64 %bad'))
print(f'{checks} independent wide numeric and ABI programs passed')
