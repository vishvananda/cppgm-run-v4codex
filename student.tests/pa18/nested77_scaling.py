#!/usr/bin/env python3
"""Demand counts and source-region bounds, independent of wall time: CC WORK."""
from pathlib import Path
import hashlib,json,subprocess,sys
cc=Path(sys.argv[1]).resolve();work=Path(sys.argv[2]);work.mkdir(parents=True,exist_ok=True)
rows=[]
for family in ('dormant','layout','reuse'):
 for width in (4,64,256):
  for n in (32,128):
   source='template<int I>struct O{struct N{'+''.join(f'int f{i};' for i in range(width))+'};};'
   source+=''.join('static_assert(sizeof(O<'+str(0 if family=='reuse' else i)+'>'+('::N' if family!='dormant' else '')+')=='+str(width*4 if family!='dormant' else 1)+',"");' for i in range(n))
   source+='int main(){}'
   src=work/f'{family}-{width}-{n}.cpp';src.write_text(source)
   p=subprocess.run([cc,'--emit-lowir','-O0','--validate-lowir','--stats','-o',src.with_suffix('.lowir'),src],capture_output=True,text=True,timeout=60)
   assert p.returncode==0,(family,width,n,p.stderr)
   stats=[json.loads(s) for s in p.stderr.splitlines()]
   t=stats[0];decls=1 if family=='reuse' else n;defs=0 if family=='dormant' else decls
   assert t['nested_class_declarations']==decls and t['nested_class_definitions']==defs,(family,width,n,t)
   assert t['template_class_completions']==decls and t['template_body_transitions']==0
   rows.append(dict(family=family,width=width,n=n,source=source,source_sha256=hashlib.sha256(source.encode()).hexdigest(),telemetry=stats,checked_exit=0))
   print(family,width,n,'declarations',decls,'definitions',defs,'occurrences',t['template_occurrences'],flush=True)
for n in (32,128):
 counts=[r['telemetry'][0]['template_occurrences'] for r in rows if r['family']=='dormant' and r['n']==n]
 assert len(set(counts))==1,counts
for width in (4,64,256):
 counts=[r['telemetry'][0]['template_occurrences'] for r in rows if r['family']=='reuse' and r['width']==width]
 assert len(set(counts))==1,counts
for n in (32,128,512):
 source='template<class T,int=sizeof(T)>char f(T*);long f(...);template<int I>struct O{struct N;};'
 source+=''.join(f'using Before{i}=decltype(f((O<{i}>::N*)0));static_assert(sizeof(Before{i})==sizeof(long),"");' for i in range(n))
 source+='template<>struct O<0>::N{};using After=decltype(f((O<0>::N*)0));static_assert(sizeof(After)==sizeof(char),"");int main(){}'
 src=work/f'completion-{n}.cpp';src.write_text(source)
 p=subprocess.run([cc,'--emit-lowir','-O0','--validate-lowir','--stats','-o',src.with_suffix('.lowir'),src],capture_output=True,text=True,timeout=60)
 assert p.returncode==0,(n,p.stderr)
 stats=[json.loads(s) for s in p.stderr.splitlines()]
 assert stats[0]['query_completion_invalidations']==1,(n,stats)
 rows.append(dict(family='completion',n=n,source=source,source_sha256=hashlib.sha256(source.encode()).hexdigest(),telemetry=stats,checked_exit=0))
 print('completion',n,'invalidations',stats[0]['query_completion_invalidations'],flush=True)
(work/'results.json').write_text(json.dumps(rows,indent=2)+'\n')
