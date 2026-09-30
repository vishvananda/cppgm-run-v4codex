#!/usr/bin/env python3
"""Freeze inputs/binaries externally; retain every A/A and wall-time ABBA sample."""
import hashlib, json, os, pathlib, re, statistics, subprocess, sys, time
root=pathlib.Path(__file__).resolve().parents[2]
a,b,d=map(lambda s:pathlib.Path(s).resolve(),sys.argv[1:4]); d.mkdir(parents=True,exist_ok=True)
compilers={'A':a,'B':b}
# Keep every compiler and executable sample on one permitted logical CPU.
cpu=min(os.sched_getaffinity(0)); os.sched_setaffinity(0,{cpu})
def digest(p): return hashlib.sha256(p.read_bytes()).hexdigest()
def run(cmd, record):
    stamp=time.perf_counter()
    p=subprocess.run(['/usr/bin/time','-f','BENCH_RSS=%M',*map(str,cmd)],capture_output=True,text=True,check=True)
    record.update(wall_seconds=time.perf_counter()-stamp,rss_kib=int(re.search(r'BENCH_RSS=(\d+)',p.stderr)[1]))
    if p.stdout: raise AssertionError(p.stdout)
    for line in p.stderr.splitlines():
        if line.startswith('{'): record['stats']=json.loads(line)
        if line.startswith('native_stats '): record['stats']={k:float(v) for k,v in re.findall(r'(\w+)=([\d.e+-]+)',line)}
    return record
# Large typed LowIR input amortizes process startup and covers repeated per-function
# release. Every function contains real arithmetic; only one is the entry point.
floating_compiler=os.environ.get('PA24_FLOAT_COMPILER')=='1'
source=d/'compiler.lowir'
with source.open('w') as out:
    for f in range(4096):
        typ='f64' if floating_compiler else 'i128' if os.environ.get('PA24_WIDE_COMPILER')=='1' else 'i64'
        out.write(f'function @f{f}(%seed : {typ}) -> {typ} {{\nblock ^entry:\n')
        prior='%seed'
        for k in range(64):
            literal=str(k+1)+('.25' if floating_compiler else '')
            out.write(f'%v{k} = binary add {typ} {prior}, {literal}\n'); prior=f'%v{k}'
        out.write(f'jump ^next\nblock ^next:\nreturn {typ} {prior}\n}}\n')
    out.write(f'function @main() -> i64 [role=entry] {{\nblock ^entry:\n%x = call {typ} @f0(0)\n%bad = cmp ne {typ} %x, '+('2096.0' if floating_compiler else '2080')+'\nreturn i64 %bad\n}\n')
manifest={'binaries':{k:{'path':str(v),'sha256':digest(v)} for k,v in compilers.items()},'flags':['-O0','--stats'],
          'inputs':{'compiler':digest(source)},'runs':[], 'runtime_arguments':['input'], 'cpu':cpu, 'platform':os.uname()._asdict() if hasattr(os.uname(),'_asdict') else list(os.uname())}
# Compiler correctness is checked before timing; executables remain frozen for runtime.
executables={}
runtime_names=[] if os.environ.get('PA24_COMPILE_ONLY')=='1' else sys.argv[4:] or ['runtime','memory-runtime']
for name in runtime_names:
    input=d/f'{name}.lowir'; input.write_bytes((root/f'student.tests/pa24/{name}.lowir').read_bytes())
    manifest['inputs'][name]=digest(input)
    for label,cc in compilers.items():
        exe=d/f'{name}-{label}'
        rec=run([cc,'-O0','--stats','-o',exe,input],{'phase':'small_compile','input':name,'binary':label})
        manifest['runs'].append(rec); executables[name,label]=exe
        subprocess.run([str(exe),'input'],check=True)
        subprocess.run([str(exe),'input','second'],check=True)
        rec['executable_sha256']=digest(exe)
# A/A calibration followed by six independent ABBA blocks. Compile and run are
# measured separately and each sample's complete wall/RSS/telemetry is retained.
for phase in ['compile',*runtime_names]:
    for block,order in [('AA', 'AAAA'), *[(str(k),'ABBA') for k in range(6)]]:
        for label in order:
            rec={'phase':phase,'block':block,'binary':label}
            if phase=='compile':
                exe=d/f'compiler-{label}'
                run([compilers[label],'-O0','--stats','-o',exe,source],rec)
                subprocess.run([str(exe)],check=True)
            else: run([executables[phase,label],'input'],rec)
            manifest['runs'].append(rec)
    (d/'observations.json').write_text(json.dumps(manifest,indent=2)+'\n')
summary={}
for phase in ['compile',*runtime_names]:
    rows=[r for r in manifest['runs'] if r['phase']==phase]
    pairs=[]
    for k in range(6):
        group=[r for r in rows if r['block']==str(k)]
        av=statistics.mean(r['wall_seconds'] for r in group if r['binary']=='A')
        bv=statistics.mean(r['wall_seconds'] for r in group if r['binary']=='B')
        pairs.append(bv/av)
    summary[phase]={'paired_B_over_A':pairs,'median_ratio':statistics.median(pairs)}
    for label in ['A','B']:
        vals=[r['wall_seconds'] for r in rows if r['block']!='AA' and r['binary']==label]
        rss=[r['rss_kib'] for r in rows if r['block']!='AA' and r['binary']==label]
        summary[phase][label]={'wall_median':statistics.median(vals),'wall_range':[min(vals),max(vals)],'rss_max':max(rss)}
    noise=[r['wall_seconds'] for r in rows if r['block']=='AA']
    summary[phase]['AA_range']=[min(noise),max(noise)]
manifest['summary']=summary
(d/'observations.json').write_text(json.dumps(manifest,indent=2)+'\n')
print(json.dumps(summary,indent=2))
