#!/usr/bin/env python3
"""Dependent allocation ABI: independently compiled callers and checked runtime."""
from pathlib import Path
import hashlib,json,subprocess,sys
ROOT=Path(__file__).resolve().parents[2]
CC,WORK=[Path(x).resolve() for x in sys.argv[1:]]
WORK.mkdir(parents=True,exist_ok=True)
declarations=[];definitions=[];anchors=[];checks=[]
for global_ in ('','::'):
 for array in (False,True):
  for init in ('','()','{7}'):
   if array and init=='{7}':continue # nonempty array new-initializers are outside the inherited slice
   name='n%d'%len(declarations);expr=global_+'new T'+('[2]' if array else '')+init
   head='template<class T>auto '+name+'()->decltype('+expr+')'
   declarations.append(head+';');definitions.append(head+'{return '+expr+';}')
   anchors.append('delete'+('[]' if array else '')+' '+name+'<int>();')
   checks.append('{int*p='+name+'<int>();'+('if(*p!=%d)return 1;'%(7 if init=='{7}' else 0) if init else '')+'delete'+('[]' if array else '')+' p;}')
 for array in (False,True):
  name='d%d'%len(declarations);expr=global_+'delete'+('[]' if array else '')+' p'
  head='template<class T>auto '+name+'(T*p)->decltype('+expr+')'
  declarations.append(head+';');definitions.append(head+'{'+expr+';}')
  anchors.append(name+'((int*)0);');checks.append(name+'(new int'+('[2]' if array else '')+');')
source='\n'.join(definitions)+'\nvoid anchor(){'+''.join(anchors)+'}\n'
caller='\n'.join(declarations)+'\nint main(){'+''.join(checks)+'return 0;}\n'
(WORK/'source.cpp').write_text(source);(WORK/'caller.cpp').write_text(caller)
commands=[]
for cmd in ([CC,'--emit-lowir','-O0','--validate-lowir','-o',WORK/'source.lowir',WORK/'source.cpp'],
 [ROOT/'dev/cppgm++-ref','-c','-O0','-o',WORK/'source.o',WORK/'source.lowir'],
 ['g++','-std=c++11','-no-pie',WORK/'caller.cpp',WORK/'source.o','-o',WORK/'check'],[WORK/'check']):
 p=subprocess.run(list(map(str,cmd)),capture_output=True,text=True,timeout=30)
 commands.append(dict(argv=list(map(str,cmd)),exit=p.returncode,stdout=p.stdout,stderr=p.stderr))
 (WORK/'results.json').write_text(json.dumps(dict(source=source,caller=caller,compiler_sha256=hashlib.sha256(CC.read_bytes()).hexdigest(),commands=commands),indent=2)+'\n')
 if p.returncode:print(p.stderr);sys.exit(1)
print('Allocation ABI cross-object check PASS (%d signatures)'%len(declarations))
