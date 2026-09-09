#!/usr/bin/env python3
"""Fixed PA8 AAAA noise + two ABBA blocks, compiler and executable evidence.
measure BASE FINAL JSON [--base-count=N] [--runtime-factor=N]
verify JSON
Retains every observation; frozen binaries and generated artifacts stay in /tmp.
"""
from pathlib import Path
import argparse,datetime,hashlib,json,os,platform,statistics,struct,subprocess,sys,time
from bench_inputs import compiler_workloads,runtime_source
ROOT=Path(__file__).resolve().parents[2]
BUDGETS=dict(compiler_wall_percent=10,compiler_rss_percent=20,rss_allowance_kib=1024,
             host_text_percent=25,scaling_wall=6,scaling_rss=5,startup_multiple=20,
             runtime_percent=5,native_text_percent=0)
ORDERS=['AAAA','ABBA','ABBA']
def sha(p):return hashlib.sha256(Path(p).read_bytes()).hexdigest()
def host_text(p):
    rows=subprocess.check_output(['size','-A',p],text=True).splitlines()
    return sum(int(r.split()[1]) for r in rows if r.split() and r.split()[0]=='.text')
def native_text(p):
    # Supplied native backend emits sectionless ELF. These workloads define no
    # static data, so its load payload from entrypoint onward is code + padding.
    b=Path(p).read_bytes();assert b[:5]==b'\x7fELF\x02'
    entry,phoff=struct.unpack_from('<QQ',b,24)
    entsize,count=struct.unpack_from('<HH',b,54)
    assert count==1
    kind,flags,offset,address,_,size,memsize,align=struct.unpack_from('<IIQQQQQQ',b,phoff)
    assert kind==1 and flags&1 and memsize==size and offset<=entry-address+offset<=len(b)
    return size-(entry-address)
def observe(cmd,rss,output=None):
    start=time.perf_counter_ns()
    run=subprocess.run(['/usr/bin/time','-f','%M %U %S %c %w','-o',rss,*cmd],capture_output=True,text=True,timeout=180)
    row=dict(wall_s=(time.perf_counter_ns()-start)/1e9,returncode=run.returncode,stdout=run.stdout,stderr=run.stderr)
    usage=rss.read_text().split()
    if len(usage)==5:
        row.update(rss_kib=int(usage[0]),user_s=float(usage[1]),system_s=float(usage[2]),involuntary_switches=int(usage[3]),voluntary_switches=int(usage[4]))
    if output and Path(output).exists():row.update(output_sha256=sha(output),output_bytes=Path(output).stat().st_size)
    return row

def measure(base,final,destination,base_count=6000,runtime_factor=20,base_commit='ef37a2c9e'):
    cpu=min(os.sched_getaffinity(0));os.sched_setaffinity(0,{cpu})
    binaries=[base.resolve(),final.resolve()]
    root=Path('/tmp/pa8-evidence')/destination.stem;root.mkdir(parents=True,exist_ok=True)
    rss=root/'rss';output=root/'output.lowir'
    source_files=[ROOT/'dev/lowir.cpp',ROOT/'dev/frontend_source_sets.mk',*sorted((ROOT/'dev/src/lowir').glob('*')),
                  *[ROOT/'dev/src'/p for p in ('ir_symbol_model.h','preprocess/source.h','preprocess/source.cpp',
                    'preprocess/identifier_table.h','preprocess/identifier_table.cpp','support/tool_help_text.h',
                    'support/testing/test_runner.cpp')]]
    data=dict(budgets=BUDGETS,orders=ORDERS,base_count=base_count,runtime_factor=runtime_factor,
              measured_at=datetime.datetime.now(datetime.timezone.utc).isoformat(),
              cpu=cpu,platform=platform.platform(),processor=Path('/proc/cpuinfo').read_text().split('model name')[1].split('\n')[0],
              host_compiler=subprocess.check_output(['g++','--version'],text=True).splitlines()[0],
              compiler_flags='-std=gnu++11 -Wall -O3; TEST_RUNNER_ENABLE',native_flags='-O0',
              source_commits=[subprocess.check_output(['git','rev-parse',base_commit],text=True).strip(),subprocess.check_output(['git','rev-parse','HEAD'],text=True).strip()],
              source_hashes={str(p.relative_to(ROOT)):sha(p) for p in source_files},
              harness_hashes={str(p.relative_to(ROOT)):sha(p) for p in (Path(__file__),Path(__file__).with_name('bench_inputs.py'))},
              native_reference_manifest=sha(ROOT/'reference-binaries/manifest.tsv'),
              binaries=[dict(path=str(p),sha256=sha(p),text_bytes=host_text(p)) for p in binaries],
              startup=[],inputs={},compiler=[],telemetry=[],executables={},backend=[],runtime=[],runtime_startup=[])
    def save():destination.write_text(json.dumps(data,indent=2)+'\n')
    def checked(row):
        assert row['returncode']==0 and not row['stdout'],row
    empty=root/'empty.lowir';empty.write_text('')
    for label in 'ABABABAB':
        variant=ord(label)-ord('A');row=observe([binaries[variant],'-o',output,empty],rss,output)
        row['variant']=label;data['startup'].append(row);save();checked(row)
    for name,case in compiler_workloads(base_count):
        source=root/(name+'.lowir');source.write_text(case['source'])
        data['inputs'][name]={k:v for k,v in case.items() if k!='source'}
        data['inputs'][name].update(path=str(source),sha256=sha(source),bytes=source.stat().st_size)
        for block,order in enumerate(ORDERS):
            for position,label in enumerate(order):
                variant=ord(label)-ord('A');row=observe([binaries[variant],'-o',output,source],rss,output)
                row.update(case=name,block=block,position=position,variant=label)
                data['compiler'].append(row);save();checked(row);assert not row['stderr'],row
        for label in 'ABBA':
            variant=ord(label)-ord('A');row=observe([binaries[variant],'--stats','-o',output,source],rss,output)
            row.update(case=name,variant=label);data['telemetry'].append(row);save();checked(row)
            row['phases']=json.loads(row['stderr']);save()
        print('compiler',name,flush=True)
    for group in ('sum','swap','call','floating'):
        count=1000001*runtime_factor if group=='sum' else 1000000*runtime_factor
        harness=root/(group+'-runtime.lowir');harness.write_text(runtime_source(group,count))
        assert 'global' not in harness.read_text()
        record=dict(iterations=count,harness_path=str(harness),harness_sha256=sha(harness),
                    input_description='volatile loop counter and local memory; helper inputs vary per iteration; checked accumulator; exit 0',
                    text_measure='ELF load payload from entrypoint, excludes headers; no static data; includes code padding',variants={})
        data['executables'][group]=record
        for variant,label in enumerate('AB'):
            inputs=[]
            if group!='floating':
                helper=root/(group+'-'+label+'-helper.lowir')
                row=observe([binaries[variant],'--exercise',group,'-o',helper],rss,helper);checked(row)
                record['variants'][label]=dict(helper_sha256=sha(helper),construction=row);inputs.append(helper)
            else:record['variants'][label]={}
            inputs.append(harness)
            joined=root/(group+'-'+label+'.lowir')
            row=observe([binaries[variant],'-o',joined,*inputs],rss,joined);checked(row)
            record['variants'][label].update(lowir_sha256=sha(joined),adapter=row)
            program=root/(group+'-'+label+'.program')
            row=observe([ROOT/'dev/lowir2native-ref','-O0','-o',program,joined],rss,program)
            row.update(case=group,variant=label);data['backend'].append(row);save();checked(row)
            record['variants'][label].update(path=str(program),sha256=sha(program),text_bytes=native_text(program),file_bytes=program.stat().st_size)
        for block,order in enumerate(ORDERS):
            for position,label in enumerate(order):
                row=observe([record['variants'][label]['path']],rss)
                row.update(case=group,block=block,position=position,variant=label)
                data['runtime'].append(row);save();checked(row)
        print('runtime',group,flush=True)
    trivial=root/'startup.lowir';trivial.write_text('function @main()->i64 {block ^entry:return i64 0}')
    native=root/'startup.program';subprocess.run([ROOT/'dev/lowir2native-ref','-O0','-o',native,trivial],check=True)
    for _ in range(8):data['runtime_startup'].append(observe([native],rss));save()
    save()
    verify(destination)

def med(rows,key='wall_s'):return statistics.median(x[key] for x in rows)
def results(rows):
    aa=[r for r in rows if r['block']==0]
    noise=(max(r['wall_s'] for r in aa)/min(r['wall_s'] for r in aa)-1)*100
    pairs=[]
    for block in (1,2):
        a=[r for r in rows if r['block']==block and r['variant']=='A']
        b=[r for r in rows if r['block']==block and r['variant']=='B']
        pairs.append((med(b)/med(a)-1)*100)
    return dict(noise_percent=noise,paired_wall_percent=pairs,mean_wall_percent=statistics.mean(pairs),
                a_wall_s=med([r for r in rows if r['variant']=='A' and r['block']]),
                b_wall_s=med([r for r in rows if r['variant']=='B']),
                a_rss_kib=med([r for r in rows if r['variant']=='A' and r['block']], 'rss_kib'),
                b_rss_kib=med([r for r in rows if r['variant']=='B'],'rss_kib'))
def verify(path):
    d=json.loads(path.read_text());failures=[];summary=dict(compiler={},runtime={},scaling={})
    def check(condition,message):
        if not condition:failures.append(message)
    check(d['budgets']==BUDGETS and d['orders']==ORDERS,'protocol or budgets changed')
    expected=[(block,position,label) for block,order in enumerate(ORDERS) for position,label in enumerate(order)]
    def protocol(rows,name):
        check([(r['block'],r['position'],r['variant']) for r in rows]==expected,'observation order '+name)
        check(all(r['returncode']==0 and not r['stdout'] and not r['stderr'] for r in rows),'unsuccessful observation '+name)
    for b in d['binaries']:
        check(sha(b['path'])==b['sha256'],'binary changed '+b['path'])
        check(host_text(b['path'])==b['text_bytes'],'host text changed '+b['path'])
    check(sha(ROOT/'reference-binaries/manifest.tsv')==d['native_reference_manifest'],'backend manifest changed')
    for p,h in d.get('harness_hashes',{}).items():check(sha(ROOT/p)==h,'harness changed '+p)
    for name,case in d['inputs'].items():
        check(sha(case['path'])==case['sha256'],'input changed '+name)
        rows=[r for r in d['compiler'] if r['case']==name]
        protocol(rows,name)
        check(len(rows)==12,'missing compiler observations '+name)
        check(len({r['output_sha256'] for r in rows})==1,'compiler output mismatch '+name)
        r=results(rows);summary['compiler'][name]=r
        check(r['mean_wall_percent']<=BUDGETS['compiler_wall_percent']+r['noise_percent'],'compiler latency budget '+name)
        check(r['b_rss_kib']<=r['a_rss_kib']*1.2+1024,'compiler RSS budget '+name)
        check(min(x['wall_s'] for x in rows)>med(d['startup'])*20,'compiler samples too short '+name)
        telemetry=[r for r in d['telemetry'] if r['case']==name]
        check(len(telemetry)==4,'missing telemetry '+name)
        for t in telemetry:
            check(t['output_sha256']==rows[0]['output_sha256'],'telemetry changes output '+name)
            check(t['phases']['instructions']==t['phases']['validated_instructions'],'missing validation work '+name)
        r['telemetry_overhead_percent']={v:(med([x for x in telemetry if x['variant']==v])/r[v.lower()+'_wall_s']-1)*100 for v in 'AB'}
    for group in sorted({v['group'] for v in d['inputs'].values()}):
        a=summary['compiler'][group+'-1'];b=summary['compiler'][group+'-4']
        summary['scaling'][group]=dict(wall=b['b_wall_s']/a['b_wall_s'],rss=b['b_rss_kib']/a['b_rss_kib'])
        check(b['b_wall_s']<6*a['b_wall_s'],'wall scaling '+group)
        check(b['b_rss_kib']<5*a['b_rss_kib']+1024,'RSS scaling '+group)
    check(d['binaries'][1]['text_bytes']<=d['binaries'][0]['text_bytes']*1.25,'host text growth')
    for group,case in d['executables'].items():
        rows=[r for r in d['runtime'] if r['case']==group];r=results(rows);summary['runtime'][group]=r
        protocol(rows,group)
        check(sha(case['harness_path'])==case['harness_sha256'],'native harness changed '+group)
        check(len(rows)==12,'missing runtime observations '+group)
        check(all(x['returncode']==0 and not x['stdout'] for x in rows),'runtime result '+group)
        check(min(x['wall_s'] for x in rows)>20*med(d['runtime_startup']),'runtime samples too short '+group)
        check(r['mean_wall_percent']<=5+r['noise_percent'],'runtime budget '+group)
        a,b=(case['variants'][v] for v in 'AB')
        check(a['sha256']==b['sha256'],'executable mismatch '+group)
        check(b['text_bytes']<=a['text_bytes'],'native text growth '+group)
        for v in (a,b):
            check(sha(v['path'])==v['sha256'],'executable changed '+group)
            check(native_text(v['path'])==v['text_bytes'],'native text changed '+group)
    for p,h in d['source_hashes'].items():check(sha(ROOT/p)==h,'source changed '+p)
    summary['failures']=failures
    print(json.dumps(summary,indent=2))
    assert not failures,failures
    return summary
if __name__=='__main__':
    if sys.argv[1]=='verify':verify(Path(sys.argv[2]))
    elif sys.argv[1]=='measure':
        parser=argparse.ArgumentParser()
        parser.add_argument('--base-count',type=int,default=6000)
        parser.add_argument('--runtime-factor',type=int,default=20)
        parser.add_argument('--base-commit',default='ef37a2c9e')
        measure(Path(sys.argv[2]),Path(sys.argv[3]),Path(sys.argv[4]),**vars(parser.parse_args(sys.argv[5:])))
