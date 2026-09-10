#!/usr/bin/env python3
"""Pair the initial retained helper with selected trivial-result elision."""
from pathlib import Path
import importlib.util
import json
import os
import platform
import statistics
import sys
import time
ROOT=Path(__file__).resolve().parents[2]
spec=importlib.util.spec_from_file_location('prior',ROOT/'student.tests/pa10/benchmark.py')
prior=importlib.util.module_from_spec(spec);spec.loader.exec_module(prior)
a,b,inputs,work,output=map(Path,sys.argv[1:6])
binaries=[a.resolve(),b.resolve()];work.mkdir(parents=True,exist_ok=True)
cpu=min(os.sched_getaffinity(0));os.sched_setaffinity(0,{cpu})
data=dict(protocol='warmups, four A/A observations, two ABBA blocks',cpu=cpu,
 platform=platform.platform(),harness_sha256=prior.sha(__file__),
 binaries=[dict(path=str(x),sha256=prior.sha(x),text_bytes=prior.text_size(x))for x in binaries],
 backend_sha256=prior.sha(ROOT/'reference-binaries/lowir2native'),
 compile_flags=['--emit-lowir','-O0'],native_flags=['-O0'],
 text_metric='compiler .text; sectionless native payload after entry',workloads={})
def observe(command):
 usage=work/'usage.txt';start=time.perf_counter_ns()
 prior.run(['/usr/bin/time','-f','%M','-o',usage,*command])
 return dict(wall_s=(time.perf_counter_ns()-start)/1e9,rss_kib=int(usage.read_text()))
def campaign(commands):
 warmups=[observe(c)for c in commands];rows=[]
 for label in prior.ORDER:
  row=observe(commands[label]);row['binary']=label;rows.append(row)
 return dict(warmups=warmups,observations=rows,
  aa_range_s=[min(x['wall_s']for x in rows[:4]),max(x['wall_s']for x in rows[:4])],
  paired_b_over_a=[statistics.mean(x['wall_s']for x in rows[k:k+4]if x['binary']==1)/
   statistics.mean(x['wall_s']for x in rows[k:k+4]if x['binary']==0)for k in (4,8)])
for name in ['explicit-400','explicit-runtime']:
 source=inputs/(name+'.cpp');commands=[];executables=[];records=[]
 for label,binary in enumerate(binaries):
  ir=work/(name+f'-{label}.lowir');exe=work/(name+f'-{label}')
  command=[binary,'--emit-lowir','-O0','-o',ir,source]
  stats=prior.run([*command,'--validate-lowir','--stats'])
  prior.run([ROOT/'dev/lowir2native-ref','-O0','-o',exe,ir]);prior.run([exe])
  commands.append(command);executables.append([exe])
  records.append(dict(binary=label,lowir_sha256=prior.sha(ir),lowir_bytes=ir.stat().st_size,
   executable_sha256=prior.sha(exe),text_bytes=prior.text_size(exe),checked_exit=0,
   telemetry=[json.loads(line)for line in stats.stderr.splitlines()]))
 data['workloads'][name]=dict(source_path=str(source),source_sha256=prior.sha(source),
  equivalence='same dynamic values; trivial transfers have no observable effects; both validate and exit 0',
  outputs=records,compiler=campaign(commands),runtime=campaign(executables))
 output.write_text(json.dumps(data,indent=2)+'\n');print(name,flush=True)
