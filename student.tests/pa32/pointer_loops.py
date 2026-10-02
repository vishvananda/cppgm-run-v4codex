#!/usr/bin/env python3
"""Finite pointer inductions, exported truths/twins, and congruence guards."""
import pathlib,subprocess,tempfile
root=pathlib.Path(__file__).resolve().parents[2]
def run(*args):
 p=subprocess.run(list(map(str,args)),capture_output=True,text=True,timeout=60)
 assert p.returncode==0,(args,p.returncode,p.stderr[-4000:]);return p
functions=[];checks=[]
for step in [-3,-1,1,3,8]:
 for inverted in [False,True]:
  name=f'walk{len(functions)}'
  functions.append(f'''function @{name}(%first : ptr, %last : ptr) -> i64 [no_inline=yes] {{
block ^entry: jump ^head
block ^head:
 %p = phi ptr [^entry: %first, ^body: %next]
 %twin = phi ptr [^entry: %first, ^body: %next]
 %more = cmp {'eq' if inverted else 'ne'} ptr %last, %twin
 branch %more, {'^done, ^body' if inverted else '^body, ^done'}
block ^body:
 %next = index i8 %p, {step}
 jump ^head
block ^done:
 %equal = cmp eq ptr %p, %last
 %truth = cmp eq i64 %more, {int(inverted)}
 %ok = binary and i64 %equal, %truth
 return i64 %ok
}}
''')
  for trips in [0,1,4,7]:checks.append((name,40 if step<0 else 0,(40 if step<0 else 0)+step*trips))
functions.append((root/'pa32/tests/o1/500-effect-free-loop-deleted.t').read_text())
# This load cannot be retired using an exit write through a different address.
functions.append(functions[-1].replace('@destruct_at_end','@other_address').replace('store ptr %new_last, %end_slot','store ptr %new_last, %buffer'))
# A missing stride congruence guarantee must not turn nontermination into return.
even=(root/'pa32/tests/o1/500-effect-free-loop-deleted-twin-backward.t').read_text()
functions.append(even)
with tempfile.TemporaryDirectory(prefix='pa32-pointer-') as tmp:
 d=pathlib.Path(tmp);src=d/'input.lowir'
 main=['function @main() -> i32 [role=entry] {slot $bytes : obj<64x8>','slot $vector : obj<16x8>','block ^entry:','%base = addr $bytes']
 for n,(name,start,last) in enumerate(checks):
  main += [f'%s{n} = index i8 %base, {start}',f'%e{n} = index i8 %base, {last}',f'%c{n} = call i64 @{name}(%s{n}, %e{n})']
  if n:main += [f'%ok{n} = binary and i64 '+('%c0' if n==1 else f'%ok{n-1}')+f', %c{n}']
 main += ['%vector = addr $vector','%field = index i8 %vector, 8','%last = index i8 %base, 10','store ptr %last, %field','call void @destruct_at_end(%vector, %base)','%value = load ptr %field','%eq = cmp eq ptr %value, %base',f'%all = binary and i64 %ok{len(checks)-1}, %eq','%bad = cmp eq i64 %all, 0','%result = convert trunc i32 i64 %bad','return i32 %result','}']
 src.write_text(''.join(functions)+'\n'.join(main))
 for level in range(4):
  opt=d/'out.lowir';run(root/'dev/lowiropt',f'-O{level}','-o',opt,src)
  if level:
   text=opt.read_text()
   for i in range(10):
    body=text.split(f'function @walk{i}(')[1].split('\n}')[0]
    assert ('phi ' in body)==(i>=8),(i,body)
   body=text.split('function @destruct_at_end(')[1].split('\n}')[0]
   assert 'load ' not in body and 'phi ' not in body,body
   body=text.split('function @other_address(')[1].split('\n}')[0]
   assert 'load ' in body and 'phi ' not in body,body
   body=text.split('function @destroy_backward(')[1].split('\n}')[0]
   assert 'phi ' in body,body
  for path in ['original','replay','native']:
   exe=d/f'{path}{level}'
   if path=='native':run(root/'dev/lowir2native','-o',exe,opt)
   else:
    obj=d/'out.o';run(root/'dev/cppgm++','-c',f'-O{level}' if path=='original' else '-O0','-o',obj,src if path=='original' else opt)
    run('g++',obj,'-o',exe)
   run(exe)
 print(f'pointer loops: {len(checks)+1} checked cases x four levels x three paths; even-stride/memory guards PASS')
