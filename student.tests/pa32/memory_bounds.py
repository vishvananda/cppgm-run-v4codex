#!/usr/bin/env python3
"""Explicit memory-state caps, scaling, dense-CFG fallback and typed guards."""
import json,pathlib,subprocess,sys,tempfile
root=pathlib.Path(__file__).resolve().parents[2]
records=[]
def run(*args):
 r=subprocess.run(list(map(str,args)),capture_output=True,text=True,timeout=90)
 assert r.returncode==0,(args,r.returncode,r.stderr[-3000:])
 return r
with tempfile.TemporaryDirectory(prefix='pa32-memory-bounds-') as directory:
 tmp=pathlib.Path(directory);source=tmp/'input.lowir';out=tmp/'output.lowir'
 def check(name,text):
  source.write_text(text)
  r=run(root/'dev/lowiropt','-O3','--stats','-o',out,source)
  stats=next(json.loads(s) for s in r.stderr.splitlines() if s.startswith('{') and 'memory_work' in s)
  run(root/'dev/lowir','-o',tmp/'valid.lowir',out)
  run(root/'dev/cppgm++','-c','-O0','-o',tmp/'object.o',out)
  records.append(dict(case=name,stats=stats,input_bytes=len(text),output_bytes=out.stat().st_size))
  return stats,out.read_text()
 for n in [100,500,1000]:
  text=''.join(f'''function @f{k}(%p : ptr) -> i64 [binding=strong] {{
block ^entry: %a = load i64 %p %b = load i64 %p %c = binary add i64 %a, %b return i64 %c }}\n''' for k in range(n))
  stats,_=check('scale-'+str(n),text);assert stats['memory_reused']==n
  assert stats['memory_peak_snapshots']==1
 assert records[0]['stats']['memory_work']*10==records[2]['stats']['memory_work']
 # All 32 tracked cells flow through a complete layered switch graph. Work
 # exhaustion publishes no unfinished intersections; lowering stays valid.
 width,depth=40,10
 text=['function @dense(%p : ptr, %c : i64) -> i64 [binding=strong] {','block ^entry:']
 text += [f'%p{i} = index i8 %p, {8*i} %v{i} = load i64 %p{i}' for i in range(32)]
 def switch(targets):return 'switch %c, ^'+targets[0]+''.join(f', {i}: ^{b}' for i,b in enumerate(targets[1:],1))
 text += [switch([f'b0_{i}' for i in range(width)])]
 for d in range(depth):
  for b in range(width):text += [f'block ^b{d}_{b}:',switch([f'b{d+1}_{i}' for i in range(width)]) if d+1<depth else 'jump ^end']
 text += ['block ^end: %last = load i64 %p return i64 %last','}']
 stats,_=check('dense-fallback','\n'.join(text));assert stats['memory_declined']==1,stats
 # A mutable address must not be made a noalias root. Plain arithmetic cannot
 # inherit a global readonly region or a parameter disjoint-object promise.
 guards='''global @ro : i64 [storage=readonly] = 7
 declare function @unknown() -> void
 function @plain_readonly(%n : i64) -> i64 [binding=strong] {
 block ^entry:
 %p = index i8 @ro, 8
 %a = load i64 %p
 call void @unknown()
 %b = load i64 %p
 %s = binary add i64 %a, %b
 return i64 %s }
 function @volatile_copy(%p : ptr [alias=noalias], %q : ptr [alias=noalias]) -> void [binding=strong] {
 block ^entry:
 %p8 = index i8 [projection=field] %p, 8
 %q8 = index i8 [projection=field] %q, 8
 %a = load volatile i64 %p
 store i64 %a, %q
 %b = load i64 %p8
 store i64 %b, %q8
 return void }
 function @unproven_copy(%p : ptr [alias=noalias], %q : ptr [alias=noalias]) -> void [binding=strong] {
 block ^entry:
 %p8 = index i8 %p, 8
 %q8 = index i8 %q, 8
 %a = load i64 %p
 store i64 %a, %q
 %b = load i64 %p8
 store i64 %b, %q8
 return void }
'''
 stats,text=check('guard-facts',guards)
 assert 'copyobj' not in text
 assert text.split('function @plain_readonly')[1].split('function @volatile_copy')[0].count('load i64')==2
 # Two IR declarations may name one linker object. Distinct textual names
 # are not disjoint-storage evidence; validate and execute the ELF case.
 source.write_text("""global @primary : i64 [object=shared_memory] = 7
 declare global @alias : i64 [object=shared_memory]
 function @main() -> i64 [role=entry] {
 block ^entry:
 %before = load i64 @primary
 store i64 19, @alias
 %after = load i64 @primary
 %sum = binary add i64 %before, %after
 %bad = cmp ne i64 %sum, 26
 return i64 %bad }
 """)
 for level in range(4):
  run(root/'dev/cppgm++',f'-O{level}','-o',tmp/'alias',source);run(tmp/'alias')
 records.append(dict(case='ELF-storage-alias',levels=4))
 # All finite unsigned decrement comparisons, both operand orders, including
 # noncanonical and truncated wider conditions. Expected values are Python's
 # modular arithmetic, never the reference optimizer.
 funcs=[];main=['function @main() -> i64 [role=entry] { block ^entry: %bad0 = const i64 0']
 count=0
 for width,ty,values in [(8,'u8',range(256)),(16,'u16',[0,1,2,255,256,65535]),(32,'u32',[0,1,2,2**31,2**32-1])]:
  for op in ['ult','ule','ugt','uge']:
   for reverse in [False,True]:
    name=f'cmp{width}{op}{int(reverse)}'
    a,b=('%x','%dec') if reverse else ('%dec','%x')
    funcs.append(f'''function @{name}(%x : {ty}) -> i64 [binding=strong, no_inline=yes] {{
block ^entry: %nz = cmp ne {ty} %x, 0 branch %nz, ^check, ^zero
block ^check: %dec = binary sub {ty} %x, 1 %r = cmp {op} {ty} {a}, {b} return i64 %r
block ^zero: %dec0 = binary sub {ty} %x, 1 %r0 = cmp {op} {ty} {a.replace('%dec','%dec0')}, {b.replace('%dec','%dec0')} return i64 %r0 }}''')
    for x in values:
     dec=(x-1)%(1<<width);a,b=(x,dec) if reverse else (dec,x)
     expected=dict(ult=a<b,ule=a<=b,ugt=a>b,uge=a>=b)[op]
     main += [f'%r{count} = call i64 @{name}({x})',f'%b{count} = cmp ne i64 %r{count}, {int(expected)}',f'%bad{count+1} = binary or i64 %bad{count}, %b{count}'];count+=1
 main += [f'return i64 %bad{count}','}']
 source.write_text('\n'.join(funcs+main))
 for level in range(4):
  run(root/'dev/lowiropt',f'-O{level}','-o',out,source)
  run(root/'dev/lowir','-o',tmp/'valid.lowir',out)
  run(root/'dev/lowir2native','-o',tmp/'run',out);run(tmp/'run')
 records.append(dict(case='unsigned-decrement',cases=count,levels=4))
if len(sys.argv)>1:pathlib.Path(sys.argv[1]).write_text(json.dumps(records,indent=2)+'\n')
print('PA32 memory bounds PASS: exact linear work scaling; 32-cell snapshots; dense-CFG budget fallback; guards; '+str(count)+' modular comparisons x four levels')
