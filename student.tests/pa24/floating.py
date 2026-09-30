#!/usr/bin/env python3
"""Independent numerical and ABI checks; no course/reference output is consumed."""
from decimal import Decimal, getcontext
getcontext().prec=100
import math, pathlib, struct, subprocess, tempfile
root=pathlib.Path(__file__).resolve().parents[2]
cc=root/'dev/lowir2native'
checks=0
with tempfile.TemporaryDirectory(prefix='pa24-floating-') as d:
 d=pathlib.Path(d)
 def run(name, source):
  global checks
  path=d/(name+'.lowir'); path.write_text(source)
  exe=d/name
  subprocess.run([str(cc),'-o',str(exe),str(path)],check=True)
  result=subprocess.run([str(exe)],timeout=10)
  assert result.returncode==0, (name,result.returncode,source)
  checks+=1
 def program(body, helpers='', slots=''):
  return helpers+'\nfunction @main() -> i64 [role=entry] {\n'+slots+'\nblock ^entry:\n'+body+'\n}\n'
 def check_expr(body, typ, value, expected):
  return body+f'\n%bad = cmp ne {typ} {value}, {expected}\nreturn i64 %bad'
 for t in ['f32','f64','f80']:
  for x in ['0.0','-0.0','1.0','-1.0','nan']:
   exp=int(float(x)==0)
   run(f'{t}-not-{x}',program(check_expr(f'%x = unary not {t} {x}',t,'%x',float(exp))))
   run(f'{t}-not-branch-{x}',program(f'%x = unary not {t} {x}\nbranch %x, ^yes, ^no\nblock ^yes:\nreturn i64 {1-exp}\nblock ^no:\nreturn i64 {exp}'))
   run(f'{t}-truth-{x}',program(f'%x = const {t} {x}\nbranch %x, ^yes, ^no\nblock ^yes:\nreturn i64 {exp}\nblock ^no:\nreturn i64 {1-exp}'))
  for a,b in [(1.5,2.25),(-7.0,2.0),(8.0,-0.5),(-3.5,-2.0),(0.0,1.0)]:
   for op,fn in [('add',lambda a,b:a+b),('sub',lambda a,b:a-b),('mul',lambda a,b:a*b),('div',lambda a,b:a/b)]:
    run(f'{t}-{op}-{a}-{b}',program(check_expr(f'%x = binary {op} {t} {a}, {b}',t,'%x',fn(Decimal(a),Decimal(b)) if t=='f80' else fn(a,b))))
  for a in [-2.0,0.0,2.0,math.inf,-math.inf,math.nan]:
   for b in [-1.0,0.0,1.0,math.inf,math.nan]:
    for pred,fn in [('eq',lambda a,b:a==b),('ne',lambda a,b:a!=b),('lt',lambda a,b:a<b),('le',lambda a,b:a<=b),('gt',lambda a,b:a>b),('ge',lambda a,b:a>=b)]:
     exp=int(fn(a,b)); text=f'%x = cmp {pred} {t} {a}, {b}'
     run(f'{t}-{pred}-{a}-{b}',program(check_expr(text,'i64','%x',exp)))
     run(f'{t}-branch-{pred}-{a}-{b}',program(text+f'\nbranch %x, ^yes, ^no\nblock ^yes:\nreturn i64 {1-exp}\nblock ^no:\nreturn i64 {exp}'))
  for n in [-9223372036854775808,-1001,-38,0,42,1024,9223372036854774784]:
   # all f64/f80 input values are exact; f32 expectations are rounded in Python.
   fp=float(n)
   if t=='f32': fp=struct.unpack('f',struct.pack('f',fp))[0]
   run(f'{t}-signed-{n}',program(check_expr(f'%x = convert sitofp {t} i64 {n}',t,'%x',str(n)+'.0' if t=='f80' else fp)))
  for n in [0,42,9223372036854775808,18446744073709549568,18446744073709551615]:
   expected=n if t=='f80' else float(n)
   if t=='f32': expected=struct.unpack('f',struct.pack('f',expected))[0]
   run(f'{t}-unsigned-{n}',program(check_expr(f'%x = convert uitofp {t} i64 {n}',t,'%x',str(expected)+('.0' if t=='f80' else ''))))
  for text,expected in [('42.75',42),('0.5',0),('9223372036854775808.0',9223372036854775808),('18446744073709549568.0',18446744073709549568)]:
   if t=='f32' and expected>2**63: continue
   run(f'{t}-uint-{expected}',program(check_expr(f'%x = convert fptoui i64 {t} {text}','i64','%x',expected)))
  for source in ['i8','i16','i32','i64']:
   run(f'{t}-narrow-{source}',program(check_expr(f'%x = convert sitofp {t} {source} -38',t,'%x','-38.0')))
  for target in ['f32','f64','f80']:
   if target==t: continue
   op='fpext' if ['f32','f64','f80'].index(t)<['f32','f64','f80'].index(target) else 'fptrunc'
   run(f'{t}-{target}',program(check_expr(f'%x = convert {op} {target} {t} -1.5',target,'%x','-1.5')))
  # Incoming values, not constants, exercise parallel XMM call moves and cycles.
  params=', '.join(f'%p{k} : {t}' for k in range(10))
  check='\n'.join(f'%c{k} = cmp ne {t} %p{k}, {float((k+3)%10)}' for k in range(10))
  check+='\n%s0 = binary or i64 %c0, %c1\n'
  for k in range(2,10): check+=f'%s{k-1} = binary or i64 %s{k-2}, %c{k}\n'
  helper=f'function @check({params}) -> i64 {{\nblock ^entry:\n{check}return i64 %s8\n}}\n'
  helper+=f'function @forward({params}) -> i64 {{\nblock ^entry:\n%r = call i64 @check('+', '.join(f'%p{(k+3)%10}' for k in range(10))+')\nreturn i64 %r\n}\n'
  run(f'{t}-parallel-stack',program('%r = call i64 @forward('+', '.join(str(float(k)) for k in range(10))+')\nreturn i64 %r',helper))
  # Call-crossing values and parallel phi edges.
  helper=f'function @identity(%x : {t}) -> {t} {{\nblock ^entry:\nreturn {t} %x\n}}\n'
  if t!='f80': run(f'{t}-phi-call',program(f'%a = call {t} @identity(1.5)\n%b = call {t} @identity(2.5)\njump ^loop\nblock ^loop:\n%x = phi {t} [^entry: %a, ^back: %y]\n%y = phi {t} [^entry: %b, ^back: %x]\n%n = phi i64 [^entry: 0, ^back: %next]\n%next = binary add i64 %n, 1\n%c = cmp lt i64 %n, 3\nbranch %c, ^back, ^done\nblock ^back:\njump ^loop\nblock ^done:\n'+check_expr('',t,'%x','2.5'),helper))
  run(f'{t}-call-live',program(f'%a = call {t} @identity(1.5)\n%b = call {t} @identity(2.5)\n%x = binary add {t} %a, %b\n'+check_expr('',t,'%x','4.0'),helper))
 # Mix GPR, XMM and stack variadic arguments, including eightbyte overflow.
 helper='function @va(%tag : i64) -> i64 [arity=variadic] {\nslot $ap : obj<24x8>\nblock ^entry:\n%p = addr $ap\nva_start %p\n'
 for k in range(10):
  helper+=f'%i{k} = va_arg i64 %p\n%f{k} = va_arg f64 %p\n%c{k} = cmp ne i64 %i{k}, {k+1}\n%d{k} = cmp ne f64 %f{k}, {k+1}.5\n%e{k} = binary or i64 %c{k}, %d{k}\n'
  if k: helper+=f'%s{k} = binary or i64 %'+('e0' if k==1 else f's{k-1}')+f', %e{k}\n'
 helper+='return i64 %s9\n}\n'
 run('variadic-mixed-overflow',program('%r = call i64 @va(0, '+', '.join(f'{k+1}, {k+1}.5' for k in range(10))+')\nreturn i64 %r',helper))
print(f'{checks} independent floating numerical, comparison, conversion, phi and ABI programs passed')
