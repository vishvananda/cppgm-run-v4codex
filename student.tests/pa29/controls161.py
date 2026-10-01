#!/usr/bin/env python3
"""Explicit atomic semantics, runtime, concurrency and rejection controls."""
import hashlib,json,pathlib,subprocess,sys
root=pathlib.Path(__file__).resolve().parents[2]
out=pathlib.Path(sys.argv[1] if len(sys.argv)>1 else '/tmp/pa29-161/controls').resolve();out.mkdir(parents=True,exist_ok=True)
cc=root/'dev/cppgm++';rows=[]
def run(name,args,ok=True):
 p=subprocess.run(list(map(str,args)),capture_output=True,timeout=90,cwd=root)
 good=(p.returncode==0)==ok
 rows.append(dict(name=name,args=list(map(str,args)),status=p.returncode,passed=good,stdout=p.stdout.decode(errors='replace'),stderr=p.stderr.decode(errors='replace')))
 if not good:print(name,p.returncode,p.stderr.decode(errors='replace'),flush=True)
 return good
for src in sorted((root/'student.tests/pa29/controls161').glob('*.cpp')):
 if src.name=='atomic-host.cpp':continue
 obj=out/(src.stem+'.o');exe=out/src.stem
 if run(src.stem+' compile',[cc,'-c',src,'-o',obj]):
  host=[src.parent/'atomic-host.cpp','-pthread'] if src.name=='atomic-hosted.cpp' else []
  if run(src.stem+' link',['g++',obj,*host,'-latomic','-o',exe]):run(src.stem+' run',[exe])
reject={
 'arity':'int x;int main(){__atomic_load_n(&x);}',
 'extra-arity':'int x;int main(){__atomic_store_n(&x,1,0,3);}',
 'store-const':'const int x=1;int main(){__atomic_store_n(&x,2,0);}',
 'c11-plain':'int x;int main(){__c11_atomic_load(&x,0);}',
 'gnu-atomic-object':'_Atomic(int) x;int main(){__atomic_load_n(&x,0);}',
 'expected-type':'int x;long e;int main(){__atomic_compare_exchange_n(&x,&e,1,false,5,2);}',
 'generic-output-const':'int x;const int y=0;int main(){__atomic_load(&x,&y,0);}',
 'boolean-fetch':'bool b;int main(){__atomic_fetch_add(&b,1,0);}',
 'double-fetch':'_Atomic(double) d;int main(){__c11_atomic_fetch_add(&d,1.,0);}',
 'function-address':'int x;auto p=&__atomic_load_n;',
 'atomic-const':'_Atomic(const int) x;',
 'atomic-reference':'_Atomic(int&) x;',
 'atomic-array':'_Atomic(int[2]) x;',
 'atomic-void':'_Atomic(void) x;',
 'atomic-nested':'_Atomic(_Atomic(int)) x;',
 'nontrivial':'struct X {X(const X&);int x;};_Atomic(X) x;',
 'dependent-nontrivial':'struct X {X(const X&);int x;};template<class T>struct Box{_Atomic(T) x;};Box<X> b;',
 'dependent-const':'template<class T>struct Box{_Atomic(T) x;};Box<const int> b;',
 'atomic-constant-read':'constexpr _Atomic(int) x=1;static_assert(x==1,"");',
 'atomic-to-plain-pointer':'_Atomic(int) x;int* p=&x;',
 'plain-to-atomic-pointer':'int x;_Atomic(int)* p=&x;',
}
for name,source in reject.items():
 src=out/(name+'-reject.cpp');src.write_text(source+'\n');run(name,[cc,'-c',src,'-o',out/'reject.o'],False)
result=dict(compiler_sha256=hashlib.sha256(cc.read_bytes()).hexdigest(),checks=rows)
(out/'results.json').write_text(json.dumps(result,indent=2)+'\n')
print('%d/%d controls passed'%(sum(r['passed'] for r in rows),len(rows)),flush=True)
sys.exit(any(not r['passed'] for r in rows))
