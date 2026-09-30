#!/usr/bin/env python3
"""Freeze inputs and compare two correct PA25 driver binaries; preserve all samples."""
import hashlib, json, os, pathlib, statistics, struct, subprocess, sys, time
out = pathlib.Path(sys.argv[1]); out.mkdir(parents=True,exist_ok=True)
A,B = map(lambda x:pathlib.Path(x).resolve(),sys.argv[2:4])
inputs = out/'inputs'; inputs.mkdir(exist_ok=True)
def save(name,text):
    p=inputs/name; p.write_text(text); return p
# Demanded templates, independent TUs and many external fixups.
templates=[]
for tu in range(8):
    text='template<int N> int item(int x){return (x+N)%97;}\n'
    text+=f'int chunk{tu}(int x) {{ int result=0;\n'
    text+=''.join(f'result+=item<{n}>(x);\n' for n in range(tu*600,(tu+1)*600))
    text+='return result; }\n'; templates.append(save(f'template{tu}.cc',text))
expected=sum((x+n)%97 for x in range(400) for n in range(4800))
main=''.join(f'int chunk{i}(int);\n' for i in range(8))
main+='int main(int argc,char**){long long result=0; for(int x=0;x<argc*400;++x){'
main+=''.join(f'result+=chunk{i}(x);' for i in range(8))
main+=f'}} return result=={expected}LL ? 0 : 1;}}\n'
templates.append(save('template-main.cc',main))
# Runtime-driven loops/calls/memory/floating operations, checked against an
# independently computed value. No volatile/static compile-time-only workload.
count=3000000
values=[i for i in range(64)]; total=0
for i in range(count):
    k=i&63; values[k]=(values[k]+((k*17+(i&255))%101))&65535
    total=(total+values[k])%1009
memory=save('memory.cc',f'''int step(int x,int y){{return (x*17+y)%101;}}
int main(int argc,char**){{int values[64]; for(int i=0;i<64;++i) values[i]=i;
int total=0;for(int i=0;i<argc*{count};++i){{int k=i&63;values[k]=(values[k]+step(k,i&255))&65535;total=(total+values[k])%1009;}}
return total=={total}?0:1;}}
''')
x=0.; total=0
for i in range(count):
    x=x*.5+(i&127); total=(total+int(x))%1009
floating=save('floating.cc',f'''double step(double x,int i){{return x*0.5+(i&127);}}
int main(int argc,char**){{double x=0;int total=0;for(int i=0;i<argc*{count};++i){{x=step(x,i);total=(total+(int)x)%1009;}}return total=={total}?0:1;}}
''')
workloads={'templates':(templates,1),'memory':([memory],64),'floating':([floating],64)}
def digest(p): return hashlib.sha256(p.read_bytes()).hexdigest()
def run(args):
    p=subprocess.run(list(map(str,args)),stdout=subprocess.PIPE,stderr=subprocess.PIPE)
    if p.returncode: raise RuntimeError((args,p.returncode,p.stderr.decode(errors='replace')))
    return p
# Preserve ordinary ELF images from each frozen binary for equality and timing.
manifest={str(p):digest(p) for p in [A,B,*inputs.glob('*.cc')]}
results={'manifest':manifest,'flags':['-O0'],'policy':'O0; no optional transforms or speedup claim', 'runs':[],'images':{}}
for name,(sources,_) in workloads.items():
    images=[]
    for label,binary in [('A',A),('B',B)]:
        exe=out/(name+'-'+label); run([binary,'-O0','-o',exe,*sources]); run([exe]); images.append(exe)
    assert images[0].read_bytes()==images[1].read_bytes(),name
    data=images[0].read_bytes(); ph=struct.unpack_from('<Q',data,32)[0]
    # RX PT_LOAD includes the ELF header; generated instruction bytes follow it.
    text=struct.unpack_from('<Q',data,ph+32)[0]-176
    results['images'][name]={'sha256':digest(images[0]),'text_bytes':text,'file_bytes':len(data)}
    for mode in ('compile','runtime'):
        repeat=workloads[name][1] if mode=='compile' else 3
        for block,order in enumerate(['AAAA','ABBA','ABBA','ABBA','ABBA','ABBA','ABBA']):
            for label in order:
                binary=A if label=='A' else B
                rss=[]; started=time.perf_counter()
                for iteration in range(repeat):
                    timefile=out/'time.txt'
                    args=[binary,'-O0','-o',out/'measure.program',*sources] if mode=='compile' else [out/(name+'-'+label)]
                    run(['/usr/bin/time','-f','%M','-o',timefile,*args]); rss.append(int(timefile.read_text()))
                elapsed=time.perf_counter()-started
                results['runs'].append({'workload':name,'mode':mode,'block':block,'label':label,'repeat':repeat,'wall_s':elapsed,'peak_rss_kib':max(rss)})
                (out/'performance.json').write_text(json.dumps(results,indent=2)+'\n')
summary={}
for name in workloads:
    row={**results['images'][name]}
    for mode in ('compile','runtime'):
        sample=[x for x in results['runs'] if x['workload']==name and x['mode']==mode]
        pairs=[]
        for block in range(1,7):
            a=[x['wall_s'] for x in sample if x['block']==block and x['label']=='A']
            b=[x['wall_s'] for x in sample if x['block']==block and x['label']=='B']
            pairs.append(statistics.mean(b)/statistics.mean(a))
        noise=[x['wall_s'] for x in sample if x['block']==0]
        row[mode]={'ratio_median':statistics.median(pairs),'ratio_range':[min(pairs),max(pairs)],'AA_range_s':[min(noise),max(noise)],'B_median_s':statistics.median([x['wall_s'] for x in sample if x['label']=='B']),'B_peak_rss_kib':max(x['peak_rss_kib'] for x in sample if x['label']=='B')}
    summary[name]=row
results['summary']=summary
(out/'performance.json').write_text(json.dumps(results,indent=2)+'\n')
print(json.dumps(summary,indent=2))
