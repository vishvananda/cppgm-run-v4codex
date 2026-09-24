#!/usr/bin/env python3
"""Reproduce six narrow oracle corrections from the frozen entry, without a compiler.

The proof and pinned reference bundle are in pa17/storage-references.md.
Default verifies; --apply writes the corrected references.
"""
from pathlib import Path
import re, subprocess, sys
ROOT=Path(__file__).resolve().parents[2]
ENTRY='7cc89281'
def original(name):
 return subprocess.check_output(['git','show',ENTRY+':pa17/tests/'+name+'.ref'],cwd=ROOT,text=True)
def replace(text,old,new):
 assert text.count(old)==1,(old,text.count(old))
 return text.replace(old,new)
def without_init(text):
 text,n=re.subn(r'function @__cppgm_init\(\).*?\n}\n','',text,flags=re.S)
 assert n==1
 return text
def corrected():
 outputs={}
 name='general/300-constexpr-static-fn-template-address-pack'
 s=original(name);s=replace(s,'  zero 8','  ptr addr @slot_void_____impl')
 outputs[name]=without_init(s)
 name='spec/300-rooted-qualified-static-data-member-template-definition'
 s=original(name);target='@n____cppgm_class_template_identity_0_0_0__instance'
 s=replace(s,'] = zero','] = addr '+target)
 s=replace(s,'    %t1 = addr '+target+'\n    store ptr %t1, @_GLOBAL__N_1__color_map\n','')
 outputs[name]=s
 name='general/300-namespace-function-template-hides-outer-callable-object'
 s=original(name);s=replace(s,'] = zero','] = addr @outer____cppgm_class_template_identity_0_0_0__value')
 outputs[name]=without_init(s)
 name='general/400-alias-nontype-pack-partial-base'
 s=original(name)
 for name2 in ('first','second'):
  s=replace(s,f'global @{name2} : ptr [binding=strong, object=_Z{len(name2)}{name2}] = zero',
              f'global @{name2} : ptr [binding=strong, object=_Z{len(name2)}{name2}] = addr @value')
 s,n=re.subn(r'    %t1 = addr @value\n.*?    store ptr %t4, @second\n','',s,flags=re.S);assert n==1
 outputs[name]=s
 name='general/300-dependent-hidden-friend-static-member-definition'
 s=original(name);s=replace(s,s.splitlines()[0],'declare '+s.splitlines()[0].removesuffix(' = 0'))
 s=replace(s,'    %t1 = addr $retobj__1\n','    %t1 = addr $retobj__1\n    zeroinit 1x1 %t1\n')
 outputs[name]=s
 name='general/300-function-template-local-static-per-specialization'
 s=original(name)
 for tag in ('log_functor','init_functor'):
  s=s.replace('  zero 16',f'  ptr addr @manager_{tag}___call\n  ptr addr @invoker_{tag}___call',1)
 s,n=re.subn(r'^global .*__guard : i64 .*\n','',s,flags=re.M);assert n==2
 s,n=re.subn(r'    %t1 = load i64 .*?  block \^local_static_ready_1:\n','',s,flags=re.S);assert n==2
 for value,slot in (('%t3','$argobj__1'),('%t6','$argobj__2')):
  s=replace(s,f'    {value} = addr {slot}\n',f'    {value} = addr {slot}\n    zeroinit 1x1 {value}\n')
 outputs[name]=s
 return outputs
if __name__=='__main__':
 for name,text in corrected().items():
  p=ROOT/'pa17/tests'/(name+'.ref')
  if '--apply' in sys.argv:p.write_text(text)
  else:assert p.read_text()==text,name
 print('Six storage references follow the documented surgical corrections.')
