#!/usr/bin/env python3
"""Frozen ordinary compiler comparisons; ABBA, A/A noise and separate telemetry.
measure.py measure PA5_BASE PA6_FIRST FINAL record.json
measure.py verify PA5_BASE PA6_FIRST FINAL record.json
measure.py report record.json
"""
import hashlib
import json
import os
import pathlib
import platform
import statistics as st
import subprocess
import sys
import tempfile
import time
from bench_inputs import workloads
ORDERS=[('AA1','AA'),('AA2','AA'),('BB','BB'),('ABBA1','ABBA'),('ABBA2','ABBA')]
BUDGETS=dict(wall_percent=10,rss_percent=15,rss_allowance_kib=1024,stage_text_percent=35,incremental_text_percent=5,scaling_wall=6,scaling_rss=5,startup_multiple=20)
def sha(path):return hashlib.sha256(path.read_bytes()).hexdigest()
def text_size(path):return sum(int(x.split()[1]) for x in subprocess.check_output(['size','-A',path],text=True).splitlines() if x.startswith('.text '))
def measure(paths,output):
    paths=[p.resolve() for p in paths]
    cpu=min(os.sched_getaffinity(0));os.sched_setaffinity(0,{cpu})
    data=dict(budgets=BUDGETS,protocol=ORDERS,platform=platform.platform(),cpu=cpu,
              host_flags='g++ -std=gnu++11 -Wall -O3; TEST_RUNNER_ENABLE',
              host_cxx=subprocess.check_output(['g++','--version'],text=True).splitlines()[0],
              binaries=[dict(path=str(p),sha256=sha(p),text_bytes=text_size(p)) for p in paths],
              source_commits=['9249196518f45492822fb2e3da4eb5d82af0ed13','5749f43b4',subprocess.check_output(['git','rev-parse','HEAD'],text=True).strip()],
              observations=[],startup=[],work=[],telemetry=[],inputs={},generated_runtime=None,generated_text=None)
    with tempfile.TemporaryDirectory(prefix='pa6-timing-') as tmp:
        tmp=pathlib.Path(tmp);out=tmp/'out';rss=tmp/'rss';empty=tmp/'empty.cpp';empty.write_text('')
        def observe(binary,mode,src,repeats,stats=False):
            cmd=['/usr/bin/time','-f','%M','-o',rss,paths[binary],mode]
            if stats:cmd+=['--stats']
            cmd+=['-o',out,*([src]*repeats)]
            start=time.perf_counter_ns();run=subprocess.run(cmd,capture_output=True,text=True,timeout=180)
            wall=(time.perf_counter_ns()-start)/1e9
            assert run.returncode==0,(binary,src,run.stderr)
            assert stats or not run.stderr,run.stderr
            return dict(binary=binary,mode=mode,stats=stats,wall_s=wall,rss_kib=int(rss.read_text()),
                        output_sha256=sha(out),output_bytes=out.stat().st_size,
                        phases=[json.loads(x) for x in run.stderr.splitlines()] if stats else [])
        for mode,baseline in [('--emit-ast',0),('--emit-types',1)]:
            for label in 'ABABABAB':
                row=observe(baseline if label=='A' else 2,mode,empty,4)
                row['variant']=label;data['startup'].append(row)
        for name,case in sorted(workloads().items()):
            src=tmp/(name+'.cpp');src.write_text(case['source'])
            mode=case['mode'];baseline=0 if mode=='--emit-ast' else 1
            data['inputs'][name]={k:v for k,v in case.items() if k!='source'}
            data['inputs'][name].update(sha256=sha(src),bytes=src.stat().st_size)
            expected=None
            for block,order in ORDERS:
                for ordinal,label in enumerate(order):
                    row=observe(baseline if label=='A' else 2,mode,src,case['repeats'])
                    expected=expected or row['output_sha256']
                    assert expected==row['output_sha256'],(name,'outputs differ')
                    row.update(input=name,variant=label,block=block,ordinal=ordinal)
                    data['observations'].append(row)
                output.write_text(json.dumps(data,indent=2)+'\n')
                print(name,block,'complete',flush=True)
            for label in 'AB':
                row=observe(baseline if label=='A' else 2,mode,src,case['repeats'],True)
                assert row['output_sha256']==expected
                row.update(input=name,variant=label);data['work'].append(row)
            if name in ('types-constants-4','types-signatures-4'):
                for block,order in ORDERS:
                    for ordinal,label in enumerate(order):
                        row=observe(2,mode,src,case['repeats'],label=='B')
                        assert row['output_sha256']==expected
                        row.update(input=name,variant=label,block=block,ordinal=ordinal)
                        data['telemetry'].append(row)
            output.write_text(json.dumps(data,indent=2)+'\n')
    assert all(sha(p)==v['sha256'] for p,v in zip(paths,data['binaries']))
def summarize(rows):
    blocks={k:[r for r in rows if r['block']==k] for k,_ in ORDERS}
    for k,order in ORDERS:
        assert ''.join(r['variant'] for r in blocks[k])==order
        assert [r['ordinal'] for r in blocks[k]]==list(range(len(order)))
    noise=max(abs(v[0]['wall_s']-v[1]['wall_s'])/st.mean(x['wall_s'] for x in v) for k,v in blocks.items() if k.startswith('AA'))
    paired=[]
    for block in ('ABBA1','ABBA2'):
        a=[r for r in blocks[block] if r['variant']=='A'];b=[r for r in blocks[block] if r['variant']=='B']
        paired.append(dict(wall_ratio=st.mean(r['wall_s'] for r in b)/st.mean(r['wall_s'] for r in a),
                           A_rss=st.mean(r['rss_kib'] for r in a),B_rss=st.mean(r['rss_kib'] for r in b)))
    result=dict(noise_percent=noise*100,paired=paired)
    for label in 'AB':
        values=[r for r in rows if r['variant']==label and r['block'].startswith('ABBA')]
        result[label]=dict(wall_s=st.median(r['wall_s'] for r in values),rss_kib=st.median(r['rss_kib'] for r in values),spread_s=[min(r['wall_s'] for r in values),max(r['wall_s'] for r in values)])
    return result
def verify(paths,data):
    assert data['budgets']==BUDGETS and data['protocol']==[list(x) for x in ORDERS]
    assert all(sha(p)==v['sha256'] and text_size(p)==v['text_bytes'] for p,v in zip(paths,data['binaries']))
    assert data['binaries'][2]['text_bytes']<=data['binaries'][0]['text_bytes']*1.35
    assert data['binaries'][2]['text_bytes']<=data['binaries'][1]['text_bytes']*1.05
    cases=workloads();assert set(data['inputs'])==set(cases)
    assert len(data['observations'])==14*len(cases) and len(data['startup'])==16 and len(data['work'])==2*len(cases) and len(data['telemetry'])==28
    summaries={}
    for name,case in cases.items():
        assert hashlib.sha256(case['source'].encode()).hexdigest()==data['inputs'][name]['sha256']
        rows=[r for r in data['observations'] if r['input']==name]
        assert len({r['output_sha256'] for r in rows})==1
        metrics=summarize(rows);summaries[name]=metrics
        startup=max(r['wall_s'] for r in data['startup'] if r['mode']==case['mode'])
        assert min(r['wall_s'] for r in rows)>startup*20,(name,'startup',startup)
        for pair in metrics['paired']:
            assert pair['wall_ratio']<=1.1+metrics['noise_percent']/100,(name,'wall',metrics)
            assert pair['B_rss']<=pair['A_rss']*1.15+1024,(name,'rss',metrics)
        work=[r for r in data['work'] if r['input']==name]
        assert len(work)==2 and all(r['output_sha256']==rows[0]['output_sha256'] for r in work)
        if case['mode']=='--emit-types':
            for phase in work[1]['phases']:
                assert phase['semantic_signature_work']<=phase['semantic_types']
                assert phase['semantic_constant_work']<=phase['nodes']
    for name,case in cases.items():
        if case['scale']!=4:continue
        small=name[:-1]+'1';large=summaries[name]['B'];base=summaries[small]['B']
        assert large['wall_s']<base['wall_s']*6,(name,'wall scaling')
        assert large['rss_kib']<base['rss_kib']*5+1024,(name,'rss scaling')
    assert data['generated_runtime'] is None and data['generated_text'] is None
    for name in ('types-constants-4','types-signatures-4'):
        rows=[r for r in data['telemetry'] if r['input']==name];summarize(rows)
        expected=next(r['output_sha256'] for r in data['observations'] if r['input']==name)
        assert all(r['output_sha256']==expected for r in rows)
    print('PA6 performance protocol, hashes, output equivalence and budgets passed')
def report(data):
    print('| Input | Final wall seconds (range) | Final RSS KiB | B/A paired wall | A/A noise |')
    print('| --- | --- | --- | --- | --- |')
    for name in sorted(data['inputs']):
        rows=[r for r in data['observations'] if r['input']==name]
        m=summarize(rows);b=m['B']
        print(f"| {name} | {b['wall_s']:.3f} ({b['spread_s'][0]:.3f}–{b['spread_s'][1]:.3f}) | {b['rss_kib']:.0f} | "+', '.join(f"{p['wall_ratio']:.3f}" for p in m['paired'])+f" | {m['noise_percent']:.2f}% |")
if __name__=='__main__':
    if sys.argv[1]=='report':report(json.loads(pathlib.Path(sys.argv[2]).read_text()))
    else:
        paths=[pathlib.Path(x) for x in sys.argv[2:5]];output=pathlib.Path(sys.argv[5])
        if sys.argv[1]=='measure':measure(paths,output)
        elif sys.argv[1]=='verify':verify(paths,json.loads(output.read_text()))
