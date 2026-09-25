#!/usr/bin/env python3
"""Assert fixed copy selection is shared and concrete lifetimes stay distinct."""
from pathlib import Path
import hashlib,json,subprocess,sys
R=Path(__file__).resolve().parents[2];CC=Path(sys.argv[1]).resolve();W=Path(sys.argv[2]).resolve();W.mkdir(parents=True,exist_ok=True);rows=[]
for form in ('(void)a;','a;','(a,1);','((a));','(void)(true?a:a);'):
 for count in (1,37,600):
  source='int copies,dtors;struct A{A(){}A(volatile A&){++copies;}~A(){++dtors;}};volatile A a;template<int N>void f(){'+form+'}'
  source+=''.join('void use'+str(i)+'(){f<'+str(i)+'>();}'for i in range(count))
  source+='int main(){use0();return copies!=1||dtors!=1;}'
  name='form'+str(('(void)a;','a;','(a,1);','((a));','(void)(true?a:a);').index(form))+'-'+str(count)
  p=W/(name+'.cpp');p.write_text(source);ir=p.with_suffix('.lowir');exe=p.with_suffix('.exe')
  r=subprocess.run([CC,'--emit-lowir','-O0','--validate-lowir','--stats','-o',ir,p],capture_output=True,text=True,timeout=60);assert r.returncode==0,(name,r.stderr)
  stats={k:v for line in r.stderr.splitlines() if line.startswith('{') for k,v in json.loads(line).items()}
  subprocess.run([R/'dev/lowir2native-ref','-O0','-o',exe,ir],check=True,capture_output=True,timeout=60)
  execution=subprocess.run([exe],timeout=10).returncode
  passed=execution==0 and stats['semantic_discard_selections']==1 and stats['semantic_discard_recipe_uses']==count and stats['semantic_discard_materializations']==count
  rows.append(dict(form=form,count=count,source_sha256=hashlib.sha256(source.encode()).hexdigest(),stats=stats,native_exit=execution,passed=passed))
  print(name,passed,{k:v for k,v in stats.items() if 'discard' in k},flush=True)
(W/'results.json').write_text(json.dumps(rows,indent=2)+'\n');assert all(r['passed']for r in rows)
