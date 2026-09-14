#!/usr/bin/env python3
"""Explicit PA16 static-order and automatic constant-data controls. CC WORK [--reference]."""
from pathlib import Path
import hashlib,json,re,subprocess,sys
ROOT=Path(__file__).resolve().parents[2]
CC=Path(sys.argv[1]).resolve();WORK=Path(sys.argv[2]).resolve();WORK.mkdir(parents=True,exist_ok=True)
REFERENCE='--reference' in sys.argv[3:]
def sha(p):return hashlib.sha256(Path(p).read_bytes()).hexdigest()
GOOD={
'array_identity': ('int f(int i){int a[3]={1,2};int b[3]={1,2};a[i]=9;return a!=b&&b[0]==1&&b[1]==2&&b[2]==0&&a[i]==9?0:1;}int main(){return f(0)||f(2);}',2,1),
'array_nested': ('int main(){int a[2][3]={{1,2},{3}};return a[0][1]+a[1][0]+a[1][2]-5;}',1,1),
'array_zero': ('int read(int i){int a[1000]={};return a[i];}int main(){return read(0)||read(999);}',1,1),
'array_string': ('int main(){char a[]="abc";char b[]="abc";a[0]=120;return a!=b&&b[0]==97&&a[0]==120&&a[3]==0?0:1;}',2,1),
'array_scalar_conversion': ('struct B{constexpr operator int()const{return 4;}};int main(){int a[3]={B(),B()};return a[0]+a[1]+a[2]-8;}',1,1),
'array_float': ('int main(){double a[]={0.0,-0.0};double b[]={0.0,0.0};return 1.0/a[1]<0&&1.0/b[1]>0?0:1;}',2,2),
'array_enum': ('enum E{a=2,b=5};int main(){E x[]={a,b};return x[0]+x[1]-7;}',1,1),
'array_nullptr': ('int main(){decltype(nullptr) a[2]={nullptr};return a[0]==nullptr&&a[1]==nullptr?0:1;}',1,1),
'array_bool': ('int main(){bool a[]={false,true,bool(0.5)};return !a[0]&&a[1]&&a[2]?0:1;}',1,1),
'array_pointer_addends': ('int x[3]={2,4,6};int main(){int*p[]={x,x+1};int*q[]={x,x+2};return *p[1]+*q[1]-10;}',2,2),
'array_pointer_omission': ('int x;int main(){int*p[3]={&x};return p[0]==&x&&p[1]==nullptr&&p[2]==nullptr?0:1;}',1,1),
'array_function_pointer': ('int f(int n){return n+1;}int main(){int(*p[2])(int)={f};return p[0](3)==4&&p[1]==nullptr?0:1;}',1,1),
'array_static_local_pointer': ('int f(){static int n=3;int*p[]={&n};return ++*p[0];}int main(){return f()==4&&f()==5?0:1;}',1,1),
'array_local_const': ('int main(){const int n=7;int a[]={n,n+1};return a[0]+a[1]-15;}',1,1),
'array_parameter': ('int f(int n){int a[]={n,2};return a[0]+a[1];}int main(){return f(9)-11;}',0,0),
'array_auto_address': ('int main(){int n=4;int*p[]={&n};return *p[0]-4;}',0,0),
'array_side_effect': ('int calls;int f(){return ++calls;}int main(){int a[]={f(),f()};return calls==2&&a[0]==1&&a[1]==2?0:1;}',0,0),
'array_volatile': ('int main(){volatile int a[]={1,2};return a[0]+a[1]-3;}',0,0),
'array_nonconstant_probe': ('int calls;int f(){return ++calls;}constexpr int g(){int unused=f();return 3;}int main(){int a[]={g()};return a[0]==3&&calls==1?0:1;}',0,0),
'array_class_lifetime': ('int alive;struct C{int n;C(int n):n(n){++alive;}~C(){--alive;}};int f(){C a[]={1,2};return alive==2&&a[1].n==2?0:1;}int main(){int n=f();return n||alive;}',None,0),
}
BAD={'array_required_nonconstant':'int f();constexpr int a[]={f()};',
     'array_required_address':'constexpr const int*f(){int n=4;return &n;}constexpr const int*a[]={f()};'}
rows=[]
def run(name,source,good=True,copies=None,images=None,reference=False):
 src=WORK/(name+'.cpp');src.write_text(source);ir=src.with_suffix('.lowir');exe=src.with_suffix('.exe')
 cc=ROOT/'dev/cppgm++-ref' if reference else CC
 r=subprocess.run([cc,'--emit-lowir','-O0',*([] if reference else ['--validate-lowir']),'-o',ir,src],capture_output=True,text=True,timeout=30)
 row=dict(name=name,reference=reference,source_sha256=sha(src),compile_exit=r.returncode,diagnostic=r.stderr)
 ok=(r.returncode==0)==good
 if not r.returncode:
  text=ir.read_text();row['ir_sha256']=sha(ir)
  row['copies']=len(re.findall(r'^    copyobj ',text,re.M));row['readonly_images']=text.count('storage=readonly')
  if copies is not None:ok &= row['copies']==copies
  if images is not None:ok &= row['readonly_images']==images
  r=subprocess.run([ROOT/'dev/lowir2native-ref','-O0','-o',exe,ir],capture_output=True,text=True,timeout=30)
  row['backend_exit']=r.returncode;ok &= r.returncode==0
  if not r.returncode:
   row['native_sha256']=sha(exe)
   r=subprocess.run([exe],capture_output=True,timeout=10);row['native_exit']=r.returncode
   ok &= r.returncode==0
 if reference:row['expected_reference_failure']=True;ok=row.get('compile_exit')==0 and row.get('backend_exit')==0 and row.get('native_exit',0)!=0
 row['passed']=bool(ok);rows.append(row);print(name,'reference' if reference else 'student','PASS' if ok else 'FAIL',flush=True)
for name,(source,copies,images) in GOOD.items():run(name,source,copies=copies,images=images)
for name,source in BAD.items():run(name,source,False)
for src in sorted((ROOT/'student.tests/pa16/initialization').glob('order_*.cpp')):
 run(src.stem,src.read_text())
 if REFERENCE:run(src.stem+'_reference',src.read_text(),reference=True)
array=ROOT/'student.tests/pa16/initialization/automatic_array.cpp'
run('automatic_array_reducer',array.read_text(),copies=2,images=1)
local=ROOT/'student.tests/pa16/initialization/local_order.cpp'
run('local_order',local.read_text(),copies=0)
if REFERENCE:run('local_order_reference',local.read_text(),reference=True)
(WORK/'results.json').write_text(json.dumps(dict(compiler_sha256=sha(CC),rows=rows),indent=2)+'\n')
assert all(r['passed'] for r in rows),[r['name'] for r in rows if not r['passed']]
