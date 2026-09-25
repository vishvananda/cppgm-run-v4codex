#!/usr/bin/env python3
"""List formation/validation completion edges and compressed array tails: CC WORK."""
from pathlib import Path
import hashlib,json,subprocess,sys
cc=Path(sys.argv[1]).resolve();work=Path(sys.argv[2]);work.mkdir(parents=True,exist_ok=True)
rows=[]
def run(name,source):
 src=work/(name+'.cpp');src.write_text(source)
 r=subprocess.run([cc,'--emit-lowir','-O0','--validate-lowir','--stats','-o',work/(name+'.lowir'),src],capture_output=True,text=True,timeout=60)
 row=dict(name=name,source=source,source_sha256=hashlib.sha256(source.encode()).hexdigest(),exit=r.returncode,diagnostic=r.stderr)
 rows.append(row);(work/'results.json').write_text(json.dumps(rows,indent=2)+'\n')
 assert r.returncode==0,row
 stats=[json.loads(l) for l in r.stderr.splitlines()];s=next(x for x in stats if 'query_completion_edges' in x)
 row['telemetry']=stats;row['invalidations']=s['query_completion_invalidations'];row['edges']=s['query_completion_edges'];row['plans']=s['semantic_list_plans'];row['fields']=s['semantic_list_fields']
 (work/'results.json').write_text(json.dumps(rows,indent=2)+'\n');return row
for n in (32,128,512):
 source='template<class T>auto f(int)->decltype(T{},char());template<class>long f(...);'
 source+=''.join(f'struct C{i};using Before{i}=decltype(f<C{i}>(0));static_assert(sizeof(Before{i})==sizeof(long),"");' for i in range(n))
 source+='struct C0{};using After=decltype(f<C0>(0));static_assert(sizeof(After)==1,"");int main(){}'
 print(run('completion-'+str(n),source)['invalidations'],flush=True)
assert 0<rows[0]['invalidations']==rows[1]['invalidations']==rows[2]['invalidations']
assert rows[2]['edges']<=16*rows[0]['edges']
for n in (32,4096,1048576):
 source=f'struct A{{int a[{n}];}};template<class T>auto f(int)->decltype(T{{1}},char());template<class>long f(...);static_assert(sizeof(f<A>(0))==1,"");int main(){{}}'
 print(run('omitted-'+str(n),source)['fields'],flush=True)
assert rows[3]['plans']==rows[4]['plans']==rows[5]['plans']
assert rows[3]['fields']==rows[4]['fields']==rows[5]['fields']
print('Completion invalidation is localized; omitted array tails retain constant graph size.')
