#!/usr/bin/env python3
"""Source/adapter budget equivalence and typed-to-native audit traces."""
import pathlib,subprocess,json,hashlib,sys
root=pathlib.Path(__file__).resolve().parents[2];out=pathlib.Path(sys.argv[1]).resolve();out.mkdir(parents=True,exist_ok=True)
cc=root/'dev/cppgm++';adapter=pathlib.Path(sys.argv[2]).resolve();rows=[];cases=[]
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def save():(out/'inspection.json').write_text(json.dumps(dict(compiler_sha256=sha(cc),rows=rows,cases=cases),indent=2)+'\n')
def run(args):
    args=list(map(str,args));p=subprocess.run(args,capture_output=True,text=True,timeout=120)
    rows.append(dict(command=args,status=p.returncode,stdout=p.stdout,stderr=p.stderr));save();assert p.returncode==0,rows[-1]
    return p.stdout
sources=[p for p in (root/'student.tests/pa29/source182').glob('*.cpp') if '.reject.' not in p.name]
for name,depth,double in [('inline-depth',67,False),('inline-growth',18,True)]:
    p=out/(name+'.cpp')
    text='volatile int effects;\n'
    for i in range(depth,-1,-1):
        body='++effects;' if i==depth else ('f%d();'%(i+1))*(2 if double else 1)
        text+='__attribute__((always_inline)) inline void f%d(){%s}\n'%(i,body)
    text+='int main(){f0();return effects!=%d;}\n'%((1<<depth) if double else 1)
    p.write_text(text);sources.append(p)
for src in sources:
    name=src.stem;ir=out/(name+'.lowir');obj=out/(name+'.o');prepared=out/(name+'.prepared.lowir');ao=out/(name+'.adapter.o')
    run([cc,'-c','--emit-lowir','--validate-lowir',src,'-o',ir])
    run([root/'dev/lowir',ir,'-o',out/'roundtrip']);assert ir.read_bytes()==(out/'roundtrip').read_bytes()
    counts=list(map(int,run([adapter,ir,prepared,ao]).split()))
    run([root/'dev/lowir',prepared,'-o',out/'prepared-roundtrip']);assert prepared.read_bytes()==(out/'prepared-roundtrip').read_bytes()
    run([cc,'-O0','--stats','-c',src,'-o',obj]);metrics={}
    for line in rows[-1]['stderr'].splitlines():
        if line.startswith('{'):metrics.update(json.loads(line))
    assert counts==[metrics[k] for k in ['inline_calls','inline_work','inline_declined','inline_budget_work','inline_max_function_work']]
    assert counts[1]<=counts[3]<=4194304 and counts[4]<=262144
    disassembly=[]
    for image in [obj,ao]:
        text=subprocess.check_output(['objdump','-dr',image],text=True).splitlines()[3:]
        disassembly.append(hashlib.sha256('\n'.join(text).encode()).hexdigest())
    assert disassembly[0]==disassembly[1],name
    assert sorted(run(['nm',obj]).splitlines())==sorted(run(['nm',ao]).splitlines())
    for image,label in [(obj,'direct'),(ao,'adapter')]:
        exe=out/(name+label);run(['g++',image,'-o',exe]);run([exe])
    run([cc,'-O0','-c',src,'-o',out/'plain.o']);assert obj.read_bytes()==(out/'plain.o').read_bytes()
    if name in ['integrated','array-volatile-loop','inline-cycle']:
        run([cc,'--emit-ast',src,'-o',out/(name+'.ast')])
        run([root/'dev/lowir2native','--dump-machine-ir',out/(name+'.mir'),ir])
        run(['readelf','-rSW',obj]);run(['readelf','--debug-dump=frames',obj])
    if name=='array-volatile-loop':
        assert 'load volatile' in ir.read_text() and 'store volatile' in ir.read_text()
    if name in ['inline-cycle','inline-depth','inline-growth']:assert counts[2]>0
    cases.append(dict(name=name,source_sha256=sha(src),source_text=src.read_text(),lowir_sha256=sha(ir),prepared_sha256=sha(prepared),direct_sha256=sha(obj),adapter_sha256=sha(ao),disassembly_sha256=disassembly,inline_counts=counts,metrics=metrics));save()
    print(name,'pass',counts,flush=True)
print(len(rows),'commands passed',flush=True)
