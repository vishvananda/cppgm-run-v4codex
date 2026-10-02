#!/usr/bin/env python3
"""Compare emitted text, symbols and CFI after serialized LowIR reconstruction."""
import hashlib,json,pathlib,struct,subprocess,sys
root=pathlib.Path(__file__).resolve().parents[2];out=pathlib.Path(sys.argv[1]).resolve()
trace=json.loads((out/'trace.json').read_text());result=dict(compiler_sha256=trace['compiler_sha256'],commands=[],images={})
assert hashlib.sha256((root/'dev/cppgm++').read_bytes()).hexdigest()==result['compiler_sha256']
def sections(obj):
 data=obj.read_bytes();assert data[:6]==b'\x7fELF\x02\x01'
 offset=struct.unpack_from('<Q',data,40)[0];width,count,names=struct.unpack_from('<HHH',data,58)
 headers=[struct.unpack_from('<IIQQQQIIQQ',data,offset+i*width) for i in range(count)]
 h=headers[names];strings=data[h[4]:h[4]+h[5]]
 return {strings[h[0]:].split(b'\0')[0].decode(): (h[1:4],h[5:],data[h[4]:h[4]+h[5]] if h[1]!=8 else b'') for h in headers}
for name,identity in trace['images'].items():
 views={};parts={}
 for label,suffix in [('direct','.o'),('reparsed','.roundtrip.o')]:
  obj=out/(name+suffix)
  assert hashlib.sha256(obj.read_bytes()).hexdigest()==identity['direct_object_sha256' if label=='direct' else 'roundtrip_object_sha256']
  parts[label]=sections(obj)
  views[label]={}
  for kind,args in [('text',['objdump','-dr']),('symbols',['readelf','-Ws']),('cfi',['readelf','--debug-dump=frames'])]:
   p=subprocess.run([*args,str(obj)],capture_output=True,text=True,timeout=45)
   result['commands'].append(dict(args=[*args,str(obj)],status=p.returncode,stdout=p.stdout,stderr=p.stderr));assert not p.returncode
   views[label][kind]='\n'.join(p.stdout.splitlines()[3:]) if kind=='text' else p.stdout
 result['images'][name]={kind:views['direct'][kind]==views['reparsed'][kind] for kind in views['direct']}
 result['images'][name]['sections_except_symbol_strings']=all(parts['direct'][key]==parts['reparsed'][key] for key in parts['direct'] if key not in ['.strtab','.symtab']) and parts['direct'].keys()==parts['reparsed'].keys()
 assert all(result['images'][name].values()),name
 (out/'inspection.json').write_text(json.dumps(result,indent=2)+'\n')
print(len(result['commands']),'ELF inspection commands pass')
