#!/usr/bin/env python3
"""Raw signature publication and complete-class source default obligations."""
from pathlib import Path
import subprocess,sys,tempfile
ROOT=Path(__file__).resolve().parents[2]
BINARY=Path(sys.argv[1]).resolve() if len(sys.argv)>1 else ROOT/'dev/cppgm++'
CASES={
 'missing_default': 'template<class T>struct C{int get(int value=missing()){return value;}};',
 'late_type_value': 'template<class T>struct C{int get(int value=Later){return value;}using Later=int;};',
 'private_default': 'struct P{private:static int value();};template<class T>struct C{int get(int value=P::value()){return value;}};',
 'demanded_default_body': 'template<class T>struct C{int get(int value=make()){return value;}static int make(){return T::missing;}};int main(){C<int> c;return c.get();}',
 'later_added_default': 'template<class T>struct C{int get(int);};template<class T>int C<T>::get(int value=3){return value;}',
 'nested_type_value': 'template<class T>struct C{struct Inner{int get(int value=Later){return value;}};using Later=int;};',
}
CLAUSES={
 'missing_default':'N3485 [basic.lookup.unqual], [dcl.fct.default]/5',
 'late_type_value':'N3485 [basic.scope.class]/1, [class.mem]/2, [expr.prim.general]',
 'private_default':'N3485 [dcl.fct.default]/5, [class.access]',
 'demanded_default_body':'N3485 [temp.inst]/1, [temp.dep]',
 'later_added_default':'N3485 [dcl.fct.default]/6',
 'nested_type_value':'N3485 [basic.scope.class]/1, [class.mem]/2, [expr.prim.general]',
}
REDUCERS=['signature-source-identity.t','signature-query-reducer.t','signature-late-default.t']
if __name__=='__main__':
 with tempfile.TemporaryDirectory(prefix='pa14-signature-publications-') as tmp:
  work=Path(tmp)
  for name,source in CASES.items():
   path=work/(name+'.cpp');path.write_text(source)
   p=subprocess.run([BINARY,'--emit-lowir','-O0','-o',work/'rejected.lowir',path],capture_output=True,text=True,timeout=60)
   assert p.returncode==1,(name,p.returncode,p.stderr)
   assert not any(x in p.stderr for x in ['AddressSanitizer','UndefinedBehaviorSanitizer','runtime error:']),(name,p.stderr)
   print(name,'rejection PASS')
  for name in REDUCERS:
   ir=work/(name+'.lowir');exe=work/(name+'.exe');source=ROOT/'student.tests/pa14'/name
   for command in ([BINARY,'--emit-lowir','-O0','--validate-lowir','-o',ir,source],
                   [ROOT/'dev/lowir2native-ref','-O0','-o',exe,ir],[exe]):
    p=subprocess.run(list(map(str,command)),capture_output=True,text=True,timeout=60)
    assert p.returncode==0,(name,command,p.returncode,p.stderr)
    assert not any(x in p.stderr for x in ['AddressSanitizer','UndefinedBehaviorSanitizer','runtime error:']),(name,p.stderr)
   print(name,'native PASS')
