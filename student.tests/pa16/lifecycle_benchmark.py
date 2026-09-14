#!/usr/bin/env python3
"""New-correct-only multi-TU lifecycle scaling. ENTRY FINAL WORK OUT."""
from pathlib import Path
import json,os,statistics,subprocess,sys,time
ROOT=Path(__file__).resolve().parents[2]
sys.path.insert(0,str(ROOT/'student.tests/pa10'));import benchmark as shared
A,B,WORK,OUT=map(Path,sys.argv[1:5]);A=A.resolve();B=B.resolve();WORK.mkdir(parents=True,exist_ok=True)
cpu=max(os.sched_getaffinity(0));os.sched_setaffinity(0,{cpu})
r=dict(protocol='frozen inputs/binaries/flags; entry must reject duplicate singleton roles; one B warmup then six B observations per compiler/runtime phase; no comparison with invalid entry',cpu=cpu,flags=['--emit-lowir','-O0'],
 implementation_commit=shared.run(['git','rev-parse','HEAD']).stdout.strip(),harness_sha256=shared.sha(__file__),
 binaries=[dict(path=str(p),sha256=shared.sha(p),text_bytes=shared.text_size(p)) for p in [A,B]],
 backend=dict(path=str(ROOT/'reference-binaries/lowir2native'),sha256=shared.sha(ROOT/'reference-binaries/lowir2native'),flags=['-O0']),
 acceptance='PA16 required singleton role correctness. O(TU hooks) coordinator calls/storage, at most two coordinator functions; no optional optimizer or mandated numeric latency ceiling.',workloads={})
def observe(command):
 start=time.perf_counter_ns();usage=WORK/'usage.txt';shared.run(['/usr/bin/time','-f','%M %U %S %c %w','-o',usage,*command])
 rss,u,s,i,v=usage.read_text().split();return dict(wall_s=(time.perf_counter_ns()-start)/1e9,rss_kib=int(rss),user_s=float(u),system_s=float(s),involuntary=int(i),voluntary=int(v),checked_exit=0)
def measure(command):return dict(warmups=[observe(command)],observations=[observe(command) for _ in range(6)])
def save():OUT.write_text(json.dumps(r,indent=2)+'\n')
for count in [64,256]:
 work=WORK/str(count);work.mkdir(exist_ok=True);sources=[]
 for i in range(count):
  p=work/f'unit{i}.cpp';p.write_text(f'int get{i}(){{return {i};}}int value{i}=get{i}();\n');sources.append(p)
 source=''.join(f'extern int value{i};\n' for i in range(count))+'int main(){int total=0;'+''.join(f'total+=value{i};' for i in range(count))
 source+=f'volatile int n=60000000;unsigned sum=0;for(int i=0;i<n;++i)sum=(sum+value{count-1})&65535u;return total=={count*(count-1)//2}&&sum=={(60000000*(count-1))&65535}u?0:1;}}\n'
 main=work/'main.cpp';main.write_text(source);sources.append(main)
 ir=work/'program.lowir';exe=work/'program';command=[B,'--emit-lowir','-O0','-o',ir,*sources]
 old=subprocess.run([A,'--emit-lowir','-O0','--validate-lowir','-o',work/'entry.lowir',*sources],capture_output=True,text=True,timeout=300)
 assert old.returncode!=0 and 'duplicate singleton role' in old.stderr
 preflight=shared.run([*command,'--stats','--validate-lowir']);shared.run([ROOT/'dev/lowir','-o',work/'roundtrip.lowir',ir]);shared.run([ROOT/'dev/lowir2native-ref','-O0','-o',exe,ir]);shared.run([exe])
 telemetry=[json.loads(s) for s in preflight.stderr.splitlines()];assert telemetry[-1]['initializer_units']==count
 body=ir.read_text();assert body.count('role=init')==1 and body.count('role=fini')==0
 payload=shared.text_size(exe)
 item=dict(sources=[dict(path=str(p),sha256=shared.sha(p)) for p in sources],entry_exit=old.returncode,entry_diagnostic=old.stderr,
  output=dict(path=str(ir),sha256=shared.sha(ir),bytes=ir.stat().st_size,telemetry=telemetry[-1]),
  native=dict(path=str(exe),sha256=shared.sha(exe),code_and_alignment_bytes=payload-4*count,global_data_bytes=4*count,payload_bytes=payload,file_bytes=exe.stat().st_size,checked_exit=0))
 r['workloads'][str(count)]=item;save();print(count,'preflight',flush=True)
 item['compiler']=measure(command);item['runtime']=measure([exe]);save();print(count,'measured',flush=True)
for b in r['binaries']:assert shared.sha(b['path'])==b['sha256']
r['observations']=28;save()
