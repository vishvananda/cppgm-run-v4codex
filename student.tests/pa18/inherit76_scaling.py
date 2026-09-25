#!/usr/bin/env python3
"""Assert linear forwarding work and dormant bodies from retained telemetry: CC WORK."""
from pathlib import Path
import hashlib,json,subprocess,sys
cc=Path(sys.argv[1]).resolve();work=Path(sys.argv[2]);work.mkdir(parents=True,exist_ok=True)
rows=[]
for family in ('declaration','query','body','reuse'):
 for n in (32,128,512):
  body=':n(a){}' if family=='body' else '{typename T::missing x;}'
  source='template<int I>struct B{int n;template<class T>B(T a)noexcept '+body+'};template<int I>struct D:B<I>{using B<I>::B;};'
  if family=='declaration':source+=''.join(f'static_assert(sizeof(D<{i}>)==4,"");' for i in range(n))
  elif family=='body':source+=''.join(f'int f{i}(){{D<{i}> d({i});return d.n!={i};}}' for i in range(n))
  else:source+=''.join(f'static_assert(noexcept(D<{0 if family=="reuse" else i}>(1)),"");' for i in range(n))
  source+='int main(){return '+('+'.join(f'f{i}()' for i in range(n)) if family=='body' else '0')+';}'
  src=work/f'{family}-{n}.cpp';src.write_text(source);ir=src.with_suffix('.lowir')
  p=subprocess.run([cc,'--emit-lowir','-O0','--validate-lowir','--stats','-o',ir,src],text=True,capture_output=True,timeout=60)
  assert p.returncode==0,(family,n,p.stderr)
  telemetry=[json.loads(s) for s in p.stderr.splitlines()];stats=telemetry[0]
  expected=0 if family=='declaration' else 1 if family=='reuse' else n
  assert stats['semantic_inherited_arguments']==expected,(family,n,stats)
  assert stats['template_body_transitions']==(n if family=='body' else 0),(family,n,stats)
  row=dict(family=family,n=n,source=source,source_sha256=hashlib.sha256(source.encode()).hexdigest(),telemetry=telemetry,checked_exit=0)
  rows.append(row);print(family,n,'arguments',expected,'bodies',stats['template_body_transitions'],flush=True)
(work/'results.json').write_text(json.dumps(rows,indent=2)+'\n')
