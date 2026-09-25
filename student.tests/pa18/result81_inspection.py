#!/usr/bin/env python3
"""Explicit summary/emission bounds and native trace: CC WORK."""
from pathlib import Path
import hashlib,json,subprocess,sys
ROOT=Path(__file__).resolve().parents[2];cc=Path(sys.argv[1]).resolve();work=Path(sys.argv[2]);work.mkdir(parents=True,exist_ok=True)
rows=[]
def run(name,source):
 src=work/(name+'.cpp');src.write_text(source);ir=src.with_suffix('.lowir');exe=src.with_suffix('.exe')
 r=subprocess.run([cc,'--emit-lowir','-O0','--stats','--validate-lowir','-o',ir,src],capture_output=True,text=True);assert r.returncode==0,(name,r.stderr)
 text=ir.read_text();stats={k:v for s in r.stderr.splitlines() for k,v in json.loads(s).items()}
 subprocess.run([ROOT/'dev/lowir2native-ref','-O0','-o',exe,ir],check=True,capture_output=True);subprocess.run([exe],check=True)
 rows.append(dict(name=name,source=source,source_sha256=hashlib.sha256(src.read_bytes()).hexdigest(),lowir_sha256=hashlib.sha256(ir.read_bytes()).hexdigest(),native_sha256=hashlib.sha256(exe.read_bytes()).hexdigest(),stats=stats,native_exit=0))
 return text,stats
for n in (1,600,2400):
 source='template<int N>struct X{static const int value=N;operator int(){return value;}};'+''.join(f'int f{i}(){{X<{i}>x;return x;}}' for i in range(n))+'int main(){return f0()!=0||f'+str(n-1)+'()!='+str(n-1)+';}'
 text,stats=run('scale-'+str(n),source)
 assert stats['semantic_conversion_result_work']==n,stats
 assert len([l for l in text.splitlines() if l.startswith('function ')])==n+1
 assert stats['instructions']<=4*n+25
# Actual use requires emission, while summary-only use leaves no unused body.
for direct in (False,True):
 source='struct X{static const int value=7;operator int(){return value;}};int main(){X x;int n=x;return n!=7'+('||x.operator int()!=7' if direct else '')+';}'
 text,stats=run('retained-'+str(direct),source)
 assert len([l for l in text.splitlines() if l.startswith('function ')])==1+direct
# The wrapper budget has a conservative fallback and never expands source work.
for n in (8,9,100):
 source='struct X{static const int value=7;operator int(){return '+'('*n+'value'+')'*n+';}};int main(){X x;return int(x)!=7;}'
 text,stats=run('budget-'+str(n),source)
 assert stats['semantic_conversion_result_work']<=9
 assert len([l for l in text.splitlines() if l.startswith('function ')])==1+(n>8)
run('trace',(ROOT/'student.tests/pa18/result81_trace.cpp').read_text())
(work/'results.json').write_text(json.dumps(rows,indent=2)+'\n');print('Inspection passed:',len(rows),'checked native programs; bounded summary work, emission and source trace.')
