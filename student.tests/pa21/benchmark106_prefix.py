#!/usr/bin/env python3
"""Final-only live-prefix scaling; no speedup claim against incorrect entry EH."""
from pathlib import Path
import hashlib,json,os,statistics,subprocess,sys,time
ROOT=Path(__file__).resolve().parents[2]
sys.path.insert(0,str(ROOT/'student.tests/pa10'))
from benchmark import run,sha,text_size
CC,WORK,OUT=[Path(p).resolve() for p in sys.argv[1:]]
assert not OUT.exists();WORK.mkdir(parents=True,exist_ok=True)
cpu=max(os.sched_getaffinity(0));os.sched_setaffinity(0,{cpu})
result=dict(compiler_sha256=sha(CC),object_backend_sha256=sha(ROOT/'reference-binaries/cppgm++'),harness_sha256=sha(__file__),cpu=cpu,protocol='one warmup, four final-only observations per scale; all exits checked',rows=[])
for n in (8,32,128):
 source='int drops;struct G{int n;G(int x):n(x){if(x<0)throw x;}~G(){++drops;}};int f(){try{'
 source+=''.join('G g%d(%d);'%(i,i) for i in range(n))+'throw 7;}catch(int n){return n;}}int main(){return f()!=7||drops!=%d;}'%n
 src=WORK/('prefix-'+str(n)+'.cpp');src.write_text(source);ir=src.with_suffix('.lowir');obj=src.with_suffix('.o');exe=src.with_suffix('')
 cmd=[CC,'--emit-lowir','-O0','-o',ir,src]
 stats=run([*cmd,'--stats','--validate-lowir'])
 run([ROOT/'dev/cppgm++-ref','-c','-O0','-o',obj,ir]);run(['g++','-no-pie',obj,'-o',exe]);run([exe])
 def observe(command):
  usage=WORK/'time.txt';start=time.perf_counter_ns()
  run(['/usr/bin/time','-f','%M','-o',usage,*command])
  return dict(wall_s=(time.perf_counter_ns()-start)/1e9,rss_kib=int(usage.read_text()),checked_exit=0)
 row=dict(n=n,source=source,source_sha256=sha(src),lowir_sha256=sha(ir),lowir_bytes=ir.stat().st_size,
  compiler_warmup=observe(cmd),compiler_samples=[observe(cmd) for _ in range(4)],runtime_warmup=observe([exe]),runtime_samples=[observe([exe]) for _ in range(4)],
  telemetry=[json.loads(s) for s in stats.stderr.splitlines()],host_text_bytes=text_size(exe),checked_destructions=n)
 result['rows'].append(row);OUT.write_text(json.dumps(result,indent=2)+'\n');print(n,flush=True)
assert result['compiler_sha256']==sha(CC)
