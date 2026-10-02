#!/usr/bin/env python3
"""Reconstruct hosted objects from the public LowIR view and compare consumed bytes."""
import hashlib, json, pathlib, re, struct, subprocess, sys
root=pathlib.Path(__file__).resolve().parents[2]
out=pathlib.Path(sys.argv[1]).resolve();out.mkdir(parents=True,exist_ok=True)
cc=root/'dev/cppgm++'
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
r=dict(compiler_sha256=sha(cc),commands=[],images={})
def save():(out/'trace.json').write_text(json.dumps(r,indent=2)+'\n')
def run(args):
 p=subprocess.run(list(map(str,args)),capture_output=True,text=True,timeout=45)
 r['commands'].append(dict(args=list(map(str,args)),status=p.returncode,stdout=p.stdout,stderr=p.stderr));save()
 assert p.returncode==0,r['commands'][-1]
 return p.stdout
def sections(obj):
 data=obj.read_bytes();assert data[:6]==b'\x7fELF\x02\x01'
 offset=struct.unpack_from('<Q',data,40)[0];width,count,names=struct.unpack_from('<HHH',data,58)
 headers=[struct.unpack_from('<IIQQQQIIQQ',data,offset+i*width) for i in range(count)]
 h=headers[names];strings=data[h[4]:h[4]+h[5]]
 return {strings[h[0]:].split(b'\0')[0].decode():data[h[4]:h[4]+h[5]] for h in headers if h[1]!=8}
def unwind(obj):
 data=obj.read_bytes();offset=struct.unpack_from('<Q',data,40)[0]
 width,count,names=struct.unpack_from('<HHH',data,58)
 h=[struct.unpack_from('<IIQQQQIIQQ',data,offset+i*width) for i in range(count)]
 def raw(i):return data[h[i][4]:h[i][4]+h[i][5]]
 def string(table,i):return table[i:].split(b'\0')[0].decode()
 section_names=[string(raw(names),v[0]) for v in h]
 ei=section_names.index('.eh_frame');ri=section_names.index('.rela.eh_frame')
 symbol_table=h[ri][6];symbols=[]
 for start in range(0,h[symbol_table][5],24):
  name,info,other,section,value,size=struct.unpack_from('<IBBHQQ',raw(symbol_table),start)
  symbols.append(string(raw(h[symbol_table][6]),name) or section_names[section])
 relocs={}
 for start in range(0,h[ri][5],24):
  at,info,addend=struct.unpack_from('<QQq',raw(ri),start)
  relocs[at]=(symbols[info>>32],info&0xffffffff,addend)
 eh=raw(ei);records={};cies={};start=0
 while start<len(eh):
  length=struct.unpack_from('<I',eh,start)[0]
  if not length:break
  end=start+4+length;pointer=struct.unpack_from('<I',eh,start+4)[0]
  if not pointer:cies[start]=eh[start+8:end].hex()
  else:
   owner=relocs[start+8]
   # Initial-location and LSDA relocation slots are zero in the object.
   # Their named target classes remain checked; table ordering is incidental.
   edges=[(at-start,v[0],v[1]) for at,v in relocs.items() if start<=at<end]
   records[str(owner)]=(cies[start+4-pointer],eh[start+8:end].hex(),sorted(edges))
  start=end
 return records
objects=[p for p in sorted((root/'obj/dev').rglob('*.o')) if 'entry' not in p.parts and not p.name.startswith('test_runner')]
adapter=out/'adapter'
run(['g++','-std=c++11','-O2','-I'+str(root/'dev/src'),root/'student.tests/pa30/ir-object204.cpp',*objects,'-o',adapter])
inputs={s:root/f'student.tests/pa31/source205/{s}.cpp' for s in ['allocation-client','vbase-complete','inherited-default','list-exceptions']}
inputs.update({s:root/f'pa31/tests/link/700-hosted-{s}.t.1' for s in ['self-element-map-unique-ptr-runtime','vector-braced-temporary-dtor-link-smoke']})
for name,src in inputs.items():
 obj=out/(name+'.o');ir=out/(name+'.lowir');rebuilt=out/(name+'.rebuilt.o');mir=out/(name+'.mir')
 run([cc,'-O0','-c',src,'-o',obj]);plain=sha(obj)
 stats=run([cc,'-O0','-c','--stats',src,'-o',obj]);assert plain==sha(obj)
 run([cc,'-c','--emit-lowir','--validate-lowir',src,'-o',ir])
 canonical=out/(name+'.canonical');run([root/'dev/lowir',ir,'-o',canonical])
 run([root/'dev/lowir',canonical,'-o',out/'twice.lowir']);assert canonical.read_bytes()==(out/'twice.lowir').read_bytes()
 run([adapter,canonical,rebuilt,mir])
 a,b=sections(obj),sections(rebuilt)
 keys={k for k in a if k.startswith('.text')}
 assert keys=={k for k in b if k.startswith('.text')}
 assert all(a[k]==b[k] for k in keys),name
 # Compare disassembly including named relocations, unaffected by ELF string ordering.
 da=run(['objdump','-dr',obj]);db=run(['objdump','-dr',rebuilt])
 def disassembly(text):
  return sorted(section.strip() for section in text.split('Disassembly of section ')[1:])
 assert disassembly(da)==disassembly(db),name
 run(['readelf','--debug-dump=frames',obj]);run(['readelf','--debug-dump=frames',rebuilt])
 # Compare unrelocated FDE payloads by their named function section/offset.
 # readelf applies PC-relative LSDA/personality relocations, whose displayed
 # displacements vary with record ordering; the encoded CFI does not.
 assert unwind(obj)==unwind(rebuilt),name
 r['images'][name]=dict(source_sha256=sha(src),direct_object_sha256=plain,rebuilt_object_sha256=sha(rebuilt),lowir_sha256=sha(ir),mir_sha256=sha(mir),text_identical=True,function_cfi_identical=True,named_relocations_identical=True,telemetry_identical=True)
 if name!='allocation-client' and name!='vbase-complete':
  run(['g++',rebuilt,'-o',out/name]);run([out/name])
 save()
assert sha(cc)==r['compiler_sha256'];r['passed']=True;save()
print(len(r['commands']),'trace commands passed')
