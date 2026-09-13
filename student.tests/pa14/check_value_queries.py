#!/usr/bin/env python3
"""C++11 dependent array bounds, short-circuit obligations and ABI spellings."""
from pathlib import Path
import subprocess,sys,tempfile
ROOT=Path(__file__).resolve().parents[2]
CASES=[
 'template<class T>int f(int (&)[sizeof(T)-sizeof(T)]);int main(){int a[1];return f<int>(a);}',
 'template<class T>int f(int (&)[-int(sizeof(T))]);int main(){int a[1];return f<int>(a);}',
 'template<class T>int f(int (&)[sizeof(T)]);int main(){int a[3];return f<int>(a);}',
 'template<class T>int f(int (&)[T::count]);struct C{};int main(){int a[3];return f<C>(a);}',
 'template<class T>int f(int (&)[T::count]);struct C{using count=int;};int main(){int a[3];return f<C>(a);}',
 'template<class T>int f(int (&)[T::count]);struct C{int count;};int main(){int a[3];return f<C>(a);}',
 'template<class T>int f(int (&)[T::count]);struct C{static int count;};int main(){int a[3];return f<C>(a);}',
 'template<class T>int f(int (&)[T::count]);class C{static const int count=3;};int main(){int a[3];return f<C>(a);}',
 'template<class T>int f(int (&)[sizeof(T)?1/0:3]);int main(){int a[3];return f<int>(a);}',
 'template<class T>int f(int (&)[sizeof(T)||*sizeof(T)]);',
 'template<class T>int f(int (&)[sizeof(T)?3:nullptr]);int main(){int a[3];return f<int>(a);}',
 'template<class T>int f(int (&)[sizeof(T)&&(1/0)]);int main(){int a[1];return f<int>(a);}',
 'template<class T>int f(int (&)[static_cast<int*>(sizeof(T))]);int main(){int a[1];return f<int>(a);}',
 'template<class T>int f(int (&)[sizeof(T)/0]);int main(){int a[1];return f<int>(a);}',
]
if __name__=='__main__':
 binary=Path(sys.argv[1]).resolve() if len(sys.argv)>1 else ROOT/'dev/cppgm++'
 with tempfile.TemporaryDirectory(prefix='pa14-value-queries-') as directory:
  work=Path(directory)
  for i,source in enumerate(CASES):
   src=work/f'{i}.cpp';src.write_text(source)
   r=subprocess.run([binary,'--emit-lowir','-O0','-o',work/'out',src],capture_output=True,text=True,timeout=60)
   assert r.returncode==1,(i,r.returncode,r.stderr)
   assert not any(m in r.stderr for m in ('AddressSanitizer','UndefinedBehaviorSanitizer','runtime error:')),(i,r.stderr)
  r=subprocess.run([ROOT/'dev/abimangle','-o',work/'names',ROOT/'student.tests/pa14/value-query-names.abi'],capture_output=True,text=True)
  assert r.returncode==0,r.stderr
  assert (work/'names').read_text()=='AstT__i\nAatT__i\nAszfp__i\nAazfp__i\n'
  ir=work/'reducer.lowir';exe=work/'reducer'
  for command in ([binary,'--emit-lowir','-O0','--validate-lowir','-o',ir,ROOT/'student.tests/pa14/value-conversion.t'],
                  [ROOT/'dev/lowir2native-ref','-O0','-o',exe,ir],[exe]):
   r=subprocess.run(list(map(str,command)),capture_output=True,text=True,timeout=60)
   assert r.returncode==0,(command,r.returncode,r.stderr)
   assert not any(m in r.stderr for m in ('AddressSanitizer','UndefinedBehaviorSanitizer','runtime error:')),r.stderr
  print(len(CASES),'bound rejections, four ABI controls and conversion reducer PASS')
