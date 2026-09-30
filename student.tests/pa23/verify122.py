#!/usr/bin/env python3
"""PA23 lifecycle entries, key-owned tables, separate TUs and repeated-base RTTI."""
from pathlib import Path
import hashlib,json,re,subprocess,sys
ROOT=Path(__file__).resolve().parents[2]
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
cases={}
def add(name,source):cases[name]=[source]
add('delete-secondary-order','int trace;struct A{virtual ~A(){trace=trace*10+1;}};struct B{virtual ~B(){trace=trace*10+2;}};struct D:A,B{~D(){trace=trace*10+3;}};int main(){B*p=new D;delete p;return trace!=321;}')
add('implicit-delete-two-bases','int trace;struct A{virtual ~A(){trace=trace*10+1;}};struct B{virtual ~B(){trace=trace*10+2;}};struct D:A,B{};int main(){B*p=new D;delete p;return trace!=21;}')
add('implicit-delete-three-bases','int trace;struct A{virtual ~A(){trace=trace*10+1;}};struct B{virtual ~B(){trace=trace*10+2;}};struct C{virtual ~C(){trace=trace*10+3;}};struct D:A,B,C{};int main(){C*p=new D;delete p;return trace!=321;}')
add('delete-virtual-during-destruction','int trace;struct A{virtual int f(){return 1;}virtual ~A(){trace=trace*10+f();}};struct B{virtual int g(){return 2;}virtual ~B(){trace=trace*10+g();}};struct D:A,B{int f(){return 3;}int g(){return 4;}~D(){A&a=*this;B&b=*this;trace=a.f()*10+b.g();}};int main(){B*p=new D;delete p;return trace!=3421;}')
add('delete-throwing-secondary','int trace;int freed;void operator delete(void*)noexcept;struct A{virtual ~A()noexcept(false){trace=trace*10+1;}};struct B{virtual ~B()noexcept(false){trace=trace*10+2;throw 9;}};struct D:A,B{static void operator delete(void*p)noexcept{++freed;::operator delete(p);}};int main(){B*p=new D;try{delete p;return 1;}catch(int n){return n!=9||trace!=21||freed!=1;}}')
add('key-definition-after-use','struct A{virtual int f();};struct B{virtual int g();};struct D:A,B{int f();int g();};int run(){D d;B&b=d;return b.g();}int A::f(){return 1;}int B::g(){return 2;}int D::f(){return 3;}int D::g(){return 7;}int main(){return run()!=7;}')
add('inline-key-definition-after-use','struct A{virtual int f();};struct B{virtual int g();};struct D:A,B{int g();};int run(){D d;B&b=d;return b.g();}inline int A::f(){return 1;}inline int B::g(){return 2;}inline int D::g(){return 7;}int main(){return run()!=7;}')
add('template-key-demand','template<int I>struct A{virtual int f();};template<int I>int A<I>::f(){return I;}template<int I>struct B{virtual int g();};template<int I>int B<I>::g(){return I+1;}struct D:A<2>,B<6>{};int main(){D d;B<6>&b=d;return b.g()!=7;}')
add('key-unused-body','struct A{virtual int f();};int A::f(){return 7;}int main(){return 0;}')
add('repeated-destructor-base',(ROOT/'student.tests/pa23/reducers/repeated-destructor-base.cpp').read_text())
header='struct A{virtual int f();virtual ~A();};struct B{virtual int g();virtual ~B();};struct D:A,B{int f();int g();~D();};'
owner=header+'int A::f(){return 1;}A::~A(){}int B::g(){return 2;}B::~B(){}int D::f(){return 3;}int D::g(){return 7;}D::~D(){}'
cases['separate-key-owner']=[header+'int main(){D d;B&b=d;return b.g()!=7;}',owner]
cases['separate-delete-owner']=[header+'int main(){B*b=new D;int n=b->g();delete b;return n!=7;}',owner]
header2='struct A{virtual int f(){return 1;}};struct B{virtual int g(){return 2;}};struct Mid:A,B{virtual int h(){return 3;}};struct P{virtual int p(){return 4;}};struct D:P,Mid{int g();};'
cases['separate-nested-secondary']=[header2+'int main(){D d;B&b=d;Mid&m=d;return b.g()!=7||m.h()!=3;}',header2+'int D::g(){return 7;}']
for count in (4,16,64):
 source='int destroyed;'+''.join('struct B%d{virtual ~B%d(){++destroyed;}};'%(i,i) for i in range(count))
 source+='struct D:'+','.join('B%d'%i for i in range(count))+'{};'
 add('destructor-scaling-%d'%count,source+'int main(){B%d*p=new D;delete p;return destroyed!=%d;}'%(count-1,count))
def check(cc,work):
 work.mkdir(parents=True,exist_ok=True);rows=[]
 for name,sources in cases.items():
  row=dict(name=name,sources=sources,units=[]);objects=[]
  for i,source in enumerate(sources):
   src=work/(name+str(i)+'.cpp');src.write_text(source);ir=src.with_suffix('.lowir');obj=src.with_suffix('.o')
   command=[str(cc),'--emit-lowir','-O0','--validate-lowir','-o',str(ir),str(src)]
   p=subprocess.run(command,capture_output=True,text=True)
   unit=dict(compile_exit=p.returncode,diagnostic=p.stderr);row['units'].append(unit)
   if p.returncode:break
   raw=ir.read_bytes();q=subprocess.run(command+['--stats'],capture_output=True,text=True)
   unit.update(lowir_sha256=sha(ir),stats_identical=q.returncode==0 and raw==ir.read_bytes(),telemetry=[json.loads(x) for x in q.stderr.splitlines() if x.startswith('{')])
   unit['vtable_definitions']=len(re.findall(r'^global @[^\n]*object=_ZTV',ir.read_text(),re.M))
   unit['vtable_declarations']=len(re.findall(r'^declare global @[^\n]*object=_ZTV',ir.read_text(),re.M))
   unit['instruction_count']=len(re.findall(r'^    ',ir.read_text(),re.M))
   p=subprocess.run([str(ROOT/'dev/cppgm++-ref'),'-c','-O0','-o',str(obj),str(ir)],capture_output=True,text=True)
   unit.update(backend_exit=p.returncode,backend_diagnostic=p.stderr)
   if p.returncode:break
   objects.append(obj)
  if len(objects)==len(sources):
   exe=work/(name+'.exe');p=subprocess.run(['g++','-no-pie',*[str(x) for x in objects],'-o',str(exe)],capture_output=True,text=True)
   row.update(link_exit=p.returncode,link_diagnostic=p.stderr)
   if not p.returncode:row['runtime_exit']=subprocess.run([str(exe)],timeout=20).returncode
  if len(sources)>1:
   ir=work/(name+'.merged.lowir');obj=ir.with_suffix('.o');exe=ir.with_suffix('.exe')
   commands=[[str(cc),'--emit-lowir','-O0','--validate-lowir','-o',str(ir),*[str(work/(name+str(i)+'.cpp')) for i in range(len(sources))]],
    [str(ROOT/'dev/cppgm++-ref'),'-c','-O0','-o',str(obj),str(ir)],['g++','-no-pie',str(obj),'-o',str(exe)],[str(exe)]]
   row['merged']=[]
   for command in commands:
    q=subprocess.run(command,capture_output=True,text=True,timeout=30)
    row['merged'].append(dict(exit=q.returncode,diagnostic=q.stderr))
    if q.returncode:break
  row['passed']=row.get('runtime_exit')==0 and all(x.get('stats_identical') for x in row['units']) and all(x['exit']==0 for x in row.get('merged',[]));rows.append(row)
  print(name, 'PASS' if row['passed'] else 'FAIL',file=sys.stderr,flush=True)
 # Pin the corrected field without deriving an expected answer from our compiler.
 ref=ROOT/'pa23/tests/general/100-diamond-virtual-destructor-slot-merge.ref'
 flags=re.search(r'global @__rtti_class_D .*?\n  i32 (\d+)',ref.read_text(),re.S)
 assert flags and flags.group(1)=='1'
 observed=work/'reducer.reference.lowir'
 q=subprocess.run([str(ROOT/'dev/cppgm++-ref'),'--emit-lowir','-O0','-o',str(observed),str(ROOT/'student.tests/pa23/reducers/repeated-destructor-base.cpp')],capture_output=True,text=True)
 assert q.returncode==0,q.stderr
 match=re.search(r'global @[^\n]*object=_ZTI1D[^\n]*\n(?:[^\n]*\n){2}  i32 (\d+)',observed.read_text())
 assert match and match.group(1)=='0'
 return dict(compiler=str(cc),compiler_sha256=sha(cc),cases=rows,reference_correction=dict(observed_reducer_flags=0,reference_reducer_sha256=sha(observed),bundle_revision='c2f713cd70d06170632bfde3e75dd6fe1aa44d98',oracle=str(ref.relative_to(ROOT)),sha256=sha(ref),vmi_flags=1))
if __name__=='__main__':
 cc,work=map(lambda x:Path(x).resolve(),sys.argv[1:3]);result=check(cc,work)
 print(json.dumps(result,indent=2));sys.exit(not all(r['passed'] for r in result['cases']))
