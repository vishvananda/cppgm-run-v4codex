#!/usr/bin/env python3
"""Reconstruct PA20 automatic-array contract corrections from entry references.
No student compiler output is read. Run --apply once, otherwise verifies exactly.
"""
from pathlib import Path
import hashlib,json,re,subprocess,sys
ROOT=Path(__file__).resolve().parents[2]
ENTRY='ef897177cd34e8bf1e878a7eb94208237240c0f1'
def sha(s):return hashlib.sha256(s.encode()).hexdigest()
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
  if key not in images:images[key]='@__pa95_array_'+str(len(images))
  symbol=images[key]
  out.append(f'    copyobj {size}x{align} {symbol}, {base}\n');i=j
  records.append(dict(slot=slot,bytes=size,alignment=align,data=data,extracted_stores=stores,symbol=symbol))
 assert sorted(r['slot'] for r in records)==sorted(slots),(slots,records)
 globals=''.join('global '+symbol+' [binding=internal, storage=readonly] = {\n'+''.join(f'  {ty} {v}\n' for ty,v in data)+'}\n' for (_,_,data),symbol in images.items())
 return globals+''.join(out),records
arrays={
 '100-array-brace-init':['a'],
 '100-array-brace-string-literal':['value'],
 '100-range-for-array':['a'],
 '200-range-for-adl-begin-end':['data'],
 '200-range-for-adl-ignores-enclosing-member-begin':['data'],
 '200-range-for-array-const-auto-ref':['a'],
 '200-range-for-array-const-ref':['a'],
 '200-range-for-array-value-categories':['values'],
 '200-range-for-inherited-member-begin-end':['data'],
 '200-range-for-member-begin-end':['data'],
 '200-template-member-range-for-local':['xs'],
}
changes=[]
for name,slots in arrays.items():
 path='pa20/tests/general/'+name+'.ref'
 before=subprocess.check_output(['git','show',ENTRY+':'+path],cwd=ROOT,text=True)
 after,records=array_images(before,slots)
 row=dict(path=path,before_sha256=sha(before),after_sha256=sha(after),arrays=records,
  source_sha256=sha((ROOT/path).with_suffix('.t').read_text()))
 changes.append(row)
 if '--apply' in sys.argv:(ROOT/path).write_text(after)
 else:assert (ROOT/path).read_text()==after,path
record=dict(entry=ENTRY,bundle_revision='c2f713cd70d06170632bfde3e75dd6fe1aa44d98',
 local_revision='pa20-array-images-95',proof='pa20/reference-corrections95.md',revisions=changes)
manifest=ROOT/'student.tests/pa20/reference95-revisions.json'
if '--apply' in sys.argv:manifest.write_text(json.dumps(record,indent=2)+'\n')
else:assert json.loads(manifest.read_text())==json.loads(json.dumps(record))
print('Verified',len(changes),'independently reconstructed array reference corrections.')
