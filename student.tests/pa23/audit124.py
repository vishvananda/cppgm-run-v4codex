#!/usr/bin/env python3
"""Audit controls for program ABI ownership and completed base-path facts."""
from pathlib import Path
import collections, hashlib, json, re, subprocess, sys
ROOT = Path(__file__).resolve().parents[2]

def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()

h = 'struct A{virtual int a(){return 1;}};struct B{virtual int b(){return 2;}};struct D:A,B{int b(){return 7;}};'
cases = {'shared-thunk': [h+'int one(){D d;B&b=d;return b.b();}', h+'struct E:D{};int one();int main(){E e;B&b=e;return one()!=7||b.b()!=7;}']}
h = 'struct V{virtual int f(){return 1;}};struct D:virtual V{int f();};'
cases['virtual-key-owner'] = [h+'int main(){D d;V&v=d;return v.f()!=7;}', h+'int D::f(){return 7;}']
h = 'struct Pad{int p;};struct R{int r;};struct Result:Pad,R{};struct V{virtual R*get(){return 0;}};struct D:virtual V{Result value;Result*get();};'
cases['virtual-covariant-key'] = [h+'int main(){D d;V&v=d;return v.get()!=static_cast<R*>(&d.value)||d.get()!=&d.value;}', h+'Result*D::get(){return &value;}']
h = 'struct A{virtual int a(){return 1;}};struct B{virtual B*self(){return this;}};struct D:A,B{D*self(){return this;}};'
cases['shared-result-thunk'] = [h+'int one(){D d;B&b=d;return b.self()!=static_cast<B*>(&d);}', h+'struct E:D{};int one();int main(){E e;B&b=e;return one()||b.self()!=static_cast<B*>(&e);}']
h = 'struct A{virtual int a(){return 1;}};struct B{virtual B*self(){return this;}};struct D:A,B{D*self(){return 0;}};'
cases['shared-null-thunk'] = [h+'int one(){D d;B&b=d;return b.self()!=0;}', h+'struct E:D{};int one();int main(){E e;B&b=e;return one()||b.self()!=0;}']
h = 'struct A{virtual int a(){return 1;}};struct B{virtual int b(){return 2;}};struct D:A,B{int b(){return VALUE;}};'
cases['internal-targets'] = ['namespace{'+h.replace('VALUE','3')+'}int one(){D d;B&b=d;return b.b();}', 'namespace{'+h.replace('VALUE','7')+'}int one();int main(){D d;B&b=d;return one()!=3||b.b()!=7;}']
h = 'template<int I>struct A{virtual int a(){return I;}};template<int I>struct B{virtual int b(){return I+1;}};template<int I>struct D:A<I>,B<I>{int b(){return I+2;}int dormant(){return I.missing;}};'
cases['template-thunk'] = [h+'int one(){D<5>d;B<5>&b=d;return b.b();}', h+'struct E:D<5>{};int one();int main(){E e;B<5>&b=e;return one()!=7||b.b()!=7;}']
for name, sources in list(cases.items()):
    cases[name+'-reversed'] = list(reversed(sources))
# A shared anchor followed by a nonzero ordinary tail, through nested sharing.
cases['nested-virtual-tail'] = ['struct Pad{int p;};struct R{int r;};struct V:Pad,R{};struct A:virtual V{};struct B:A{};struct C:virtual B{};struct D:C{long n;};int read(B&b){return b.r;}int main(){D d;d.p=11;d.r=7;return read(d)!=7||static_cast<R*>(&d)!=static_cast<R*>(static_cast<B*>(&d));}']
for count in (64,256,1024):
    source = 'struct C0{int x;};'+''.join('struct C%d:C%d{};'%(i,i-1) for i in range(1,count+1))
    source += ''.join('C0* get%d(C%d*p){return p;}'%(i,i) for i in range(count,0,-1))
    cases['base-paths-'+str(count)] = [source+'int main(){C%d c;c.x=7;return get%d(&c)->x!=7;}'%(count,count)]

for depth in (4,8,12):
    source = 'struct V{virtual int f(){return 7;}};struct A0:virtual V{};struct B0:virtual V{};'
    for i in range(1,depth+1):
        source += 'struct A%d:virtual A%d,virtual B%d{};struct B%d:virtual A%d,virtual B%d{};'%(i,i-1,i-1,i,i-1,i-1)
    cases['shared-depth-'+str(depth)] = [source+'int call(A%d&a){return a.f();}int main(){return 0;}'%depth]

def check(cc, work):
    work.mkdir(parents=True, exist_ok=True)
    rows = []
    for name, sources in cases.items():
        paths = []
        for i, source in enumerate(sources):
            path = work/(name+str(i)+'.cpp'); path.write_text(source); paths.append(path)
        row = dict(name=name, sources=sources, lanes=[])
        lanes = [[p] for p in paths] + [paths] if len(paths)>1 else [paths]
        objects = []
        for i, lane in enumerate(lanes):
            if isinstance(lane, Path): lane = [lane]
            ir = work/(name+'-lane'+str(i)+'.lowir'); obj = ir.with_suffix('.o')
            command = [str(cc),'--emit-lowir','-O0','--validate-lowir','-o',str(ir),*map(str,lane)]
            p = subprocess.run(command,capture_output=True,text=True,timeout=60)
            record = dict(compile_exit=p.returncode,diagnostic=p.stderr); row['lanes'].append(record)
            if p.returncode: continue
            raw = ir.read_bytes(); q = subprocess.run(command+['--stats'],capture_output=True,text=True,timeout=60)
            record.update(lowir_sha256=sha(ir),stats_identical=q.returncode==0 and raw==ir.read_bytes())
            telemetry = [json.loads(line) for line in q.stderr.splitlines() if line.startswith('{')]
            record['work'] = [{k:v for k,v in t.items() if k in ('semantic_base_layout_work','semantic_base_adjustment_work','semantic_subobject_paths','semantic_virtual_slots','semantic_virtual_views','semantic_virtual_slot_work','semantic_final_overrider_work','semantic_virtual_demands','instructions','peak_rss_kib')} for t in telemetry]
            names = re.findall(r'^function [^\n]*\bobject=([^], ]+)',ir.read_text(),re.M)
            record['duplicate_function_objects'] = sorted(n for n,c in collections.Counter(names).items() if c>1)
            q = subprocess.run([str(ROOT/'dev/cppgm++-ref'),'-c','-O0','-o',str(obj),str(ir)],capture_output=True,text=True,timeout=60)
            record.update(backend_exit=q.returncode,backend_diagnostic=q.stderr)
            if q.returncode: continue
            if len(lane)==1 and len(paths)>1: objects.append(obj)
            else:
                exe = obj.with_suffix('.exe')
                q = subprocess.run(['g++','-no-pie',str(obj),'-o',str(exe)],capture_output=True,text=True)
                record.update(link_exit=q.returncode,link_diagnostic=q.stderr)
                if not q.returncode: record['runtime_exit'] = subprocess.run([str(exe)],timeout=20).returncode
                native = exe.with_suffix('.standalone')
                q = subprocess.run([str(ROOT/'dev/lowir2native-ref'),'-O0','-o',str(native),str(ir)],capture_output=True,text=True)
                record.update(standalone_backend_exit=q.returncode,standalone_diagnostic=q.stderr)
                if not q.returncode: record['standalone_exit'] = subprocess.run([str(native)],timeout=20).returncode
        if len(objects)==len(paths):
            exe = work/(name+'-separate.exe')
            q = subprocess.run(['g++','-no-pie',*map(str,objects),'-o',str(exe)],capture_output=True,text=True)
            row.update(separate_link_exit=q.returncode,separate_diagnostic=q.stderr)
            if not q.returncode: row['separate_exit'] = subprocess.run([str(exe)],timeout=20).returncode
        row['passed'] = all(r.get('stats_identical') and r.get('backend_exit')==0 and not r.get('duplicate_function_objects') for r in row['lanes']) and row['lanes'][-1].get('runtime_exit')==0 and row['lanes'][-1].get('standalone_exit')==0 and (len(paths)==1 or row.get('separate_exit')==0)
        rows.append(row); print(name,'PASS' if row['passed'] else 'FAIL',file=sys.stderr,flush=True)
    return dict(compiler=str(cc),compiler_sha256=sha(cc),cases=rows)

if __name__ == '__main__':
    cc, work = [Path(p).resolve() for p in sys.argv[1:3]]
    result = check(cc,work); print(json.dumps(result,indent=2))
    sys.exit(not all(r['passed'] for r in result['cases']))
