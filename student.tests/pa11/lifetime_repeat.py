#!/usr/bin/env python3
"""Keep a complete targeted follow-up of noisy frozen compiler observations."""
from pathlib import Path
import importlib.util
import json
import os
import sys
import time
ROOT=Path(__file__).resolve().parents[2]
spec=importlib.util.spec_from_file_location('prior',ROOT/'student.tests/pa10/benchmark.py')
prior=importlib.util.module_from_spec(spec);spec.loader.exec_module(prior)
source,destination=map(Path,sys.argv[1:3]);original=json.loads(source.read_text())
os.sched_setaffinity(0,{original['cpu']})
data=dict(protocol='performance-protocol.md#lifetime-continuation-campaign',original_path=str(source),
    original_sha256=prior.sha(source),harness_sha256=prior.sha(__file__),binaries=original['binaries'],
    cpu=original['cpu'],order=prior.ORDER,inputs={},observations=[])
for binary in data['binaries']:assert prior.sha(binary['path'])==binary['sha256']
for group in ('template-semantics-1','references-4','references-8000','memory-float-4'):
    entry=original['inputs'][group];src=Path(entry['path'])
    assert prior.sha(src)==entry['sha256']
    assert [prior.sha(src.with_suffix(s)) for s in ('.ref','.my')]==entry['output_hashes']
    data['inputs'][group]=entry
    output,usage=src.with_suffix('.repeat'),src.with_suffix('.time')
    for ordinal,label in enumerate(prior.ORDER):
        flags=[] if entry['mode']=='--emit-semantics' else ['-O0']
        command=[data['binaries'][label]['path'],entry['mode'],*flags,'-o',output,src]
        start=time.perf_counter_ns()
        result=prior.run(['/usr/bin/time','-f','%M %U %S %c %w','-o',usage,*command])
        wall=(time.perf_counter_ns()-start)/1e9
        rss,user,system,involuntary,voluntary=usage.read_text().split()
        assert not result.stderr
        assert prior.sha(output)==entry['output_hashes'][label]
        data['observations'].append(dict(group=group,binary=label,ordinal=ordinal,wall_s=wall,rss_kib=int(rss),
            user_s=float(user),system_s=float(system),involuntary=int(involuntary),voluntary=int(voluntary)))
        destination.write_text(json.dumps(data,indent=2)+'\n')
    print(group,prior.ratios([r for r in data['observations'] if r['group']==group]),flush=True)
