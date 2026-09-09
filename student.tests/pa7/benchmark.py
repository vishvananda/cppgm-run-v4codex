#!/usr/bin/env python3
"""Frozen PA6 A/B and PA7 absolute/scaling evidence; no executable is produced.
Usage: benchmark.py measure|verify BASE FINAL observations.json
       benchmark.py measure|verify BEFORE FINAL observations.json --audit-delta
       Optional for measure/verify: --prefix=GROUP --repeat-factor=N
       benchmark.py report observations.json
"""
import hashlib
import importlib.util
import json
import os
import pathlib
import platform
import statistics as st
import subprocess
import sys
import tempfile
import time
ROOT = pathlib.Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT/'student.tests/pa6'))
import measure as prior
from bench_inputs import workloads as inherited
ORDERS = prior.ORDERS
BUDGETS = dict(wall_percent=10, rss_percent=20, rss_allowance_kib=1024,
               text_percent=50, scaling_wall=6, scaling_rss=5, startup_multiple=20)
def workloads(audit_delta=False, prefix='', repeat_factor=1):
    result = {} if audit_delta else inherited()
    for scale in (1, 4):
        count = 1800*scale
        calls = ''.join(f'namespace N{i}{{int pick(int);long pick(long);int run(short x){{int(*p)(int)=pick;return p(x)+pick(x);}}}}\n' for i in range(count))
        memory = ''.join(f'double sum{i}(double*p,int n){{double x=0.;for(int i=0;i<n;++i){{x+=p[i]*2.;}}return x;}}\n' for i in range(count))
        templates = 'template<class T>void target(T);template<class T>void consume(T);\n'
        templates += ''.join(f'void run{i}(){{consume(static_cast<void(*)(int)>(&target<int>));}}\n' for i in range(count*2))
        for group, source in [('calls', calls), ('memory-float-loops', memory), ('template-demand', templates)]:
            result[f'semantics-{group}-{scale}'] = dict(source=source, mode='--emit-semantics', repeats=4, scale=scale, group=group)
    result = {name:case for name,case in result.items() if name.startswith(prefix)}
    for case in result.values(): case['repeats'] *= repeat_factor
    return result

def measure(paths, output, audit_delta=False, prefix='', repeat_factor=1):
    paths = [p.resolve() for p in paths]
    cpu = min(os.sched_getaffinity(0)); os.sched_setaffinity(0, {cpu})
    data = dict(budgets=BUDGETS, protocol=ORDERS, platform=platform.platform(), cpu=cpu,
                host_flags='g++ -std=gnu++11 -Wall -O3; TEST_RUNNER_ENABLE',
                host_cxx=subprocess.check_output(['g++','--version'],text=True).splitlines()[0],
                source_commits=[subprocess.check_output(['git','rev-parse','682532c07' if audit_delta else 'c02f4ea09'],text=True).strip(),subprocess.check_output(['git','rev-parse','HEAD'],text=True).strip()],
                binaries=[dict(path=str(p),sha256=prior.sha(p),text_bytes=prior.text_size(p)) for p in paths],
                inputs={}, observations=[], startup=[], work=[], telemetry=[],
                audit_delta=audit_delta,
                prefix=prefix,
                repeat_factor=repeat_factor,
                semantics_comparison='Frozen pre-audit PA7 versus audited PA7' if audit_delta else 'A and B are the same final binary: absolute cost, noise and scaling only',
                generated_runtime=None, generated_text=None)
    with tempfile.TemporaryDirectory(prefix='pa7-timing-') as directory:
        directory=pathlib.Path(directory); out=directory/'out'; rss=directory/'rss'; empty=directory/'empty.cc';empty.write_text('')
        def observe(binary, mode, src, repeats, stats=False):
            cmd=['/usr/bin/time','-f','%M %U %S %c %w','-o',rss,paths[binary],mode]
            if stats:cmd+=['--stats']
            cmd+=['-o',out,*([src]*repeats)]
            start=time.perf_counter_ns();run=subprocess.run(cmd,capture_output=True,text=True,timeout=180)
            wall=(time.perf_counter_ns()-start)/1e9
            assert run.returncode==0,(src,run.stderr)
            assert stats or not run.stderr,run.stderr
            usage=rss.read_text().split()
            return dict(binary=binary,mode=mode,stats=stats,wall_s=wall,rss_kib=int(usage[0]),
                        user_s=float(usage[1]),system_s=float(usage[2]),
                        involuntary_switches=int(usage[3]),voluntary_switches=int(usage[4]),
                        output_sha256=prior.sha(out), output_bytes=out.stat().st_size,
                        phases=[json.loads(x) for x in run.stderr.splitlines()] if stats else [])
        for mode in ['--emit-ast','--emit-types','--emit-semantics']:
            for label in 'ABABABAB':
                row=observe(1 if label=='B' or (mode=='--emit-semantics' and not audit_delta) else 0,mode,empty,4*repeat_factor)
                row['variant']=label;data['startup'].append(row)
        for name,case in sorted(workloads(audit_delta,prefix,repeat_factor).items()):
            src=directory/(name+'.cc');src.write_text(case['source'])
            data['inputs'][name]={k:v for k,v in case.items() if k!='source'}
            data['inputs'][name].update(sha256=prior.sha(src),bytes=src.stat().st_size)
            expected=None
            for block,order in ORDERS:
                for ordinal,label in enumerate(order):
                    binary=1 if label=='B' or (case['mode']=='--emit-semantics' and not audit_delta) else 0
                    row=observe(binary,case['mode'],src,case['repeats'])
                    expected=expected or row['output_sha256'];assert row['output_sha256']==expected,(name,'outputs differ')
                    row.update(input=name,variant=label,block=block,ordinal=ordinal);data['observations'].append(row)
                output.write_text(json.dumps(data,indent=2)+'\n');print(name,block,'complete',flush=True)
            for label in 'AB':
                row=observe(1 if label=='B' or (case['mode']=='--emit-semantics' and not audit_delta) else 0,case['mode'],src,case['repeats'],True)
                assert row['output_sha256']==expected
                row.update(input=name,variant=label);data['work'].append(row)
            if name=='semantics-template-demand-4':
                for block,order in ORDERS:
                    for ordinal,label in enumerate(order):
                        row=observe(1,case['mode'],src,case['repeats'],label=='B')
                        assert row['output_sha256']==expected
                        row.update(input=name,variant=label,block=block,ordinal=ordinal);data['telemetry'].append(row)
            output.write_text(json.dumps(data,indent=2)+'\n')
    assert all(prior.sha(p)==b['sha256'] for p,b in zip(paths,data['binaries']))

def verify(paths,data,audit_delta=False,prefix='',repeat_factor=1,recheck=None):
    assert data.get('audit_delta',False)==audit_delta
    assert data.get('prefix','')==prefix
    assert data.get('repeat_factor',1)==repeat_factor
    assert data['budgets']==BUDGETS and data['protocol']==[list(x) for x in ORDERS]
    assert all(prior.sha(p)==b['sha256'] and prior.text_size(p)==b['text_bytes'] for p,b in zip(paths,data['binaries']))
    assert data['binaries'][1]['text_bytes']<=data['binaries'][0]['text_bytes']*1.5
    cases=workloads(audit_delta,prefix,repeat_factor);assert cases and set(data['inputs'])==set(cases)
    telemetry_count=14 if 'semantics-template-demand-4' in cases else 0
    assert len(data['observations'])==14*len(cases) and len(data['startup'])==24 and len(data['work'])==2*len(cases) and len(data['telemetry'])==telemetry_count
    for mode in ['--emit-ast','--emit-types','--emit-semantics']:
        startup=[r for r in data['startup'] if r['mode']==mode]
        assert ''.join(r['variant'] for r in startup)=='ABABABAB'
        assert all(not r['stats'] and not r['phases'] and r['binary']==(1 if r['variant']=='B' or (mode=='--emit-semantics' and not audit_delta) else 0) for r in startup)
        assert len({r['output_sha256'] for r in startup})==1
    summaries={};failures=[]
    for name,case in cases.items():
        assert hashlib.sha256(case['source'].encode()).hexdigest()==data['inputs'][name]['sha256']
        assert all(data['inputs'][name][k]==v for k,v in case.items() if k!='source')
        assert len(case['source'].encode())==data['inputs'][name]['bytes']
        rows=[r for r in data['observations'] if r['input']==name]
        assert all(r['mode']==case['mode'] and not r['stats'] and not r['phases'] and r['binary']==(1 if r['variant']=='B' or (case['mode']=='--emit-semantics' and not audit_delta) else 0) for r in rows)
        assert len({r['output_sha256'] for r in rows})==1
        summary=prior.summarize(rows);summaries[name]=summary
        startup=max(r['wall_s'] for r in data['startup'] if r['mode']==case['mode'])
        if min(r['wall_s'] for r in rows)<=startup*20:failures.append((name,'startup dominance'))
        for pair in summary['paired']:
            if pair['wall_ratio']>1.1+summary['noise_percent']/100:failures.append((name,'wall',pair['wall_ratio'],summary['noise_percent']))
            if pair['B_rss']>pair['A_rss']*1.2+1024:failures.append((name,'rss',pair))
        work=[r for r in data['work'] if r['input']==name]
        assert len(work)==2 and all(r['output_sha256']==rows[0]['output_sha256'] and r['stats'] and len(r['phases'])==case['repeats'] for r in work)
        if case['mode']=='--emit-semantics':
            for phase in work[1]['phases']:
                assert phase['semantic_expression_work']<=phase['nodes']
                if 'semantic_conversions' in phase:
                    assert phase['semantic_conversions']<=3*phase['nodes']
                if 'semantic_dependence_work' in phase:
                    assert phase['semantic_dependence_work']<=phase['semantic_types']
                assert phase['semantic_constant_work']<=phase['nodes']
                if case['group']=='template-demand':
                    assert phase['semantic_specializations']==2 and phase['semantic_argument_packs']==2
                assert phase['semantic_demand_processed']==phase['semantic_member_demands']
    for name,case in cases.items():
        if case['scale']!=4:continue
        small=summaries[name[:-1]+'1']['B'];large=summaries[name]['B']
        if large['wall_s']>=small['wall_s']*6:failures.append((name,'wall scaling'))
        if large['rss_kib']>=small['rss_kib']*5+1024:failures.append((name,'rss scaling'))
    if data['telemetry']:
        prior.summarize(data['telemetry'])
        reference=next(r for r in data['observations'] if r['input']=='semantics-template-demand-4')
    for row in data['telemetry']:
        assert row['input']=='semantics-template-demand-4' and row['binary']==1 and row['mode']=='--emit-semantics'
        assert row['stats']==(row['variant']=='B') and len(row['phases'])==(cases['semantics-template-demand-4']['repeats'] if row['stats'] else 0)
        assert row['output_sha256']==reference['output_sha256'] and row['output_bytes']==reference['output_bytes']
    assert data['generated_runtime'] is None and data['generated_text'] is None
    if recheck is not None:
        # A longer frozen rerun can supersede timing for complete input groups.
        # Validate both raw records; never suppress identity/output/work failures
        # or silently drop another workload's failed budget.
        assert recheck.get('audit_delta',False)==audit_delta
        assert recheck.get('repeat_factor',1)>repeat_factor
        assert recheck['source_commits']==data['source_commits']
        for key in ('host_flags','host_cxx','platform','cpu','binaries'):
            assert recheck[key]==data[key],key
        assert set(recheck['inputs'])<=set(cases)
        for name,case in recheck['inputs'].items():
            assert all(case[key]==data['inputs'][name][key] for key in case if key!='repeats')
        verify(paths,recheck,audit_delta,recheck.get('prefix',''),recheck.get('repeat_factor',1))
        superseded=[failure for failure in failures if failure[0] in recheck['inputs']]
        print('Retained earlier timing failures superseded by the verified longer group:',superseded)
        failures=[failure for failure in failures if failure[0] not in recheck['inputs']]
    assert not failures,failures
    print('PA7 compiler evidence protocol, identities, outputs, work bounds and budgets passed')

if __name__=='__main__':
    if sys.argv[1]=='report':prior.report(json.loads(pathlib.Path(sys.argv[2]).read_text()))
    else:
        paths=[pathlib.Path(x) for x in sys.argv[2:4]];output=pathlib.Path(sys.argv[4])
        prefix=next((x.split('=',1)[1] for x in sys.argv if x.startswith('--prefix=')),'')
        factor=int(next((x.split('=',1)[1] for x in sys.argv if x.startswith('--repeat-factor=')),'1'))
        assert factor>=1
        if sys.argv[1]=='measure':measure(paths,output,'--audit-delta' in sys.argv,prefix,factor)
        elif sys.argv[1]=='verify':
            recheck=next((x.split('=',1)[1] for x in sys.argv if x.startswith('--recheck=')),None)
            verify(paths,json.loads(output.read_text()),'--audit-delta' in sys.argv,prefix,factor,
                   json.loads(pathlib.Path(recheck).read_text()) if recheck else None)
