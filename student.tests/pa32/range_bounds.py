#!/usr/bin/env python3
"""Linear range work, single helper emission, exit-edge growth and ABI guards."""
import json,pathlib,subprocess,sys,tempfile
root=pathlib.Path(__file__).resolve().parents[2]
out=pathlib.Path(sys.argv[1]);rows=[]
def run(*args):
 r=subprocess.run(list(map(str,args)),capture_output=True,text=True,timeout=90)
 assert r.returncode==0,(args,r.returncode,r.stderr[-3000:]);return r
fill=(root/'pa32/tests/o1/500-fill-loop-word-zero.t').read_text()
walk=(root/'pa32/tests/o1/500-effect-free-loop-deleted-twin-backward.t').read_text().replace('index i8 %walk, -8','index i8 %walk, -1')
with tempfile.TemporaryDirectory(prefix='pa32-range-bounds-') as temp:
 d=pathlib.Path(temp);src=d/'input.lowir';ir=d/'out.lowir'
 for n in [100,500,1000]:
  src.write_text(''.join(fill.replace('@zero_words',f'@fill{k}')+walk.replace('@destroy_backward',f'@walk{k}') for k in range(n)))
  r=run(root/'dev/lowiropt','-O1','--stats','-o',ir,src)
  stats=next(json.loads(s) for s in r.stderr.splitlines() if s.startswith('{') and 'loops_filled' in s)
  assert stats['loops_filled']==n and stats['loops_removed']==n,stats
  assert stats['optimize_work']<n*10000,stats
  assert ir.read_text().count('object=cppgm_opt_fill_bytes')==1
  run(root/'dev/lowir','-o',d/'valid.lowir',ir);run(root/'dev/cppgm++','-c','-O0','-o',d/'out.o',ir)
  rows.append(dict(case='scale',count=n,stats=stats))
 print([(r['count'],r['stats']['optimize_work']) for r in rows],flush=True)
 assert rows[2]['stats']['optimize_work'] <= 11*rows[0]['stats']['optimize_work']
 # Reuse the full compatible private ABI identity on explicit IR input.
 declaration='declare function @existing(%p : ptr, %v : i32, %n : i64) -> void [unwind=no, binding=internal, object=cppgm_opt_fill_bytes]\n'
 src.write_text(declaration+fill)
 run(root/'dev/lowiropt','-O1','-o',ir,src)
 assert ir.read_text().count('object=cppgm_opt_fill_bytes')==1
 assert 'call void @existing' in ir.read_text()
 run(root/'dev/cppgm++','-c','-O0','-o',d/'reuse.o',ir)
 # A stronger extent promise is not part of the new calls' fact key.
 src.write_text(declaration.replace('%p : ptr,','%p : ptr [object_bytes=1],')+fill)
 run(root/'dev/lowiropt','-O1','-o',ir,src)
 assert 'call void @existing' not in ir.read_text()
 assert 'call void @opt_fill_' in ir.read_text()
 src.write_text('global @tls : i64 [storage=thread_local] = 0\n'+declaration.replace('object=cppgm_opt_fill_bytes','object=cppgm_opt_fill_bytes, tls_for=@tls')+fill)
 run(root/'dev/lowiropt','-O1','-o',ir,src)
 assert 'call void @existing' not in ir.read_text()
 run(root/'dev/cppgm++','-c','-O0','-o',d/'distinct-runtime.o',ir)
 rows.append(dict(case='helper-identity',compatible_reused=True,stronger_promise_declined=True,distinct_runtime_declined=True))
 # Every exit phi needs the new body edge, with the same final pointer value.
 count=64
 s=['function @join(%first : ptr, %n : i64, %take : i64) -> i64 [no_inline=yes] {block ^entry:']
 s += [f'%other{k} = index i8 %first, {k}' for k in range(count)]
 s += ['%last = index i8 %first, %n','branch %take, ^pre, ^other','block ^pre: jump ^head','block ^head: %p = phi ptr [^pre: %first, ^body: %next]','%c = cmp ne ptr %p, %last','branch %c, ^body, ^done','block ^body: store u8 0, %p','%next = index i8 %p, 1','jump ^head','block ^other: jump ^done','block ^done:']
 s += [f'%r{k} = phi ptr [^head: %p, ^other: %other{k}]' for k in range(count)]
 s += [f'%c{k} = cmp eq ptr %r{k}, %last' for k in range(count)]
 for k in range(1,count):s += [f'%s{k} = binary add i64 '+('%c0' if k==1 else f'%s{k-1}')+f', %c{k}']
 s += [f'return i64 %s{count-1}','}', 'function @main() -> i32 [role=entry] {slot $buf : obj<64x8> block ^entry: %p = addr $buf','%a = call i64 @join(%p, 32, 1)','%b = call i64 @join(%p, 32, 0)',f'%aok = cmp eq i64 %a, {count}','%bok = cmp eq i64 %b, 1','%ok = binary and i64 %aok, %bok','%bad = cmp eq i64 %ok, 0','%result = convert trunc i32 i64 %bad','return i32 %result','}']
 src.write_text('\n'.join(s))
 for level in range(4):
  r=run(root/'dev/lowiropt',f'-O{level}','--stats','-o',ir,src)
  run(root/'dev/lowir2native','-o',d/'exe','--dump-machine-ir',d/'out.mir',ir);run(d/'exe')
  if level:
   stats=next(json.loads(s) for s in r.stderr.splitlines() if s.startswith('{') and 'loops_filled' in s)
   assert stats['loops_filled']==1,stats
   assert (d/'out.mir').read_text().count('fill_bytes rdi, rax, rcx')==1
   rows.append(dict(case='exit-phis',level=level,stats=stats))
 # Malformed externally supplied helper ABI is rejected before encoding.
 src.write_text('declare function @bad() -> void [object=cppgm_opt_fill_bytes]\nfunction @main() -> i32 [role=entry] {block ^entry: call void @bad() return i32 0}')
 result=subprocess.run([str(root/'dev/lowir2native'),'-o',str(d/'bad'),str(src)],capture_output=True,text=True)
 assert result.returncode!=0 and 'invalid fill boundary' in result.stderr
 rows.append(dict(case='invalid-helper-signature',exit_code=result.returncode))
out.write_text(json.dumps(rows,indent=2)+'\n')
print('range bounds: linear work; one four-instruction helper; 64 live exit phis x four levels; ABI rejection PASS')
