#!/usr/bin/env python3
"""Check declaration-pattern conversion encodings from the typed source path."""
from pathlib import Path
import json,re,subprocess,sys
ROOT=Path(__file__).resolve().parents[2]
WORK=Path(sys.argv[1]);WORK.mkdir(parents=True,exist_ok=True)
# Itanium: nested name N/K, conversion cv<type>, template args I...E,
# no result type on a conversion function. doc/itanium-mangling.txt.
cases={
 'scalar':('struct S{template<class T>operator T()const{return T();}};int main(){S s;int i=s;long l=s;int j=s;return i+l+j;}',['_ZNK1ScvT_IiEEv','_ZNK1ScvT_IlEEv']),
 'pointer':('struct S{template<class T>operator T*()const{return 0;}};int main(){S s;int*p=s;const int*q=s;return p!=q;}',['_ZNK1ScvPT_IiEEv','_ZNK1ScvPT_IKiEEv']),
 'reference':('int n;struct S{template<class T>operator T&()const{return n;}};int main(){S s;int&r=s;return r;}',['_ZNK1ScvRT_IiEEv']),
 'const_reference':('int n;struct S{template<class T>operator const T&()const{return n;}};int main(){S s;const int&r=s;return r;}',['_ZNK1ScvRKT_IiEEv']),
}
rows=[]
for name,(source,expected) in cases.items():
 src=WORK/(name+'.cpp');src.write_text(source);ir=src.with_suffix('.lowir')
 r=subprocess.run([ROOT/'dev/cppgm++','--emit-lowir','-O0','--validate-lowir','-o',ir,src],capture_output=True,text=True,timeout=30)
 actual=re.findall(r'^function .*object=([^,\] ]+)',ir.read_text(),re.M) if not r.returncode else []
 actual=[s for s in actual if 'cv' in s]
 row=dict(name=name,source=source,expected=expected,actual=actual,exit=r.returncode,diagnostic=r.stderr,passed=r.returncode==0 and sorted(actual)==sorted(expected));rows.append(row)
(WORK/'results.json').write_text(json.dumps(rows,indent=2)+'\n')
assert all(r['passed'] for r in rows),rows
print('Four conversion ABI controls pass; repeated specialization emits once.')
