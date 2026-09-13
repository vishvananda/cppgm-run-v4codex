#!/usr/bin/env python3
"""Copy/direct/list mode controls, N3485 [over.match.ctor]/1,
[over.match.copy]/1, [over.match.list]/1, [dcl.init.aggr]/2,7,
[class.copy]/31-32 and [temp.res]/8. No reference changes.
"""
from pathlib import Path
import hashlib,json,os,subprocess,sys
ROOT=Path(__file__).resolve().parents[2]
BINARY=Path(sys.argv[1]).resolve() if len(sys.argv)>1 else ROOT/'dev/cppgm++'
WORK=Path(sys.argv[2]) if len(sys.argv)>2 else Path(os.environ['RALPH_ARTIFACT_DIR'])/'pa14-final-audit/mode-controls'
WORK.mkdir(parents=True,exist_ok=True)
rows=[]
def sha(p):return hashlib.sha256(Path(p).read_bytes()).hexdigest()
def run(name,source,reject):
 path=WORK/(name+'.cpp');path.write_text(source)
 ir=WORK/(name+'.lowir');log=WORK/(name+'.log')
 p=subprocess.run([str(BINARY),'--emit-lowir','-O0','-o',str(ir),str(path),'--validate-lowir'],capture_output=True,text=True)
 log.write_text(p.stdout+p.stderr)
 assert p.returncode==int(reject),(name,p.returncode,p.stderr)
 assert 'Sanitizer' not in p.stderr and 'runtime error:' not in p.stderr,(name,p.stderr)
 row=dict(name=name,source_path=str(path),source_sha256=sha(path),reject=reject,exit_code=p.returncode,log=str(log),log_sha256=sha(log))
 if not reject:
  exe=WORK/name
  p=subprocess.run([str(ROOT/'dev/lowir2native-ref'),'-O0','-o',str(exe),str(ir)],capture_output=True,text=True)
  assert p.returncode==0,(name,p.stderr)
  p=subprocess.run([str(exe)],timeout=10)
  assert p.returncode==0,(name,p.returncode)
  row['native']=dict(path=str(exe),sha256=sha(exe),exit_code=p.returncode)
 rows.append(row);print(name,'PASS',flush=True)
prefix='int tag;struct E{explicit E(int){tag=1;}E(double){tag=2;}};struct X{explicit X(int){}};struct Z{explicit Z(){}};struct C{C(){}explicit C(C&){tag=3;}C(const C&){tag=4;}};struct M{M(){}explicit M(M&&){tag=5;}M(const M&){tag=6;}};struct R{R(){}explicit R(const R&){}};'
invalid={
 'copy-list':'E e={1};',
 'aggregate-copy-list':'struct A{X x;};A a{{1}};',
 'aggregate-copy':'struct A{X x;};A a{1};',
 'array-copy-list':'X a[1]={{1}};',
 'array-copy':'X a[1]={1};',
 'omitted-copy-list':'struct A{Z z;};A a{};',
 'omitted-array':'Z a[1]={};',
 'copy-explicit-only':'R a;R b=a;',
 'copy-prvalue-explicit-only':'R a=R();',
}
valid={
 'copy-overload':'E e=1;return tag!=2;',
 'direct-overload':'E e(1);return tag!=1;',
 'direct-list':'E e{1};return tag!=1;',
 'copy-double-list':'E e={1.0};return tag!=2;',
 'aggregate-overload':'struct A{E e;};A a{1};return tag!=2;',
 'copy-transfer':'C a;C b=a;return tag!=4;',
 'direct-transfer':'C a;C b(a);return tag!=3;',
 'copy-move':'M a;M b=static_cast<M&&>(a);return tag!=6;',
 'direct-move':'M a;M b(static_cast<M&&>(a));return tag!=5;',
 'direct-explicit-only':'R a;R b(a);return 0;',
 'dependent-group':'struct A{T x;int n;};A a{{1},{2}};return a.x+a.n!=3;',
}
for name,body in invalid.items():
 for style in ('ordinary','unused','used','member','qualified'):
  source={'ordinary':'void f(){'+body+'}int main(){}','unused':'template<class T>void f(){'+body+'}int main(){}',
   'used':'template<class T>void f(){'+body+'}int main(){f<int>();}',
   'member':'template<class T>struct V{void f(){'+body+'}};int main(){}',
   'qualified':'template<class T>struct V{void f();};template<class T>void V<T>::f(){'+body+'}int main(){}'}[style]
  run(style+'-'+name,prefix+source,True)
for name,body in valid.items():
 run('template-'+name,prefix+'template<class T>int f(){'+body+'}int main(){return f<int>()||f<long>();}',False)
 run('ordinary-'+name,prefix+'typedef int T;int f(){'+body+'}int main(){return f();}',False)
for name,body in {'partial-aggregate':'struct A{T x;int* p;};A a{{1},{42}};',
 'partial-array':'struct A{T x[2];int* p;};A a{{{1},{2}},{42}};',
 'partial-omitted':'struct A{T x;int& r;};A a{{1}};',
 'partial-narrow':'struct A{T x;char c;};A a{{1},{300}};'}.items():
 run(name,'template<class T>void f(){'+body+'}int main(){}',True)
run('return-copy',prefix+'template<class T>C f(){C a,b;if(tag)return a;return b;}int main(){f<int>();return tag!=4;}',False)
run('return-move',prefix+'template<class T>M f(){M a,b;if(tag)return a;return b;}int main(){f<int>();return tag!=6;}',False)
run('argument-copy',prefix+'void g(C a){}template<class T>int f(){C a;g(a);return tag!=4;}int main(){return f<int>()||f<long>();}',False)
run('argument-move',prefix+'void g(M a){}template<class T>int f(){M a;g(static_cast<M&&>(a));return tag!=6;}int main(){return f<int>()||f<long>();}',False)
print(len(rows),'initialization mode checks PASS')
(WORK/'checks.json').write_text(json.dumps(dict(binary=dict(path=str(BINARY),sha256=sha(BINARY)),harness_sha256=sha(__file__),checks=rows),indent=2)+'\n')
