#!/usr/bin/env python3
"""PA14 expression-record packing: frozen common A/A+ABBA and new B-only work."""
from pathlib import Path
import json, os, platform, statistics, subprocess, sys, time
ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0,str(ROOT/'student.tests/pa10'))
import benchmark as shared
A, B, WORK, OUT = map(Path,sys.argv[1:5])
binaries = [A.resolve(),B.resolve()]
WORK.mkdir(parents=True,exist_ok=True)
cpu = min(os.sched_getaffinity(0)); os.sched_setaffinity(0,{cpu})
result = dict(protocol='one warmup per binary; four A/A observations; two ABBA blocks; new semantics six B-only observations',
 cpu=cpu, platform=platform.platform(), host=shared.run(['g++','--version']).stdout.splitlines()[0],
 flags=['--emit-lowir','-O0'],backend_flags=['-O0'],
 build_flags='g++ -std=gnu++11 -Wall -O3; TEST_RUNNER_ENABLE',
 commits=['47f5f975',shared.run(['git','rev-parse','HEAD']).stdout.strip()],
 harness_sha256=shared.sha(__file__),shared_harness_sha256=shared.sha(ROOT/'student.tests/pa10/benchmark.py'),
 backend_sha256=shared.sha(ROOT/'reference-binaries/lowir2native'),
 binaries=[dict(path=str(p),sha256=shared.sha(p),text_bytes=shared.text_size(p)) for p in binaries],
 text_metric='compiler .text; supplied sectionless ELF payload after entry (includes support/data)',
 acceptance='O0 semantic implementation, no optional optimization, no self-imposed numeric exit gate; disclose and investigate common-correct regressions',
 workloads={})
def observe(command):
 usage=WORK/'usage.txt'; started=time.perf_counter_ns()
 shared.run(['/usr/bin/time','-f','%M %U %S %c %w','-o',usage,*command])
 rss,user,system,involuntary,voluntary=usage.read_text().split()
 return dict(wall_s=(time.perf_counter_ns()-started)/1e9,rss_kib=int(rss),user_s=float(user),system_s=float(system),involuntary=int(involuntary),voluntary=int(voluntary),checked_exit=0)
def campaign(commands):
 warmups=[dict(binary=b,**observe(c)) for b,c in commands.items()]
 rows=[dict(binary=b,**observe(commands[b])) for b in (shared.ORDER if len(commands)==2 else [1]*6)]
 out=dict(warmups=warmups,observations=rows)
 if len(commands)==2:
  out['aa_range_s']=[min(r['wall_s'] for r in rows[:4]),max(r['wall_s'] for r in rows[:4])]
  out['paired_b_over_a']=[statistics.mean(r['wall_s'] for r in rows[k:k+4] if r['binary']==1)/statistics.mean(r['wall_s'] for r in rows[k:k+4] if r['binary']==0) for k in (4,8)]
 return out
prior=json.loads((ROOT/'student.tests/pa14/symbolic-context-performance.json').read_text())
corpus=[]
for name in ('template-semantics-4','calls-4','query-instances-4000','query-runtime'):
 w=prior['workloads'][name]
 assert shared.sha(w['source_path'])==w['source_sha256']
 corpus.append((name,Path(w['source_path']).read_text(),w['mode'],True,'runtime' in w))
for name,source,mode,common,executable in corpus:
 src=WORK/(name+'.cpp');src.write_text(source); outputs=[];commands={};runtimes={}
 for b in ((0,1) if common else (1,)):
  ir=WORK/(name+f'-{b}.out'); command=[binaries[b],mode,*(['-O0'] if mode=='--emit-lowir' else []),'-o',ir,src]
  stats=shared.run([*command,'--stats',*(['--validate-lowir'] if mode=='--emit-lowir' else [])])
  item=dict(binary=b,path=str(ir),sha256=shared.sha(ir),bytes=ir.stat().st_size,telemetry=[json.loads(line) for line in stats.stderr.splitlines()])
  if executable:
   exe=WORK/(name+f'-{b}');shared.run([ROOT/'dev/lowir2native-ref','-O0','-o',exe,ir]);shared.run([exe])
   item['native']=dict(path=str(exe),sha256=shared.sha(exe),text_bytes=shared.text_size(exe),checked_exit=0);runtimes[b]=[exe]
  outputs.append(item);commands[b]=command
 if common:
  assert outputs[0]['sha256']==outputs[1]['sha256'],(name,'common correct output changed')
  if executable: assert outputs[0]['native']['sha256']==outputs[1]['native']['sha256'],name
 if name.startswith('definition-instances-'):
  assert outputs[0]['telemetry'][0]['template_definition_applications']==3*int(name.rsplit('-',1)[1])
 if name.startswith('template-repeat-'):
  assert outputs[0]['telemetry'][0]['template_body_transitions']==1
 if name.startswith('class-instances-'):
  assert outputs[0]['telemetry'][0]['template_class_completions']==int(name.rsplit('-',1)[1])
 item=dict(source_path=str(src),source_sha256=shared.sha(src),mode=mode,outputs=outputs,
  equivalence='byte-identical correct A/B output' if common else 'correct B only; A lacks dependent query types or correct fixed binding through dependent bases',compiler=campaign(commands))
 if runtimes:item['runtime']=campaign(runtimes)
 result['workloads'][name]=item;OUT.write_text(json.dumps(result,indent=2)+'\n');print(name,flush=True)
assert [shared.sha(p) for p in binaries]==[b['sha256'] for b in result['binaries']]
