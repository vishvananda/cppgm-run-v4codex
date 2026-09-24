#!/usr/bin/env python3
"""Check that completing one class invalidates only its query consumers."""
from pathlib import Path
import hashlib,json,subprocess,sys
cc=Path(sys.argv[1]).resolve();work=Path(sys.argv[2]);work.mkdir(parents=True,exist_ok=True)
rows=[]
for n in (32,128,512):
 source='template<class T,int=sizeof(T)>char f(T*);long f(...);'
 source+=''.join(f'struct C{i};using Before{i}=decltype(f((C{i}*)0));static_assert(sizeof(Before{i})==sizeof(long), "");' for i in range(n))
 source+='struct C0{};using After=decltype(f((C0*)0));static_assert(sizeof(After)==sizeof(char), "");int main(){}'
 src=work/f'{n}.cpp';src.write_text(source)
 r=subprocess.run([cc,'--emit-lowir','-O0','--validate-lowir','--stats','-o',work/f'{n}.lowir',src],capture_output=True,text=True,timeout=30)
 assert r.returncode==0,r.stderr
 stats=[json.loads(l) for l in r.stderr.splitlines()];semantic=next(x for x in stats if 'query_completion_edges' in x)
 rows.append(dict(count=n,source=source,source_sha256=hashlib.sha256(source.encode()).hexdigest(),telemetry=stats,invalidations=semantic['query_completion_invalidations'],edges=semantic['query_completion_edges'],compiler_exit=r.returncode))
 print(n,rows[-1]['edges'],rows[-1]['invalidations'])
(work/'results.json').write_text(json.dumps(rows,indent=2)+'\n')
assert 0<rows[0]['invalidations']==rows[1]['invalidations']==rows[2]['invalidations']
assert rows[2]['edges']<=16*rows[0]['edges']
