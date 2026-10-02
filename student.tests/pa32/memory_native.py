#!/usr/bin/env python3
"""Actual bulk encoder clobbers: GP/FP live carriers at every size threshold."""
import pathlib,subprocess,tempfile
root=pathlib.Path(__file__).resolve().parents[2]
def run(*args):
 r=subprocess.run(list(map(str,args)),capture_output=True,text=True,timeout=90)
 assert r.returncode==0,(args,r.returncode,r.stderr[-4000:])
 return r
functions=[];calls=[]
for size in [1,2,3,7,8,15,16,17,24,31,32,33,48,64,65,128]:
 for align in [1,8]:
  name='bulk'+str(len(functions))
  functions.append(f'''function @{name}(%p : ptr, %q : ptr, %a : i64, %b : i64, %c : i64, %d : i64, %x : f64, %y : f64) -> i64 [binding=strong, no_inline=yes] {{
block ^entry:
 %before = load i64 %p
 %gp = binary add i64 %a, %b
 %fp = binary add f64 %x, %y
 copyobj {size}x{align} %p, %q
 %after = load i64 %q
 %fpint = convert fptosi i64 f64 %fp
 %v0 = binary add i64 %before, %after
 %v1 = binary add i64 %v0, %gp
 %v2 = binary add i64 %v1, %c
 %v3 = binary add i64 %v2, %d
 %v4 = binary add i64 %v3, %fpint
 return i64 %v4 }}''')
  calls.append(name)
main=['function @main() -> i64 [role=entry] {','slot $p : obj<160x8>','slot $q : obj<160x8>','block ^entry:','%p = addr $p','%q = addr $q','zeroinit 160x8 %p','store i64 100, %p','%bad0 = const i64 0']
for n,name in enumerate(calls):main += ['zeroinit 160x8 %q',f'%r{n} = call i64 @{name}(%p, %q, 1, 2, 3, 4, 1.25, 2.75)',f'%b{n} = cmp ne i64 %r{n}, 214',f'%bad{n+1} = binary or i64 %bad{n}, %b{n}']
main += [f'return i64 %bad{len(calls)}','}']
with tempfile.TemporaryDirectory(prefix='pa32-memory-native-') as directory:
 tmp=pathlib.Path(directory);source=tmp/'input.lowir';source.write_text('\n'.join(functions+main))
 for level in range(4):
  out=tmp/f'o{level}.lowir';run(root/'dev/lowiropt',f'-O{level}','-o',out,source)
  run(root/'dev/lowir','-o',tmp/'valid.lowir',out)
  run(root/'dev/lowir2native','-o',tmp/'native',out);run(tmp/'native')
  run(root/'dev/cppgm++',f'-O{level}','-o',tmp/'direct',source);run(tmp/'direct')
  run(root/'dev/cppgm++','-O0','-o',tmp/'replay',out);run(tmp/'replay')
print('PA32 bulk clobbers PASS: 32 size/alignment boundaries with six GP and two FP arguments, live GP/FP results, four levels and three execution paths')
