#!/usr/bin/env python3
"""CFG/ABI/debug guards and pipeline work/growth evidence for affine loops."""
import json,pathlib,subprocess,tempfile,sys
ROOT=pathlib.Path(__file__).resolve().parents[2]
out=pathlib.Path(sys.argv[1]).resolve() if len(sys.argv)>1 else None
if out:out.mkdir(parents=True,exist_ok=True)
def run(*args):
 r=subprocess.run(list(map(str,args)),capture_output=True,text=True,timeout=60)
 assert r.returncode==0,(args,r.returncode,r.stderr[-3000:]);return r
base='''function @f(%x : i64) -> i64 [binding=strong, linkage=c, no_inline=yes] {
block ^entry: jump ^head
block ^head:
 %i = phi i64 [^entry: 0, ^body: %next]
 %sum = phi i64 [^entry: %x, ^body: %s]
 %c = cmp lt i64 %i, 4
 branch %c, ^body, ^exit
block ^body:
 %s = binary add i64 %sum, %i !dbg("loops.cpp", 12, 3)
 store volatile i64 %s, @observed !dbg("loops.cpp", 13, 3)
 %next = binary add i64 %i, 1
 jump ^head
block ^exit: return i64 %sum
}'''
rows=[]
with tempfile.TemporaryDirectory(prefix='pa32-loop-bounds-') as tmp:
 d=pathlib.Path(tmp)
 for count in [100,500,1000]:
  src=d/'many.lowir';src.write_text('global @observed : i64 = 0\n'+''.join(base.replace('@f(',f'@f{n}(') for n in range(count)))
  for level in [1,3]:
   ir=d/'out.lowir';r=run(ROOT/'dev/lowiropt',f'-O{level}','--stats','-o',ir,src)
   metrics=[json.loads(s) for s in r.stderr.splitlines() if s.startswith('{')]
   opt=next(m for m in metrics if 'loop_candidates' in m)
   assert opt['loop_candidates']==count,opt
   assert opt['loop_clone_reserved']<=min(4096,2*(count*11+1)),opt
   assert opt['optimize_work']<count*6000,opt
   if level==1: assert opt['loops_unrolled']==0,opt
   else:
    assert opt['loops_unrolled']>0,opt
    assert ir.read_text().count('!dbg("loops.cpp", 13, 3)')==count+opt['loops_unrolled']*3
   rows.append(dict(functions=count,level=level,telemetry=metrics))
 # A single caller's growth budget applies across every disjoint loop.
 parts=['global @observed : i64 = 0','function @f() -> i64 {block ^entry: jump ^head0']
 for n in range(20):
  parts.append(f'''block ^head{n}:
 %i{n} = phi i64 [^{'entry' if n==0 else 'exit'+str(n-1)}: 0, ^body{n}: %next{n}]
 %c{n} = cmp lt i64 %i{n}, 4
 branch %c{n}, ^body{n}, ^exit{n}
block ^body{n}:
 store volatile i64 %i{n}, @observed
 %next{n} = binary add i64 %i{n}, 1
 jump ^head{n}
block ^exit{n}:
 {'jump ^head'+str(n+1) if n<19 else 'return i64 3'}''')
 parts.append('}')
 src=d/'one.lowir';src.write_text('\n'.join(parts));ir=d/'one-out.lowir'
 r=run(ROOT/'dev/lowiropt','-O3','--stats','-o',ir,src)
 opt=next(json.loads(s) for s in r.stderr.splitlines() if s.startswith('{') and 'loop_candidates' in s)
 assert 1<opt['loops_unrolled']<20,opt
 assert opt['loop_clone_reserved']<=256,opt
 rows.append(dict(case='one-function-budget',telemetry=opt))
 # More than 64 cloned body instructions must be declined transactionally.
 src=d/'large.lowir';src.write_text('global @observed : i64 = 0\n'+base.replace(' %s = binary add',(' store volatile i64 %x, @observed\n'*17)+' %s = binary add'))
 r=run(ROOT/'dev/lowiropt','-O3','--stats','-o',ir,src)
 opt=next(json.loads(s) for s in r.stderr.splitlines() if s.startswith('{') and 'loop_candidates' in s)
 assert opt['loops_unrolled']==0 and opt['loop_clone_reserved']==0,opt
 assert 'phi ' in ir.read_text()
 rows.append(dict(case='body-budget',telemetry=opt))
 # A variadic narrow phi retains its operand ABI type through snapshots.
 src=d/'varargs.lowir';src.write_text('''declare function @check() -> i64 [arity=variadic, binding=strong, linkage=c, unwind=no]
function @f() -> i64 [binding=strong, linkage=c] {
block ^entry: jump ^head
block ^head:
 %i = phi i8 [^entry: -4, ^body: %next]
 %c = cmp lt i8 %i, 0
 branch %c, ^body, ^exit
block ^body:
 %r = call i64 @check(%i)
 %next = binary add i8 %i, 1
 jump ^head
block ^exit: return i64 0 }
''')
 ir=d/'var-out.lowir';run(ROOT/'dev/lowiropt','-O3','-o',ir,src)
 assert ir.read_text().count('call i64 @check')==4
 run(ROOT/'dev/cppgm++','-c','-O0',ir,'-o',d/'var.o')
 # Exception registration invalidates ordinary dominance, keeping a real loop.
 src=d/'eh.lowir';src.write_text('global @observed : i64 = 0\n'+base.replace('block ^entry: jump ^head','block ^entry: eh_cleanup ^landing\njump ^head').replace('block ^exit: return i64 %sum','block ^exit: eh_end\nreturn i64 %sum\nblock ^landing: resume'))
 run(ROOT/'dev/lowiropt','-O3','-o',ir,src);assert 'phi ' in ir.read_text()
print('loop scaling, unit/function/body budgets, debug, variadic ABI and EH guards PASS')
if out:(out/'bounds.json').write_text(json.dumps(rows,indent=2)+'\n')
