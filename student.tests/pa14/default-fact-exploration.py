from pathlib import Path
import json,subprocess,sys
B,W=map(lambda x:Path(x).resolve(),sys.argv[1:3]);W.mkdir(parents=True,exist_ok=True)
CASES={
'function-conversion':'template<class T>void f(T v="wrong"){} int main(){f<int>();}',
'function-conversion-unused':'template<class T>void f(T v="wrong"){} int main(){f<int>(1);}',
'member-conversion':'template<class T>struct C{static void f(T v="wrong") {}};int main(){C<int>::f();}',
'function-explicit':'struct C{explicit C(int){}};template<class T>void f(T v=1){}int main(){f<C>();}',
'function-reference':'template<class T>void f(T& v=1){}int main(){f<int>();}',
'function-narrowing':'template<class T>void f(T v={1.5}){}int main(){f<int>();}',
'function-recursive':'template<class T>int f(int v=f<T>()){return v;}int main(){return f<int>();}',
'unused-ordinary':'template<class T>int fail(){return T::missing;}void f(int v=fail<int>());int main(){return 0;}',
'unused-template':'template<class T>int fail(){return T::missing;}template<class T>void f(int v=fail<T>());int main(){return 0;}',
'private-conversion':'struct C{private:operator int(){return 3;}};template<class T>void f(int v=T()){}int main(){f<C>();}',
'member-recursive':'template<class T>struct C{static int f(int v=f()){return v;}};int main(){return C<int>::f();}',
}
rows=[]
for name,source in CASES.items():
 p=W/(name+'.cpp');p.write_text(source);r=[]
 for cmd in ([B,'--emit-lowir','-O0','--validate-lowir','-o',W/(name+'.lowir'),p],['g++','-std=c++11','-pedantic-errors','-fsyntax-only',p]):
  try:
   proc=subprocess.run(list(map(str,cmd)),capture_output=True,text=True,timeout=30)
   r.append(dict(command=list(map(str,cmd)),exit_code=proc.returncode,stdout=proc.stdout,stderr=proc.stderr))
  except subprocess.TimeoutExpired:r.append(dict(command=list(map(str,cmd)),timeout=True))
 rows.append(dict(name=name,source=source,results=r));(W/'checks.json').write_text(json.dumps(rows,indent=2)+'\n')
 print(name,[(x.get('exit_code'),x.get('stderr','')[:100]) for x in r],flush=True)
