#!/usr/bin/env python3
"""Header truth survives full unrolling as a value, including parallel phis."""
import argparse,json,pathlib,subprocess
ROOT=pathlib.Path(__file__).resolve().parents[2]
p=argparse.ArgumentParser();p.add_argument('out',type=pathlib.Path);p.add_argument('--entry',action='store_true');args=p.parse_args()
out=args.out.resolve();out.mkdir(parents=True,exist_ok=True)
opt=out.parent/'entry-lowiropt' if args.entry else ROOT/'dev/lowiropt'
records=[]
def run(command,required=True):
 command=list(map(str,command));r=subprocess.run(command,capture_output=True,text=True,timeout=60)
 records.append(dict(command=command,exit_code=r.returncode,stdout=r.stdout,stderr=r.stderr))
 (out/'runs.json').write_text(json.dumps(records,indent=2)+'\n')
 if required:assert r.returncode==0,(command,r.stderr[-3000:],r.stdout[-1000:])
 return r
functions=[];expected=[]
for trips in [0,1,2,3,4,5]:
 for invert in [False,True]:
  for mode in ['store','backedge','body','call']:
   name='truth'+str(len(expected));truth=int(not invert)
   observed=7 if trips==0 else truth
   result={'store':observed,'backedge':7 if trips==0 else truth,'body':11+trips*truth,'call':observed}[mode]
   # Include the exit comparison so continuing/exit snapshots cannot conflate.
   result=100*result+int(invert)
   expected.append((name,result))
   phi=' %acc = phi i64 [^entry: 7, ^body: %c]\n' if mode=='backedge' else ' %acc = phi i64 [^entry: 11, ^body: %sum]\n' if mode=='body' else ''
   effect={'store':'store volatile i64 %c, @seen','backedge':'nop','body':'%sum = binary add i64 %acc, %c','call':'call void @touch(%c)'}[mode]
   result_read='%v = load volatile i64 @seen' if mode in ['store','call'] else '%v = copy i64 %acc'
   functions.append(f'''function @{name}() -> i64 [no_inline=yes] {{
block ^entry:
 store volatile i64 7, @seen
 jump ^head
block ^head:
 %i = phi i64 [^entry: 0, ^body: %next]
{phi} %c = cmp {'ge' if invert else 'lt'} i64 %i, {trips}
 branch %c, {'^exit, ^body' if invert else '^body, ^exit'}
block ^body:
 {effect} !dbg("truth.cpp", 12, 3)
 %next = binary add i64 %i, 1
 jump ^head
block ^exit:
 {result_read}
 %hundred = binary mul i64 %v, 100
 %r = binary add i64 %hundred, %c
 return i64 %r
}}''')
main=['function @main() -> i32 [role=entry] {block ^entry:','%bad0 = const i64 0']
for n,(name,value) in enumerate(expected):
 main += [f'%v{n} = call i64 @{name}()',f'%b{n} = cmp ne i64 %v{n}, {value}',f'%bad{n+1} = binary or i64 %bad{n}, %b{n}']
main += [f'%r = convert trunc i32 i64 %bad{len(expected)}','return i32 %r','}']
source=out/'truth.lowir';source.write_text('''global @seen : i64 = 0
function @touch(%v : i64) -> void [no_inline=yes] {
block ^entry: store volatile i64 %v, @seen return void }
'''+ '\n'.join(functions+main))
failures=[]
for level in range(4):
 ir=out/f'O{level}.lowir';run([opt,f'-O{level}','-o',ir,source])
 for path in ['direct','replay','native']:
  exe=out/f'{path}{level}';obj=out/'truth.o'
  if path=='native':command=[ROOT/'dev/lowir2native','-o',exe,ir]
  else:
   driver=out.parent/'entry-cppgm++' if args.entry else ROOT/'dev/cppgm++'
   command=[driver,'-c',f'-O{level}' if path=='direct' else '-O0','-o',obj,source if path=='direct' else ir]
  r=run(command,required=not args.entry)
  if r.returncode:failures.append((level,path,'compile'));continue
  if path!='native':run(['g++',obj,'-o',exe])
  r=run([exe],required=not args.entry)
  if r.returncode:failures.append((level,path,'execute'))
 if level==3 and not args.entry:
  text=ir.read_text();assert 'truth.cpp' in text
  run([ROOT/'dev/lowir2native','--dump-machine-ir',out/'truth.mir',ir])
  assert 'truth.cpp' in (out/'truth.mir').read_text()
# Without an exported comparison, the broken overlay leaves an undefined
# temporary instead of incorrectly using the comparison's final exit value.
source=out/'unexported.lowir'
source.write_text('''global @seen : i64 = 0
function @main() -> i32 [role=entry] {
block ^entry: jump ^head
block ^head:
 %i = phi i64 [^entry: 0, ^body: %next]
 %c = cmp lt i64 %i, 3
 branch %c, ^body, ^exit
block ^body:
 store volatile i64 %c, @seen
 %next = binary add i64 %i, 1
 jump ^head
block ^exit:
 %v = load volatile i64 @seen
 %bad = cmp ne i64 %v, 1
 %r = convert trunc i32 i64 %bad
 return i32 %r
}
''')
ir=out/'unexported-out.lowir';run([opt,'-O3','-o',ir,source])
r=run([ROOT/'dev/cppgm++','-c','-O0','-o',out/'unexported.o',ir],required=not args.entry)
if args.entry:assert r.returncode!=0 and 'undefined' in r.stderr
else:
 run(['g++',out/'unexported.o','-o',out/'unexported']);run([out/'unexported'])
if args.entry:assert failures and all(v[0]==3 for v in failures),failures
else:assert not failures
print(json.dumps(dict(cases=len(expected),levels=4,paths=3,entry_failures=failures)))
