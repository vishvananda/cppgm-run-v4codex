#!/usr/bin/env python3
"""Batch short translation units to expose fixed host-environment setup costs."""
import hashlib,json,pathlib,resource,statistics,subprocess,sys,time
if sys.argv[1]=='child':
    binary,source,obj=sys.argv[2:5]; samples=[]
    for _ in range(64):
        start=time.perf_counter();r=subprocess.run([binary,'-O0','-c','-o',obj,source],capture_output=True)
        assert r.returncode==0,r.stderr.decode();samples.append(time.perf_counter()-start)
    print(json.dumps({'samples_s':samples,'child_tree_peak_rss_kib':resource.getrusage(resource.RUSAGE_CHILDREN).ru_maxrss}));sys.exit()
out=pathlib.Path(sys.argv[1]).resolve();out.mkdir(parents=True,exist_ok=True)
binaries={k:str(pathlib.Path(v).resolve()) for k,v in zip('AB',sys.argv[2:4])}
source=out/'short.cpp';source.write_text('int f(int n){return n*7+3;}int main(int argc,char**){return f(argc)==10?0:1;}\n')
def sha(p):return hashlib.sha256(pathlib.Path(p).read_bytes()).hexdigest()
result={'binaries':{k:{'path':v,'sha256':sha(v)} for k,v in binaries.items()},'input_sha256':sha(source),
        'flags':['-O0','-c'],'batch_count':64,'runs':[],'images':{}}
for k,binary in binaries.items():
    obj=out/(k+'.o');exe=out/k
    subprocess.run([binary,'-O0','-c','-o',obj,source],check=True)
    subprocess.run(['g++',obj,'-o',exe],check=True);subprocess.run([exe],check=True)
    result['images'][k]={'object_sha256':sha(obj),'executable_sha256':sha(exe)}
assert result['images']['A']==result['images']['B']
for block,order in enumerate(['AAAA']+['ABBA']*6):
    for label in order:
        start=time.perf_counter()
        r=subprocess.run([sys.executable,__file__,'child',binaries[label],str(source),str(out/'measure.o')],capture_output=True,text=True,check=True)
        row=json.loads(r.stdout);row.update(block=block,label=label,wall_s=time.perf_counter()-start)
        result['runs'].append(row);(out/'startup.json').write_text(json.dumps(result,indent=2)+'\n')
rows=result['runs'];ratios=[]
for b in range(1,7):
    means=[statistics.mean(r['wall_s'] for r in rows if r['block']==b and r['label']==k) for k in 'AB'];ratios.append(means[1]/means[0])
summary={'AA_range_s':[min(r['wall_s'] for r in rows if r['block']==0),max(r['wall_s'] for r in rows if r['block']==0)],
         'paired_ratio_median':statistics.median(ratios),'paired_ratio_range':[min(ratios),max(ratios)]}
for k in 'AB':
    selected=[r for r in rows if r['label']==k and r['block']]
    summary[k]={'median_batch_s':statistics.median(r['wall_s'] for r in selected),
                'batch_min_max_s':[min(r['wall_s'] for r in selected),max(r['wall_s'] for r in selected)],
                'child_tree_peak_rss_kib':max(r['child_tree_peak_rss_kib'] for r in selected)}
result['summary']=summary
(out/'startup.json').write_text(json.dumps(result,indent=2)+'\n')
subprocess.run([sys.executable,str(pathlib.Path(__file__).with_name('startup_rss143.py')),str(out)],check=True)

