#!/usr/bin/env python3
"""Observe reduced oracle defects independently; this script rewrites no fixtures."""
from pathlib import Path
import hashlib,json,subprocess,sys
ROOT=Path(__file__).resolve().parents[2]
cc,work=map(lambda x:Path(x).resolve(),sys.argv[1:3]);work.mkdir(parents=True,exist_ok=True)
helper=work/'placement-library.cpp';helper.write_text('#include <new>\nvoid*(*keep_placement)(std::size_t,void*)=&::operator new;\n')
helper_obj=helper.with_suffix('.o')
subprocess.run(['g++','-std=c++11','-c',str(helper),'-o',str(helper_obj)],check=True)
rows=[]
for name in ['nonpoly-virtual-reference','construction-view-rtti','shared-copy-base','null-placement-virtual','null-placement-standard']:
 src=ROOT/'student.tests/pa23/reducers'/(name+'.cpp');row=dict(name=name,source=src.read_text(),source_sha256=hashlib.sha256(src.read_bytes()).hexdigest(),required_exit=0,lanes=[])
 for label,compiler in [('reference',ROOT/'dev/cppgm++-ref'),('student',cc)]:
  ir=work/(name+'-'+label+'.lowir');obj=ir.with_suffix('.o');exe=ir.with_suffix('.exe')
  commands=[[str(compiler),'--emit-lowir','-O0','-o',str(ir),str(src)],[str(ROOT/'dev/cppgm++-ref'),'-c','-O0','-o',str(obj),str(ir)],['g++','-no-pie',str(obj),*([str(helper_obj)] if name=='null-placement-standard' else []),'-o',str(exe)],[str(exe)]]
  lane=dict(compiler=label,commands=[]);row['lanes'].append(lane)
  for i,command in enumerate(commands):
   p=subprocess.run(command,capture_output=True,text=True,timeout=30);lane['commands'].append(dict(command=command,exit=p.returncode,stdout=p.stdout,stderr=p.stderr))
   if i==0 and p.returncode==0:lane.update(lowir=ir.read_text(),lowir_sha256=hashlib.sha256(ir.read_bytes()).hexdigest())
   if i==3:lane['runtime_exit']=p.returncode
   if p.returncode:break
 rows.append(row);print(name,[(x['compiler'],x.get('runtime_exit')) for x in row['lanes']],file=sys.stderr,flush=True)
print(json.dumps(dict(bundle_revision='c2f713cd70d06170632bfde3e75dd6fe1aa44d98',cases=rows),indent=2));sys.exit(not all(x['lanes'][1].get('runtime_exit')==0 for x in rows))
