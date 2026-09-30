#!/usr/bin/env python3
"""Execute shared layouts/access and inspect table ownership using student LowIR."""
from pathlib import Path
import hashlib,json,re,subprocess,sys
ROOT=Path(__file__).resolve().parents[2]
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
cases={
 'covariant-virtual-tail':'struct Pad{int p;};struct R{int r;};struct V:Pad,R{};struct Result:virtual V{int x;};struct More:Result{long y;};More value;struct B{virtual R*get(){return 0;}};struct D:B{Result*get(){return &value;}};int main(){D d;B&b=d;return b.get()!=static_cast<R*>(&value);}',

 'inherited-secondary-override-slot':'struct V{virtual int f(){return 1;}};struct A:virtual V{};struct B:virtual V{int f(){return 7;}};struct D:A,B{};int main(){D d;V&v=d;return v.f()!=7||d.f()!=7;}',
 'inherited-virtual-override-slot':'struct V{virtual int f(){return 1;}};struct B:V{int f(){return 7;}};struct D:virtual B{};int main(){D d;V&v=d;return v.f()!=7||d.f()!=7;}',

 'covariant-virtual-result':"struct R{int r;};struct Result:virtual R{int x;};struct More:Result{long y;};More value;struct B{virtual R*get(){return 0;}};struct D:B{Result*get(){return &value;}};int main(){D d;B&b=d;return b.get()!=static_cast<R*>(&value)||d.get()!=static_cast<Result*>(&value);}",
 'covariant-virtual-null':"struct R{int r;};struct Result:virtual R{int x;};struct More:Result{long y;};More value;struct B{virtual R*get(){return 0;}};struct D:B{Result*get(){return 0;}};int main(){D d;B&b=d;return b.get()!=0;}",
 'covariant-virtual-reference':"struct R{int r;};struct Result:virtual R{int x;};struct More:Result{long y;};More value;struct B{virtual R&get()=0;};struct D:B{Result&get(){return value;}};int main(){D d;B&b=d;return &b.get()!=static_cast<R*>(&value);}",
 'nonpoly-reference-reducer':(ROOT/'student.tests/pa23/reducers/nonpoly-virtual-reference.cpp').read_text(),
 'shared-nonpoly-field':'struct V{int x;};struct A:virtual V{};struct B:virtual V{};struct D:A,B{};int read(A&a){return a.x;}int main(){D d;d.x=7;B&b=d;return read(d)!=7||b.x!=7||static_cast<V*>(&b)!=static_cast<V*>(&d);}',
 'secondary-shared-pointer':'struct V{int x;};struct A:virtual V{};struct B:virtual V{};struct D:A,B{};int read(B*b){return b?b->x:0;}int main(){D d;d.x=9;return read(&d)!=9||read(0)!=0;}',
 'nested-shared-field':'struct V{int x;};struct A:virtual V{};struct B:virtual A{};struct C:virtual A{};struct D:B,C{};int read(C*c){return c->x;}int main(){D d;d.x=11;return read(&d)!=11||static_cast<V*>(static_cast<B*>(&d))!=static_cast<V*>(static_cast<C*>(&d));}',
 'three-virtual-rows':'struct X{int x;};struct Y{long y;};struct Z{int z;};struct D:virtual X,virtual Y,virtual Z{};int sum(D&d){return d.x+d.y+d.z;}int main(){D d;d.x=1;d.y=2;d.z=4;return sum(d)!=7;}',
 'nonpoly-typeid':'namespace std{class type_info{public:bool operator==(const type_info&)const;};}struct V{int x;};struct B:virtual V{};struct D:B{};int main(){D d;B&b=d;return !(typeid(b)==typeid(B));}',
 'single-virtual-dispatch':'struct V{virtual int f(){return 1;}};struct D:virtual V{virtual int g(){return 3;}int f(){return 7;}};int main(){D d;V&v=d;return v.f()!=7||d.g()!=3;}',
 'shared-final-overrider':'struct V{virtual int f(){return 1;}};struct A:virtual V{int f(){return 7;}};struct B:virtual V{};struct D:A,B{};int main(){D d;V&v=d;return v.f()!=7||d.f()!=7;}',
 'shared-rtti':'namespace std{class type_info{public:bool operator==(const type_info&)const;};}struct V{virtual int f(){return 1;}};struct A:virtual V{};struct B:virtual V{};struct D:A,B{};int main(){D d;V*v=&d;return dynamic_cast<B*>(v)!=static_cast<B*>(&d)||!(typeid(*v)==typeid(D));}',
 'virtual-base-alignment':'struct V{long double x;};struct A:virtual V{};struct B:virtual V{};struct D:A,B{int x;};int read(B&b){return b.x==3.0L;}int main(){D d;d.x=9;V&v=d;v.x=3.0L;return !read(d)||d.x!=9;}',
 'nonvirtual-tail-in-virtual-base':'struct R{int x;};struct V:R{int y;};struct A:virtual V{};struct B:virtual V{};struct D:A,B{};int read(B&b){return b.x+b.y;}int main(){D d;d.x=2;d.y=5;return read(d)!=7;}',
}
# These cross the next owner boundary and deliberately remain recorded failures.
unfinished={
 'construct-shared-once':'int count;struct V{V(){++count;}virtual void f(){}};struct A:virtual V{};struct B:virtual V{};struct D:A,B{};int main(){D d;return count!=1;}',
 'construct-hidden-vptr-target':(ROOT/'pa23/tests/general/100-virtual-base-constructor-vptr-hidden-target.t').read_text(),
}
def check(cc,work):
 work.mkdir(parents=True,exist_ok=True);rows=[]
 for name,source in {**cases,**unfinished}.items():
  src=work/(name+'.cpp');src.write_text(source);ir=src.with_suffix('.lowir');obj=src.with_suffix('.o');exe=src.with_suffix('.exe')
  command=[str(cc),'--emit-lowir','-O0','--validate-lowir','-o',str(ir),str(src)]
  p=subprocess.run(command,capture_output=True,text=True);row=dict(name=name,source=source,compile_exit=p.returncode,diagnostic=p.stderr,unfinished=name in unfinished)
  if not p.returncode:
   raw=ir.read_bytes();q=subprocess.run(command+['--stats'],capture_output=True,text=True)
   row.update(lowir_sha256=sha(ir),stats_identical=q.returncode==0 and raw==ir.read_bytes(),telemetry=[json.loads(x) for x in q.stderr.splitlines() if x.startswith('{')])
   text=ir.read_text();row['tables']=[]
   for match in re.finditer(r'^global ([^\n]+) = \{\n(.*?)^\}',text,re.M|re.S):
    if 'object=_ZTV' in match[1] or 'object=__cppgm_vtable_view_' in match[1] or 'object=_ZTI' in match[1]:row['tables'].append(dict(header=match[1],rows=match[2].splitlines()))
   if name=='shared-rtti':
    row['virtual_cast_unknown_hint']=bool(re.search(r'call ptr @[^\n]+, -1\)',text))
   if name in ('inherited-secondary-override-slot','inherited-virtual-override-slot'):
    primary=[t for t in row['tables'] if 'object=_ZTV1D]' in t['header']]
    row['required_primary_override_slot']=len(primary)==1 and sum(x.strip().startswith('ptr addr') for x in primary[0]['rows'])==2
   q=subprocess.run([str(ROOT/'dev/cppgm++-ref'),'-c','-O0','-o',str(obj),str(ir)],capture_output=True,text=True)
   row.update(backend_exit=q.returncode,backend_diagnostic=q.stderr)
   if not q.returncode:
    q=subprocess.run(['g++','-no-pie',str(obj),'-o',str(exe)],capture_output=True,text=True);row.update(link_exit=q.returncode,link_diagnostic=q.stderr)
    if not q.returncode:row['runtime_exit']=subprocess.run([str(exe)],timeout=15).returncode
  row['passed']=row.get('runtime_exit')==0 and row.get('stats_identical',False) and row.get('required_primary_override_slot',True) and row.get('virtual_cast_unknown_hint',True);rows.append(row)
  print(name,'PASS' if row['passed'] else 'FAIL', '(unfinished lifecycle)' if name in unfinished else '',file=sys.stderr,flush=True)
 return dict(compiler=str(cc),compiler_sha256=sha(cc),cases=rows)
if __name__=='__main__':
 cc,work=map(lambda x:Path(x).resolve(),sys.argv[1:3]);result=check(cc,work)
 print(json.dumps(result,indent=2));sys.exit(not all(r['passed'] for r in result['cases'] if not r['unfinished']))
