#!/usr/bin/env python3
"""Observe the pinned reference on reducers; preserve its LowIR and native result."""
from pathlib import Path
import hashlib,json,subprocess,sys
ROOT=Path(__file__).resolve().parents[2]
CASES={
 'reference': 'int value;extern int& ref;int check(){return &ref==&value;}int observed=check();int& ref=value;int main(){return observed!=1;}',
 'constexpr_function_address': 'template<class T>void f(){}struct Table{void(*p)();constexpr Table(void(*q)()):p(q){}};template<class T>struct X{static constexpr Table t=Table(&f<T>);};template<class T>constexpr Table X<T>::t;int main(){return X<int>::t.p!=&f<int>;}',
 'local_function_address': 'template<class T>struct X{static int f(){return 7;}};struct Table{int(*p)();};template<class T>int g(){static const Table t={&X<T>::f};return t.p();}int main(){return g<int>()!=7;}',
 'unused_member': 'int calls;int init(){++calls;return 1;}template<class T>struct X{static const int n;friend X f(X){return X();}};template<class T>const int X<T>::n=init();int main(){X<int>x;f(x);return calls;}',
 'empty_value': 'struct X{};X f(){return X();}int main(){X x=f();return 0;}',
}
def sha(p):return hashlib.sha256(Path(p).read_bytes()).hexdigest()
if __name__=='__main__':
 work=Path(sys.argv[1]).resolve();work.mkdir(parents=True,exist_ok=True);out=Path(sys.argv[2]);rows=[]
 for name,source in CASES.items():
  src=work/(name+'.cpp');src.write_text(source);ir=src.with_suffix('.lowir');exe=src.with_suffix('.exe')
  r=subprocess.run([ROOT/'dev/cppgm++-ref','--emit-lowir','-O0','-o',ir,src],text=True,capture_output=True)
  row=dict(name=name,source=source,source_sha256=sha(src),compiler_exit=r.returncode,diagnostic=r.stderr)
  if r.returncode==0:
   row.update(lowir=ir.read_text(),lowir_sha256=sha(ir))
   b=subprocess.run([ROOT/'dev/lowir2native-ref','-O0','-o',exe,ir],text=True,capture_output=True)
   row.update(backend_exit=b.returncode,backend_diagnostic=b.stderr)
   if b.returncode==0:row.update(native_exit=subprocess.run([exe]).returncode,native_sha256=sha(exe))
  rows.append(row);print(name,row['compiler_exit'],row.get('native_exit'),flush=True)
 out.write_text(json.dumps(dict(bundle='c2f713cd70d06170632bfde3e75dd6fe1aa44d98',compiler_sha256=sha(ROOT/'reference-binaries/cppgm++'),reducers=rows),indent=2)+'\n')
