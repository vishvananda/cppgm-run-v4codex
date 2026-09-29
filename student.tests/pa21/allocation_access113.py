#!/usr/bin/env python3
"""New-expression access and declaration/body demand separation."""
from pathlib import Path
import hashlib,json,subprocess,sys
CC,WORK=[Path(x).resolve() for x in sys.argv[1:]]
WORK.mkdir(parents=True,exist_ok=True)
CASES=[('private_nonthrow', 'struct M{M()noexcept{} private:static void operator delete(void*);};int main(){new M;}', False), ('private_array_nonthrow', 'struct M{M()noexcept{} private:static void operator delete[](void*);};int main(){new M[2];}', False), ('undemanded_delete_body', 'template<class T>struct M{M()noexcept{} static void operator delete(void*){T::missing();}};int main(){M<int>*p=new M<int>;return p==0;}', True), ('global_new_private_delete', 'struct M{M()noexcept{} private:static void operator delete(void*);};int main(){M*p=::new M;::delete p;}', True)]
rows=[]
for name,source,valid in CASES:
 src=WORK/(name+'.cpp');src.write_text(source)
 cmd=[str(CC),'--emit-lowir','-O0','--validate-lowir','-o',str(src.with_suffix('.lowir')),str(src)]
 p=subprocess.run(cmd,capture_output=True,text=True,timeout=30)
 rows.append(dict(name=name,source=source,valid=valid,command=cmd,exit=p.returncode,stderr=p.stderr,passed=(p.returncode==0)==valid))
 print(name,rows[-1]['passed'])
(WORK/'results.json').write_text(json.dumps(dict(compiler_sha256=hashlib.sha256(CC.read_bytes()).hexdigest(),rows=rows),indent=2)+'\n')
assert all(r['passed'] for r in rows)
