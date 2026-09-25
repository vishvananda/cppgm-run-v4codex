#!/usr/bin/env python3
"""Canonical class-result repair from entry oracles, never student output."""
from pathlib import Path
import hashlib,json,re,subprocess,sys
ROOT=Path(__file__).resolve().parents[2]
ENTRY='01b47c20b41f68d3142df6a8691fc905c20341a8'
CASES={
 'pa18/tests/general/300-friend-function-template-alias-result-definition.ref':{'make_owner':(4,4),'make_owner__ov2':(4,4)},
 'pa18/tests/spec/300-conversion-function-template-object-result-copy-init.ref':{'X__operatorT':(4,4)},
 'pa18/tests/spec/300-dependent-result-defaulted-nontype-declaring-scope.ref':{'units__operator_':(8,8)},
}
def repair(old,functions):
 text=old
 for name,(size,align) in functions.items():
  obj=f'obj<{size}x{align}>';span=f'{size}x{align}'
  pattern=r'(function @'+re.escape(name)+r'\()%ret : ptr \[pass=indirect_result, object_bytes='+str(size)+r'\], ([^\n]+?)\) -> void([^\n]*)\n(.*?)\n}'
  m=re.search(pattern,text,re.S);assert m,name
  body=m[4];pos=body.index('\n\n  block ')
  body=body[:pos]+f'\n  slot $result87 : {obj}'+body[pos:]
  # The destination becomes local result backing storage after parameter spills.
  point=re.search(r'(  block \^\w+:\n(?:    store [^\n]+\n)*)',body);assert point
  body=body[:point.end()]+'    %result87 = addr $result87\n'+body[point.end():]
  body=re.sub(r'%ret\b','%result87',body)
  assert body.count('return void')==1
  body=body.replace('return void',f'return {obj} $result87')
  text=text[:m.start()]+m[1]+m[2]+') -> '+obj+m[3]+'\n'+body+'\n}'+text[m.end():]
  count=[0]
  def call(m):
   count[0]+=1;v=f'%result87_{name}_{count[0]}'
   return f'    {v} = call {obj} @{name}({m[2]})\n    copyobj {span} {v}, {m[1]}'
  text,n=re.subn(r'^    call void @'+re.escape(name)+r'\((%\w+), ([^\n]*)\)$',call,text,flags=re.M)
  assert n,name
 return text
def main():
 rows=[]
 for path,functions in CASES.items():
  old=subprocess.check_output(['git','show',ENTRY+':'+path],cwd=ROOT,text=True)
  new=repair(old,functions)
  if '--write' in sys.argv:(ROOT/path).write_text(new)
  assert (ROOT/path).read_text()==new,path
  rows.append(dict(path=path,before_sha256=hashlib.sha256(old.encode()).hexdigest(),after_sha256=hashlib.sha256(new.encode()).hexdigest(),functions=functions))
 result=dict(entry=ENTRY,bundle='c2f713cd70d06170632bfde3e75dd6fe1aa44d98',bundle_sha256='c532a109ae800825da24f60ae28ea894aa4896f56efb6ebb14728cdccf25a7d7',revisions=rows)
 p=ROOT/'student.tests/pa18/reference87-revisions.json'
 if '--write' in sys.argv:p.write_text(json.dumps(result,indent=2)+'\n')
 assert json.loads(p.read_text())==json.loads(json.dumps(result))
 print('Verified three class-result oracle repairs from entry data; all other instructions and metadata retained.')
if __name__=='__main__':main()
