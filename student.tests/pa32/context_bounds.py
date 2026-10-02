#!/usr/bin/env python3
"""Count scaling and budget fallback for contextual clone admission."""
import hashlib,json,pathlib,subprocess,sys,tempfile
root=pathlib.Path(__file__).resolve().parents[2]
out=pathlib.Path(sys.argv[1]);rows=[]
def run(*args):
 p=subprocess.run(list(map(str,args)),capture_output=True,text=True,timeout=120)
 assert p.returncode==0,(args,p.returncode,p.stderr);return p
preamble='''declare function @failure() -> void
function @checked(%p : ptr, %n : i64) -> i64 {
 block ^entry: eh_try ^landing %c = cmp uge i64 %n, 4 branch %c, ^fail, ^load
 block ^fail: call void @failure() return i64 0
 block ^load: %a = index i64 %p, %n %v = load i64 %a eh_end return i64 %v
 block ^landing: eh_catch_all, 1 return i64 -1
}
'''
with tempfile.TemporaryDirectory(prefix='pa32-context-bounds-') as d:
 t=pathlib.Path(d)
 for count in [100,500,1000]:
  src=t/'input.lowir';src.write_text(preamble+''.join(f'''function @f{n}(%p : ptr) -> i64 {{block ^entry:
 %v = call i64 @checked(%p, 2) return i64 %v }}\n''' for n in range(count)))
  result=run(root/'dev/cppgm++','-O1','--stats','-c',src,'-o',t/'out.o')
  counters=[json.loads(s) for s in result.stderr.splitlines() if s.startswith('{')]
  opt=next(s for s in counters if 'optimize_work' in s);native=next(s for s in counters if 'inline_budget_work' in s)
  assert opt['inline_context_sites']==count
  assert opt['inline_context_work']<=4096*count
  assert native['inline_max_function_work']<=32768
  rows.append(dict(count=count,source_sha256=hashlib.sha256(src.read_bytes()).hexdigest(),optimize_work=opt["optimize_work"],counters=counters))
 assert rows[-1]['optimize_work']<=11*rows[0]['optimize_work']
 # A single large caller must exhaust its finite per-function allowance,
 # keeping remaining calls instead of failing or expanding without a cap.
 src.write_text(preamble+'function @large(%p : ptr) -> i64 { block ^entry:\n'+''.join(f'%v{n} = call i64 @checked(%p, 2)\n' for n in range(1200))+'return i64 %v1199\n}')
 result=run(root/'dev/cppgm++','-O1','--stats','-c',src,'-o',t/'large.o')
 counters=[json.loads(s) for s in result.stderr.splitlines() if s.startswith('{')]
 native=next(s for s in counters if 'inline_budget_work' in s)
 assert native['inline_declined']>0 and native['inline_max_function_work']<=32768
 run(root/'dev/lowiropt','-O1','-o',t/'out.lowir',src);run(root/'dev/lowir','-o',t/'validated',t/'out.lowir')
 assert 'call i64 @checked' in (t/'out.lowir').read_text()
 rows.append(dict(caller_calls=1200,source_sha256=hashlib.sha256(src.read_bytes()).hexdigest(),counters=counters))
out.write_text(json.dumps(rows,indent=2)+'\n');print('PA32 context bounds: PASS')
