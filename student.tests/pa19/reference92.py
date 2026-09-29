#!/usr/bin/env python3
"""Independently reconstruct proved PA19 revisions from the entry oracles."""
from pathlib import Path
import hashlib, json, re, subprocess, sys
ROOT=Path(__file__).resolve().parents[2]
ENTRY='065d67835340af14fc3f154f3f314e552d0b3f69'
def sha(s): return hashlib.sha256(s.encode()).hexdigest()
def old(path): return subprocess.check_output(['git','show',ENTRY+':'+path],cwd=ROOT,text=True)
# Reuse the store-to-image reconstruction from PA18 reference83.py, preserving
# its full-span and unused-address checks; no student output is consulted.
def array_images(text,slots):
 lines=text.splitlines(True);images={};records=[];out=[];i=0
 while i<len(lines):
  line=lines[i];out.append(line);i+=1
  m=re.fullmatch(r'    (%\w+) = addr \$(\w+)\n',line)
  if not m or m[2] not in slots:continue
  base,slot=m.groups();shape=re.search(r'  slot \$'+re.escape(slot)+r' : obj<(\d+)x(\d+)>',text)
  if not shape:continue
  size,align=map(int,shape.groups());positions={base:0};removed=set();stores=[];j=i;direct=True
  while j<len(lines):
   idx=re.fullmatch(r'    (%\w+) = index i8 (?:\[projection=array_element\] )?(%\w+), (\d+)\n',lines[j])
   st=re.fullmatch(r'    store ([iu]\d+|f\d+) ([^,]+), (%\w+)\n',lines[j])
   if idx and idx[2] in positions:
    positions[idx[1]]=positions[idx[2]]+int(idx[3]);removed.add(idx[1]);direct &= idx[2]==base
   elif st and st[3] in positions:
    width=int(st[1][1:])//8;assert width
    stores.append((positions[st[3]],width,st[1],st[2]))
   else:break
   j+=1
  if not stores:continue # Later ordinary address uses remain intact.
  offset=0
  for at,width,ty,value in stores:
   assert at==offset,(slot,stores);offset+=width
  assert offset==size,(slot,offset,size)
  rest=''.join(lines[j:])
  # Local SSA names restart at each function; only this function is relevant.
  rest=rest.split('\n}\n',1)[0]
  assert not any(re.search(re.escape(v)+r'(?!\w)',rest) for v in removed),(slot,removed)
  data=tuple((ty,value) for _,_,ty,value in stores)
  if direct and len(stores)>8 and all(value=='0' for _,_,_,value in stores):data=(('zero',str(size)),)
  key=(size,align,data)
  if key not in images:images[key]='@__pa92_array_'+str(len(images))
  symbol=images[key]
  out.append(f'    copyobj {size}x{align} {symbol}, {base}\n');i=j
  records.append(dict(slot=slot,bytes=size,alignment=align,data=data,extracted_stores=stores,symbol=symbol))
 assert sorted(r['slot'] for r in records)==sorted(slots),(slots,records)
 globals=''.join('global '+symbol+' [binding=internal, storage=readonly] = {\n'+''.join(f'  {ty} {v}\n' for ty,v in data)+'}\n' for (_,_,data),symbol in images.items())
 return globals+''.join(out),records
changes=[]
def change(path,proof,transform):
 path='pa19/tests/'+path+'.ref';before=old(path);after=transform(before)
 # Canonical presentation requires global declarations before functions.
 declarations=re.findall(r'^declare global [^\n]+\n',after,re.M)
 if declarations:
  after=re.sub(r'^declare global [^\n]+\n','',after,flags=re.M)
  after=''.join(declarations)+after
 assert after!=before
 changes.append(dict(path=path,before_sha256=sha(before),after_sha256=sha(after),proof=proof,text=after))
arrays={
 'general/100-inherited-using-alias-out-of-class-specialization-member':['lhs','rhs'],
 'general/200-array-reference-deduction':['values'],
 'general/300-array-qualified-member-type-sfinae':['text'],
 'spec/400-out-of-class-overloaded-member-template-definition':['values'],
 'spec/500-nondeduced-qualified-member-type-allows-conversion':['text'],
}
for path,slots in arrays.items():
 def repair(text,slots=slots):
  after,records=array_images(text,slots)
  return after
 change(path,'PA16 automatic scalar array contract; N3485 [dcl.init.aggr], [dcl.init.string]',repair)
def sub(text,pattern,replacement,count=1,flags=0):
 out,n=re.subn(pattern,replacement,text,flags=flags);assert n==count,(pattern,n);return out
change('general/100-intermediate-type-transform-value-nontype','N3485 [temp.inst]/1,8,10; [basic.def.odr]/2',
 lambda s:sub(s,r'^global @[^\n]+\n\n',''))
def structured(s):
 s=sub(s,r'^global @[^\n]+\n','',count=3,flags=re.M)
 # Source declarations: reference storage for is_convertible_impl<...>::from
 # and the two namespace pick overloads. No definition is demanded by sizeof.
 return '''declare global @from : ptr [binding=weak, object=_ZN19is_convertible_implIP13version_2_tagP13case_fold_tagE4fromE]
declare function @pick_false(%arg : obj<1x1>) -> i8 [binding=strong, object=_Z4pick5bool_ILb0EE]
declare function @pick_true(%arg : obj<1x1>) -> i32 [binding=strong, object=_Z4pick5bool_ILb1EE]
'''+s
change('general/100-structured-bool-boost-convertible-mpl-overload','N3485 [temp.inst]/1,8,10; [expr.sizeof]/1; retain source declarations',structured)
change('general/500-dependent-qualified-sizeof-static-member','N3485 [temp.inst]/1,8,10; [expr.sizeof]/1',
 lambda s:sub(s,r'^global (@\S+) (\[[^\n]+\]) = \{\n  i64 1\n  i64 2\n  i64 3\n\}',r'declare global \1 \2'))
for path in ('general/500-source-owner-member-template-sfinae-default','general/500-member-template-conditional-alias-trailing-return'):
 change(path,'N3485 [temp.inst]/1,8,10: unused npos definition remains dormant',
  lambda s:sub(s,r'^global (@\S+__npos : i32 \[[^\n]+\]) = 0$',r'declare global \1',flags=re.M))
def discard(s):
 m=re.search(r'^    (%\w+) = load i32 (%\w+)\n',s,re.M);assert m
 assert not re.search(re.escape(m[1])+r'(?!\w)',s[m.end():])
 assert re.search(re.escape(m[2])+r' = call ptr ',s[:m.start()])
 return s[:m.start()]+s[m.end():]
change('general/300-deleted-return-sfinae-same-parameter-list','N3485 [expr]/11 and [expr.static.cast]/6',discard)
def const_ref(s):
 s=sub(s,r'(global @_1 : ptr [^\n]*= )zero',r'\1addr @free1')
 return sub(s,r'function @__cppgm_init\(\) -> void \[role=init[^\n]*\n  block \^entry:\n    %t1 = addr @free1\n    store ptr %t1, @_1\n    return void\n}\n?', '')
change('general/400-nonmember-template-compound-assignment-const-lhs','N3485 [basic.start.init]/2; [expr.const]',const_ref)
change('spec/100-explicit-instantiation-after-explicit-specialization-no-effect','N3485 [temp.explicit]/5',
 lambda s:sub(s,r', object_root=yes',''))
change('general/400-qualified-member-variable-template-class-value','PA16 constant object initialization; N3485 [basic.start.init]/2',
 lambda s:sub(s,r'function @__cppgm_init\(\) -> void \[role=init[^\n]*\n  block \^entry:\n    %t1 = addr @[^\n]+\n    return void\n}\n?', ''))
for c in changes:
 path=ROOT/c['path']
 if '--apply' in sys.argv: path.write_text(c['text'])
 else: assert path.read_text()==c['text'],c['path']
 del c['text']
manifest=ROOT/'student.tests/pa19/reference92-revisions.json'
record=dict(entry=ENTRY,bundle='c2f713cd70d06170632bfde3e75dd6fe1aa44d98',revisions=changes)
if '--apply' in sys.argv: manifest.write_text(json.dumps(record,indent=2)+'\n')
else: assert json.loads(manifest.read_text())==record
print('Verified',len(changes),'independently reconstructed reference revisions.')
