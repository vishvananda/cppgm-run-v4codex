#!/usr/bin/env python3
"""Observe explicit-tag projection; compiler agreement is not a reference proof."""
import json,pathlib,subprocess
root=pathlib.Path(__file__).resolve().parent
out=pathlib.Path('/tmp/pa29-193');src=root/'source193'
cases={}
for owner,head,qual,decl in [
 ('ordinary','','Plain','struct Plain { BODY };'),
 ('direct','template<class T>','Box<T>','template<class T> struct Box { BODY };'),
 ('nested','template<class T>','Box<T>::Inner','template<class T> struct Box { struct Inner { BODY }; };'),
 ('deep','template<class T>','Box<T>::Inner::Leaf','template<class T> struct Box { struct Inner { struct Leaf { BODY }; }; };'),
 ('member_template','template<class T> template<class U>','Box<T>::Inner<U>','template<class T> struct Box { template<class U> struct Inner { BODY }; };')]:
 for mode in ['inline','outside','repeated','declaration']:
  tag='__attribute__((abi_tag("keep")))'
  body=tag+' static int value(int x)'+('{ return x+7; }' if mode=='inline' else ';')
  source=decl.replace('BODY',body)+'\n'
  name=qual.replace('<T>','<int>').replace('<U>','<long>')
  if mode in ['outside','repeated']:
   source+=head+'\n'+(tag+' ' if mode=='repeated' else '')+'int '+qual+'::value(int x){return x+7;}\n'
  source+='int use(int x){return '+name+'::value(x);}\n'
  if mode!='declaration':source+='int main(){return use(3)!=10;}\n'
  cases[owner+'-'+mode]=source
result={}
for name,source in cases.items():
 path=src/(name+'.cpp');path.write_text(source);result[name]={}
 for compiler in ['/tmp/pa29-193/entry','g++','clang++']:
  obj=out/(name+'.o');p=subprocess.run([compiler,'-std=c++11','-O0','-c',str(path),'-o',str(obj)],capture_output=True,text=True)
  symbols=subprocess.run(['nm',str(obj)],capture_output=True,text=True).stdout if p.returncode==0 else ''
  result[name][compiler]={'status':p.returncode,'stderr':p.stderr,'symbols':symbols}
(root/'evidence193'/'entry-probe.json').write_text(json.dumps(result,indent=2)+'\n')
for name,row in result.items():
 print(name,[(k.split('/')[-1],v['status'],[s.split()[-1] for s in v['symbols'].splitlines() if 'value' in s]) for k,v in row.items()])
