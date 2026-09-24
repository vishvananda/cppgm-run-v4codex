#!/usr/bin/env python3
"""Verify internal NTTP referents remain distinct across merged source TUs."""
from pathlib import Path
import json,re,subprocess,sys
ROOT=Path(__file__).resolve().parents[2]
WORK=Path(sys.argv[1]);WORK.mkdir(parents=True,exist_ok=True)
CASES={
 'object':'static int value=VALUE;template<int*P>int read(){return *P;}int PUBLIC(){return read<&value>();}',
 'reference':'static int value=VALUE;template<int&R>struct Read{static int get(){return R;}};int PUBLIC(){return Read<value>::get();}',
 'function':'static int value(){return VALUE;}template<int(*P)()>int read(){return P();}int PUBLIC(){return read<&value>();}',
 'function_class':'static int value(){return VALUE;}template<int(*P)()>struct Read{static int get(){return P();}};int PUBLIC(){return Read<&value>::get();}',
 'object_pack':'static int value=VALUE;template<int*...P>struct A{};template<class T>int read(T){return value;}int PUBLIC(){return read(A<&value>());}',
}
rows=[]
for name,source in CASES.items():
 paths=[]
 for number in (1,2):
  p=WORK/(name+str(number)+'.cpp');p.write_text(source.replace('VALUE',str(number+2)).replace('PUBLIC','read'+str(number)));paths.append(p)
 main=WORK/'main.cpp';main.write_text('int read1();int read2();int main(){return read1()!=3||read2()!=4;}');paths.append(main)
 ir=WORK/(name+'.lowir');exe=WORK/name
 r=subprocess.run([ROOT/'dev/cppgm++','--emit-lowir','-O0','--validate-lowir','-o',ir,*paths],capture_output=True,text=True)
 row=dict(name=name,source=source,compiler_exit=r.returncode,diagnostic=r.stderr)
 if not r.returncode:
  b=subprocess.run([ROOT/'dev/lowir2native-ref','-O0','-o',exe,ir],capture_output=True,text=True)
  row.update(backend_exit=b.returncode,backend_diagnostic=b.stderr)
  if not b.returncode:row['native_exit']=subprocess.run([exe]).returncode
 row['passed']=row.get('native_exit')==0;rows.append(row);print(name,'PASS' if row['passed'] else 'FAIL',flush=True)
(WORK/'results.json').write_text(json.dumps(rows,indent=2)+'\n')
assert all(r['passed'] for r in rows),rows
