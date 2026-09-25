#!/usr/bin/env python3
"""Reproduce only proved reference edits from entry oracles; never read student IR."""
from pathlib import Path
import hashlib,json,re,subprocess,sys
ROOT=Path(__file__).resolve().parents[2]
ENTRY='48c864abc19c24c4e9f5706a86109c023e2da7f7'
ARRAYS={
 'pa17/tests/general/200-alias-template-template-argument-use-scope.ref':['a'],
 'pa17/tests/general/400-local-value-shadows-template-relational.ref':['values'],
 'pa18/tests/general/100-forwarded-const-reference-multidimensional-array-deduction.ref':['value','copy'],
 'pa18/tests/general/100-function-template-fixed-over-trailing-pack-fallback.ref':['data'],
 'pa18/tests/general/200-nested-array-reference-partial-ordering.ref':['a'],
 'pa18/tests/general/200-range-array-reference-mutable-begin.ref':['data'],
 'pa18/tests/general/300-function-template-nested-alias-explicit-call.ref':['src_buf','dst_buf'],
 'pa18/tests/general/400-array-bound-expression-is-nondeduced.ref':['matrix','vector'],
 'pa18/tests/general/500-constructor-sfinae-namespace-constant-symbol.ref':['text'],
 'pa18/tests/spec/100-constructor-template-braced-array-bound-deduction.ref':['parser_buf'],
 'pa18/tests/spec/100-function-template-array-bound-deduction.ref':['a'],
 'pa18/tests/spec/100-function-template-array-bound-shared-deduction.ref':['a','b'],
 'pa18/tests/spec/200-array-reference-cv-partial-ordering.ref':['data','const_data'],
 'pa18/tests/spec/200-nondeduced-qualified-member-type-allows-conversion.ref':['text'],
 'pa18/tests/spec/300-defaulted-enable-if-after-array-bound-deduction.ref':['a','b'],
}
def sha(s):return hashlib.sha256(s.encode()).hexdigest()
def old(path):return subprocess.check_output(['git','show',ENTRY+':'+path],cwd=ROOT,text=True)
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
  if key not in images:images[key]='@__pa83_array_'+str(len(images))
  symbol=images[key]
  out.append(f'    copyobj {size}x{align} {symbol}, {base}\n');i=j
  records.append(dict(slot=slot,bytes=size,alignment=align,data=data,extracted_stores=stores,symbol=symbol))
 assert sorted(r['slot'] for r in records)==sorted(slots),(slots,records)
 globals=''.join('global '+symbol+' [binding=internal, storage=readonly] = {\n'+''.join(f'  {ty} {v}\n' for ty,v in data)+'}\n' for (_,_,data),symbol in images.items())
 return globals+''.join(out),records
# Keep the writer auditable: changes retain all later instructions and names.
changes=[]
for path,slots in ARRAYS.items():
 before=old(path)
 source=before
 if path.endswith('200-alias-template-template-argument-use-scope.ref'):
  # I<T> has no fields, bases, user constructors/destructor or address escape.
  # The discarded empty temporaries have no observable effect (as-if rule).
  for slot in ('discard__1','discard__2'):
   source,n=re.subn(r'  slot \$'+slot+r' : obj<1x1>\n','',source);assert n==1
   source,n=re.subn(r'    (%\w+) = addr \$'+slot+r'\n    zeroinit 1x1 \1\n','',source);assert n==1
 # The transformation returns the independent image facts as its manifest.
 after,records=array_images(source,slots)
 changes.append(dict(path=path,before_sha256=sha(before),after_sha256=sha(after),proof='PA16 README Assignment Boundary automatic scalar arrays',arrays=records,text=after))
p='pa18/tests/spec/100-constexpr-union-active-pack-constructor.ref';before=old(p);after=before
for name,data in [('inactive','  u8 0\n  zero 3'),('active','  i32 1')]:
 after,n=re.subn(r'(global @'+name+r' [^\n]*\n)  zero 4',lambda m:m[1]+data,after);assert n==1
# Constant initialization precedes every dynamic initializer; preserve ctor bodies.
after,n=re.subn(r'function @[^\n]+\[role=init[^\n]*\n.*?^}\n?', '',after,flags=re.M|re.S);assert n==1
changes.append(dict(path=p,before_sha256=sha(before),after_sha256=sha(after),proof='N3485 3.6.2 [basic.start.init]/2; 7.1.5 [dcl.constexpr]/9',text=after))
p='pa18/tests/general/300-empty-pack-unknown-bound-array-lowir.ref.exit_status';before=old(p);after='EXIT_FAILURE\n';assert before.strip()=='EXIT_SUCCESS'
changes.append(dict(path=p,before_sha256=sha(before),after_sha256=sha(after),proof='N3485 8.5.1 [dcl.init.aggr]/4 forbids empty unknown-bound initialization',text=after))
for c in changes:
 p=ROOT/c['path']
 if '--apply' in sys.argv:p.write_text(c['text'])
 else:assert p.read_text()==c['text'],c['path']
 del c['text']
manifest=ROOT/'student.tests/pa18/reference83-revisions.json'
if '--apply' in sys.argv:manifest.write_text(json.dumps(dict(entry=ENTRY,bundle='c2f713cd70d06170632bfde3e75dd6fe1aa44d98',revisions=changes),indent=2)+'\n')
else:assert json.loads(manifest.read_text())['revisions']==json.loads(json.dumps(changes))
print('Verified',len(changes),'independently derived reference revisions.')
