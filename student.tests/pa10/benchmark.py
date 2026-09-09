#!/usr/bin/env python3
"""Fixed PA10 compiler and generated-program evidence; see performance-protocol.md."""
from pathlib import Path
import hashlib
import json
import os
import platform
import statistics
import subprocess
import sys
import time
ROOT=Path(__file__).resolve().parents[2]
ORDER=[0,0,0,0,0,1,1,0,0,1,1,0]
def sha(path): return hashlib.sha256(Path(path).read_bytes()).hexdigest()
def text_size(path):
    rows=subprocess.check_output(['size','-A',path],text=True).splitlines()
    return next(int(r.split()[1]) for r in rows if r.startswith('.text '))
def workloads():
    for scale in (1,4):
        n=3500*scale
        source=''.join(f'namespace N{i}{{int add(int x){{return x+2;}} int run(int n){{int s=0;for(int j=0;j<n;++j)s+=add(j);return s;}}}}\n' for i in range(n))
        yield f'calls-{scale}',source,'--emit-lowir',scale
        source=''.join(f'double sum{i}(double*p,int n){{double s=0.;for(int j=0;j<n;++j)s+=p[j]*2.;return s;}}\n' for i in range(n))
        yield f'memory-float-{scale}',source,'--emit-lowir',scale
        source='namespace N{const int value=7;const int&r0=value;\n'
        source+=''.join(f'const int&r{i}=r{i-1};\n' for i in range(1,800*scale))+'}\nint main(){return N::r0-7;}\n'
        yield f'references-{scale}',source,'--emit-lowir',scale
        source='template<class T>void consume(T);\n'+''.join(f'void run{i}(){{consume(1);}}\n' for i in range(n))
        yield f'template-semantics-{scale}',source,'--emit-semantics',scale

def runtimes():
    n=6000000
    # The sequence repeats every 1024; summation is checked modulo 65536.
    cycle=sum((i*17+3)&1023 for i in range(1024))
    expected=((n//1024)*cycle+sum((i*17+3)&1023 for i in range(n%1024)))&65535
    yield 'calls',f'int step(int x){{return (x*17+3)&1023;}} int main(){{volatile int n={n};int s=0;for(int i=0;i<n;++i)s=(s+step(i))&65535;return s=={expected}?0:1;}}'
    n=4000000
    expected=sum((i+n//64+(i<n%64))&4095 for i in range(64))
    yield 'memory',f'int main(){{int a[64];for(int i=0;i<64;++i)a[i]=i;volatile int n={n};for(int i=0;i<n;++i){{int j=i&63;a[j]=(a[j]+1)&4095;}}int sum=0;for(int i=0;i<64;++i)sum+=a[i];return sum=={expected}?0:1;}}'
    yield 'floating','double step(double x){return x+0.125;} int main(){volatile int n=2000000;double sum=0.;for(int i=0;i<n;++i)sum=step(sum);return sum==250000.0?0:1;}'

def run(cmd,**kw):
    r=subprocess.run([str(x) for x in cmd],capture_output=True,text=True,timeout=300,**kw)
    assert r.returncode==0,(cmd,r.returncode,r.stderr,r.stdout)
    return r

def measure(a,b,dest,work):
    work.mkdir(parents=True,exist_ok=True)
    binaries=[a.resolve(),b.resolve()]
    cpu=min(os.sched_getaffinity(0));os.sched_setaffinity(0,{cpu})
    result=dict(protocol='performance-protocol.md',order=ORDER,cpu=cpu,platform=platform.platform(),
        compiler_flags='g++ -std=gnu++11 -Wall -O3; course TEST_RUNNER_ENABLE',
        host=run(['g++','--version']).stdout.splitlines()[0],
        implementation_commits=['17f3deb7',run(['git','rev-parse','HEAD'],cwd=ROOT).stdout.strip()],
        binaries=[dict(path=str(p),sha256=sha(p),text_bytes=text_size(p)) for p in binaries],
        inputs={},observations=[],runtime=[],startup=[],budgets=dict(wall_ratio=1.10,rss_ratio=1.20,rss_add_kib=16384,text_add_bytes=131072,scale_wall=5.5,scale_rss=5.0),
        source_hashes={str(p.relative_to(ROOT)):sha(p) for p in Path(__file__).parent.glob('*.py')})
    def save():dest.write_text(json.dumps(result,indent=2)+'\n')
    def observe(cmd):
        usage=work/'usage.txt'
        start=time.perf_counter_ns()
        r=run(['/usr/bin/time','-f','%M %U %S %c %w','-o',usage,*cmd])
        wall=(time.perf_counter_ns()-start)/1e9
        rss,user,system,involuntary,voluntary=usage.read_text().split()
        return dict(wall_s=wall,rss_kib=int(rss),user_s=float(user),system_s=float(system),
                    involuntary=int(involuntary),voluntary=int(voluntary),stderr=r.stderr)
    empty=work/'empty.cpp';empty.write_text('int main(){return 0;}')
    for label in (0,1):
        for _ in range(4):
            r=observe([binaries[label],'--emit-lowir','-O0','-o',work/'empty.lowir',empty]);r['binary']=label;result['startup'].append(r)
    for name,source,mode,scale in workloads():
        src=work/f'{name}.t';src.write_text(source)
        outputs=[work/f'{name}.ref',work/f'{name}.my']
        flags=['-O0'] if mode=='--emit-lowir' else []
        for label in (0,1):
            run([binaries[label],mode,*flags,'-o',outputs[label],src])
            Path(str(outputs[label])+'.exit_status').write_text('EXIT_SUCCESS\n')
        if mode=='--emit-semantics':assert outputs[0].read_bytes()==outputs[1].read_bytes()
        else:run([ROOT/'pa10/scripts/compare_results.pl','ref','my',src],cwd=ROOT/'pa10')
        result['inputs'][name]=dict(path=str(src),sha256=sha(src),scale=scale,mode=mode,
            output_hashes=[sha(p) for p in outputs],output_bytes=[p.stat().st_size for p in outputs])
        for ordinal,label in enumerate(ORDER):
            r=observe([binaries[label],mode,*flags,'-o',outputs[label],src])
            assert not r['stderr'],r['stderr']
            r.update(group=name,ordinal=ordinal,binary=label);result['observations'].append(r)
        stats=observe([binaries[1],mode,*flags,'--stats','-o',outputs[1],src])
        stats['phases']=[json.loads(line) for line in stats.pop('stderr').splitlines()]
        result['inputs'][name]['telemetry']=stats
        save();print('measured compiler',name,flush=True)
    for name,source in runtimes():
        src=work/f'runtime-{name}.cpp';src.write_text(source)
        executables=[]
        for label in (0,1):
            ir=work/f'runtime-{name}-{label}.lowir';exe=work/f'runtime-{name}-{label}'
            run([binaries[label],'--emit-lowir','-O0','-o',ir,src])
            run([ROOT/'dev/lowir2native-ref','-O0','-o',exe,ir]);run([exe]);executables.append(exe)
        entry=dict(group=name,source_path=str(src),source_sha256=sha(src),
            executables=[dict(path=str(p),sha256=sha(p),text_bytes=text_size(p)) for p in executables],observations=[])
        for ordinal,label in enumerate(ORDER):
            r=observe([executables[label]]);r.update(binary=label,ordinal=ordinal);entry['observations'].append(r)
        result['runtime'].append(entry);save();print('measured runtime',name,flush=True)
    report(result)

def ratios(rows):
    paired=[]
    for first in (4,8):
        block=rows[first:first+4]
        paired.append(statistics.mean(r['wall_s'] for r in block if r['binary']==1)/statistics.mean(r['wall_s'] for r in block if r['binary']==0))
    aa=[r['wall_s'] for r in rows[:4]]
    return dict(paired_ratios=paired,aa_spread=(max(aa)-min(aa))/statistics.mean(aa),
        median_A=statistics.median(r['wall_s'] for r in rows if r['binary']==0),median_B=statistics.median(r['wall_s'] for r in rows if r['binary']==1),
        rss_A=max(r['rss_kib'] for r in rows if r['binary']==0),rss_B=max(r['rss_kib'] for r in rows if r['binary']==1))
def report(data):
    for group in data['inputs']:
        print(group,json.dumps(ratios([r for r in data['observations'] if r['group']==group])))
    for entry in data['runtime']:print('runtime',entry['group'],json.dumps(ratios(entry['observations'])),entry['executables'])
    print('compiler binaries',data['binaries'])
def verify(data):
    for b in data['binaries']:assert sha(b['path'])==b['sha256']
    for name,entry in data['inputs'].items():
        assert sha(entry['path'])==entry['sha256']
        rows=[r for r in data['observations'] if r['group']==name]
        assert [r['binary'] for r in rows]==ORDER and all(r['wall_s']>0 and r['rss_kib']>0 for r in rows)
    for e in data['runtime']:
        assert sha(e['source_path'])==e['source_sha256']
        assert [r['binary'] for r in e['observations']]==ORDER
        for b in e['executables']:assert sha(b['path'])==b['sha256'];run([b['path']])
    print('PASS: frozen artifacts, observation completeness, runtime outputs')
    report(data)
if __name__=='__main__':
    if sys.argv[1]=='measure': measure(Path(sys.argv[2]),Path(sys.argv[3]),Path(sys.argv[4]),Path(sys.argv[5]).resolve())
    elif sys.argv[1]=='verify':verify(json.loads(Path(sys.argv[2]).read_text()))
    else:report(json.loads(Path(sys.argv[2]).read_text()))
