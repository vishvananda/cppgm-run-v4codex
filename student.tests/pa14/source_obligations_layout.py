#!/usr/bin/env python3
"""Freeze source-obligation declaration and conversion record layouts."""
from pathlib import Path
import hashlib,json,subprocess,sys
ROOT=Path(__file__).resolve().parents[2]
WORK=Path(sys.argv[1]).resolve();WORK.mkdir(parents=True,exist_ok=True)
ENTRY='7f401fa8'
SOURCE=ROOT/'student.tests/pa14/virtual_demand_layout_probe.cc'
def sha(p):return hashlib.sha256(Path(p).read_bytes()).hexdigest()
def run(cmd):
 p=subprocess.run(list(map(str,cmd)),cwd=ROOT,capture_output=True,text=True)
 assert p.returncode==0,(cmd,p.stderr)
 return p.stdout
old=WORK/'entry-source'
for name in run(['git','ls-tree','-r','--name-only',ENTRY,'dev/src']).splitlines():
 if not name.endswith('.h'):continue
 dest=old/name;dest.parent.mkdir(parents=True,exist_ok=True)
 dest.write_text(run(['git','show',ENTRY+':'+name]))
result=dict(entry_commit=ENTRY,harness_sha256=sha(__file__),source_path=str(SOURCE),source_sha256=sha(SOURCE),layouts=[])
for label,origin in [('entry',old),('current',ROOT)]:
 folder=WORK/label;folder.mkdir(parents=True,exist_ok=True)
 include=origin/'dev/src'
 deps=run(['g++','-std=c++11','-I'+str(include),'-MM',SOURCE]).replace('\\\n','').split(':',1)[1].split()
 headers=[]
 for name in deps:
  src=Path(name).resolve()
  if src.suffix!='.h':continue
  path=folder/'headers'/src.relative_to(origin);path.parent.mkdir(parents=True,exist_ok=True)
  path.write_bytes(src.read_bytes());headers.append(dict(source=str(src),path=str(path),sha256=sha(src)))
 exe=folder/'probe';dump=folder/'layout.class'
 command=['g++','-std=c++11','-O2','-I'+str(folder/'headers/dev/src'),'-fdump-lang-class='+str(dump),str(SOURCE),'-o',str(exe)]
 run(command);sizes=list(map(int,run([exe]).split()))
 result['layouts'].append(dict(label=label,headers=headers,command=command,build_exit=0,sizes=sizes,binary_path=str(exe),binary_sha256=sha(exe),dump_path=str(dump),dump_sha256=sha(dump)))
 extra=ROOT/'student.tests/pa14/destination_layout_probe.cc'
 extra_binary=folder/'destination-probe'
 run(['g++','-std=c++11','-O2','-I'+str(folder/'headers/dev/src'),extra,'-o',extra_binary])
 result['layouts'][-1]['conversion_records']=dict(source_path=str(extra),source_sha256=sha(extra),binary_path=str(extra_binary),binary_sha256=sha(extra_binary),sizes=list(map(int,run([extra_binary]).split())))
 print(label,len(headers),sizes,flush=True)
 if label=='current':
  extra=ROOT/'student.tests/pa14/default_layout_probe.cc'
  exe=folder/'default-probe'
  run(['g++','-std=c++11','-O2','-I'+str(folder/'headers/dev/src'),extra,'-o',exe])
  result['defaults']=dict(source_path=str(extra),source_sha256=sha(extra),binary_path=str(exe),binary_sha256=sha(exe),sizes=list(map(int,run([exe]).split())))
 else:
  extra=ROOT/'student.tests/pa14/default_layout_probe.cc'
  exe=folder/'list-probe'
  run(['g++','-std=c++11','-O2','-DONLY_LIST','-I'+str(folder/'headers/dev/src'),extra,'-o',exe])
  result['entry_list']=dict(source_path=str(extra),source_sha256=sha(extra),binary_path=str(exe),binary_sha256=sha(exe),sizes=list(map(int,run([exe]).split())))
(ROOT/'student.tests/pa14/source-obligations-layout.json').write_text(json.dumps(result,indent=2)+'\n')
