#!/usr/bin/env python3
"""Object byte preservation across ABI rollback, indirect calls and padded tails."""
import pathlib, subprocess, tempfile
root=pathlib.Path(__file__).resolve().parents[2]; cc=root/'dev/lowir2native'; checks=0
with tempfile.TemporaryDirectory(prefix='pa24-objects-') as directory:
 d=pathlib.Path(directory)
 for size in [1,2,3,4,5,7,8,9,10,11,12,13,15,16,17,24,32,40,64,112]:
  for gp in range(8):
   t=f'obj<{size}x1>'
   params=', '.join([*(f'%p{k} : i64' for k in range(gp)),f'%object : {t}','%tail : i64'])
   args=', '.join([*(str(k) for k in range(gp)),'$object','37'])
   source=f'function @identity({params}) -> {t} {{\nblock ^entry:\nreturn {t} %object\n}}\n'
   source+=f'function @main() -> i64 [role=entry] {{\nslot $object : {t}\nslot $out : {t}\nblock ^entry:\n%base = addr $object\n'
   for k in range(size): source+=f'%p{k} = index i8 %base, {k}\nstore u8 {(k*17+3)%256}, %p{k}\n'
   source+=f'%fn = addr @identity\n%result = call {t} %fn({args}) as ({params}) -> {t}\n%out = addr $out\ncopyobj {size}x1 %result, %out\n'
   for k in range(size):
    source+=f'%q{k} = index i8 %out, {k}\n%v{k} = load u8 %q{k}\n%c{k} = cmp ne u8 %v{k}, {(k*17+3)%256}\n'
    if k: source+=f'%s{k} = binary or i64 %'+('c0' if k==1 else f's{k-1}')+f', %c{k}\n'
   source+=f'return i64 %'+('c0' if size==1 else f's{size-1}')+'\n}\n'
   path=d/'input.lowir'; exe=d/'program'; path.write_text(source)
   c=subprocess.run([str(cc),'-o',str(exe),str(path)],capture_output=True,text=True)
   assert c.returncode==0,(size,gp,c.stderr,source)
   r=subprocess.run([str(exe)],timeout=10)
   assert r.returncode==0,(size,gp,r.returncode,source)
   checks+=1
print(f'{checks} independent object ABI programs passed')
