#!/usr/bin/env python3
"""Reduced class boundary observations; no oracle generation. Usage: WORK."""
from pathlib import Path
import hashlib, json, re, subprocess, sys
ROOT = Path(__file__).resolve().parents[2]
W = Path(sys.argv[1]).resolve(); W.mkdir(parents=True, exist_ok=True)
rows = []
sources = {}
for field in ('int', 'float', 'double', 'long'):
 for form in ('aggregate', 'constructor', 'private', 'template'):
  prefix = 'template<class T>' if form == 'template' else ''
  typ = 'T' if form == 'template' else field
  ctor = '' if form == 'aggregate' else ' A(int x):n(x){}'
  body = 'struct A{'+typ+' n;'+ctor+'};'
  if form == 'private': body = 'struct A{'+typ+' n;private:A(int x):n(x){} friend A make(int);};'
  target = 'A<'+field+'>' if form == 'template' else 'A'
  init = target+'{x}' if form == 'aggregate' else target+'(x)'
  # Floating aggregate list initialization uses an explicit conversion.
  if form == 'aggregate': init = target+'{static_cast<'+field+'>(x)}'
  source = prefix+body+target+' make(int x){return '+init+';}int main(){'+target+'(*p)(int)=make;'+target+' a=p(7);return a.n!=7;}'
  sources[field+'-'+form]=source
for field in ('int','float'):
 for result in ('A','T','typename Id<T>::type','Wrap<T>','typename Id<Wrap<T>>::type'):
  for seed in ('','A seed={};Wrap<int> seed2;'):
   defs='struct A{'+field+' n;};template<class T>struct Id{typedef T type;};template<class T>struct Wrap{int n;};'
   wrap='Wrap' in result; actual='Wrap<int>' if wrap else 'A'; arg='int' if wrap else 'A'
   source=defs+seed+'template<class T>'+result+' make(){return '+result+'{};}int main(){'+actual+'(*p)()=make<'+arg+'>;'+actual+' a=p();return a.n!=0;}'
   sources[field+'-'+str(list(('A','T','typename Id<T>::type','Wrap<T>','typename Id<Wrap<T>>::type')).index(result))+'-'+str(bool(seed))]=source
sources['conversion-original']='struct A{float n;};A seed={0};struct X{A a;template<class T>operator T()const{return a;}}const x={};int main(){A a=x;return a.n!=0;}'
sources['conversion-int']=sources['conversion-original'].replace('float','int')
sources['conversion-non-template']=sources['conversion-original'].replace('template<class T>operator T()','operator A()')
sources['conversion-no-seed']=sources['conversion-original'].replace('A seed={0};','')
sources['friend-reduced']='template<class T>struct A{int n;template<class Y>friend A<Y> make(int);private:template<class U>A(U x):n(x){}};template<class T>A<T> make(int x){return A<T>(x);}int main(){A<int>a=make<int>(7);return a.n!=7;}'
sources['friend-public']=sources['friend-reduced'].replace('private:','')
sources['friend-non-template-ctor']=sources['friend-reduced'].replace('template<class U>A(U x)','A(int x)')
for name,source in list(sources.items()):
 if '(*p)()' in source: sources[name+'-direct']=re.sub(r'\w+(?:<int>)?\(\*p\)\(\)=make<([^;]+)>;', '', source).replace('a=p()','a=make<'+('int' if 'Wrap<int> a' in source else 'A')+'>()')
for name,source in sources.items():
  src=W/(name+'.cpp');src.write_text(source)
  for label,cc in (('student',ROOT/'dev/cppgm++'),('reference',ROOT/'reference-binaries/cppgm++')):
   ir=W/(name+'-'+label+'.lowir'); exe=ir.with_suffix('.exe')
   r=subprocess.run([cc,'--emit-lowir','-O0','-o',ir,src],capture_output=True,text=True)
   row=dict(name=name,label=label,source=source,compiler_exit=r.returncode,diagnostic=r.stderr)
   if r.returncode==0:
    text=ir.read_text();row['lowir']=text
    row['make_signature']=next((s for s in text.splitlines() if s.startswith('function @make(')),None)
    b=subprocess.run([ROOT/'dev/lowir2native-ref','-O0','-o',exe,ir],capture_output=True,text=True)
    row.update(backend_exit=b.returncode,backend_diagnostic=b.stderr)
    if b.returncode==0:row['native_exit']=subprocess.run([exe],timeout=10).returncode
   rows.append(row);print(name,label,row.get('make_signature',r.stderr),row.get('native_exit'),flush=True)
(W/'results.json').write_text(json.dumps(rows,indent=2)+'\n')
