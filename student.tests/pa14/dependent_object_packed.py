#!/usr/bin/env python3
"""Repeat ownership measurements with packed source prototype classification."""
from pathlib import Path
import json,os,statistics,sys,time
ROOT=Path(__file__).resolve().parents[2]
sys.path.insert(0,str(ROOT/'student.tests/pa10'))
import benchmark as shared
A,B,WORK,OUT=map(Path,sys.argv[1:5]);WORK.mkdir(parents=True,exist_ok=True)
parent=ROOT/'student.tests/pa14/dependent-object-final-performance.json';prior=json.loads(parent.read_text())
binaries=[A.resolve(),B.resolve()];assert shared.sha(A)==prior['binaries'][0]['sha256']
os.sched_setaffinity(0,{prior['cpu']})
result={k:v for k,v in prior.items() if k not in ('workloads','binaries','commits')}
result.update(harness_sha256=shared.sha(__file__),parent_path=str(parent),parent_sha256=shared.sha(parent),
 commits=['33b791da',shared.run(['git','rev-parse','HEAD']).stdout.strip()],
 purpose='Pack source prototype classification into two bits per parsed node; preserve all outputs',
 binaries=[dict(path=str(p),sha256=shared.sha(p),text_bytes=shared.text_size(p)) for p in binaries],workloads={})
def observe(command):
 usage=WORK/'usage.txt';started=time.perf_counter_ns()
 shared.run(['/usr/bin/time','-f','%M %U %S %c %w','-o',usage,*command])
 rss,user,system,involuntary,voluntary=usage.read_text().split()
 return dict(wall_s=(time.perf_counter_ns()-started)/1e9,rss_kib=int(rss),user_s=float(user),system_s=float(system),involuntary=int(involuntary),voluntary=int(voluntary),checked_exit=0)
def campaign(commands):
 warmups=[dict(binary=b,**observe(commands[b])) for b in (0,1)]
 rows=[dict(binary=b,**observe(commands[b])) for b in shared.ORDER]
 return dict(warmups=warmups,observations=rows,aa_range_s=[min(r['wall_s'] for r in rows[:4]),max(r['wall_s'] for r in rows[:4])],
  paired_b_over_a=[statistics.mean(r['wall_s'] for r in rows[k:k+4] if r['binary']==1)/statistics.mean(r['wall_s'] for r in rows[k:k+4] if r['binary']==0) for k in (4,8)])
# Preflight the entire parent corpus, including the unselected native controls.
result['preflight']=[]
for name,old in prior['workloads'].items():
 expected=old['outputs'][1];out=WORK/(name+'-preflight.lowir')
 shared.run([B,old['mode'],'-O0','--validate-lowir','-o',out,old['source_path']])
 assert shared.sha(out)==expected['sha256']
 row=dict(source_path=old['source_path'],source_sha256=old['source_sha256'],path=str(out),sha256=shared.sha(out))
 if 'native' in expected:
  exe=WORK/(name+'-preflight');shared.run([ROOT/'dev/lowir2native-ref','-O0','-o',exe,out]);shared.run([exe]);assert shared.sha(exe)==expected['native']['sha256']
  row.update(native_path=str(exe),native_sha256=shared.sha(exe),native_exit=0)
 result['preflight'].append(row)
for name in ('member-instances-4000','member-outside-4000','member-unused-4000','member-repeated-4000',
             'calls-4','memory-float-4','object-instances-4000','object-unused-4000','member-runtime','call-materializations-runtime'):
 old=prior['workloads'][name];assert shared.sha(old['source_path'])==old['source_sha256']
 commands={};runtimes={};outputs=[];expected=old['outputs'][-1]
 for b in (0,1):
  out=WORK/(name+f'-{b}.lowir');command=[binaries[b],old['mode'],'-O0','-o',out,old['source_path']]
  stats=shared.run([*command,'--stats','--validate-lowir']);assert shared.sha(out)==expected['sha256']
  item=dict(binary=b,path=str(out),sha256=shared.sha(out),bytes=out.stat().st_size,
            telemetry=[json.loads(line) for line in stats.stderr.splitlines()])
  if 'runtime' in old:
   exe=WORK/(name+f'-{b}');shared.run([ROOT/'dev/lowir2native-ref','-O0','-o',exe,out]);shared.run([exe])
   assert shared.sha(exe)==expected['native']['sha256']
   item['native']=dict(path=str(exe),sha256=shared.sha(exe),text_bytes=shared.text_size(exe),checked_exit=0);runtimes[b]=[exe]
  outputs.append(item);commands[b]=command
 item=dict(source_path=old['source_path'],source_sha256=old['source_sha256'],mode=old['mode'],outputs=outputs,
           exact_required=True,equivalence='byte-identical correct output across source field/prototype work',compiler=campaign(commands))
 if runtimes:item['runtime']=campaign(runtimes)
 result['workloads'][name]=item;OUT.write_text(json.dumps(result,indent=2)+'\n');print(name,flush=True)
for b in result['binaries']:assert shared.sha(b['path'])==b['sha256']
