#!/usr/bin/env python3
"""Explicit generic EH/frame controls: results derived from LowIR semantics."""
import json, pathlib, subprocess, sys, tempfile
root=pathlib.Path(__file__).resolve().parents[2]
cc=pathlib.Path(sys.argv[1]).resolve() if len(sys.argv)>1 else root/'dev/lowir2native'
out=pathlib.Path(sys.argv[2]) if len(sys.argv)>2 else pathlib.Path(tempfile.mkdtemp(prefix='pa24-runtime131-'))
out.mkdir(parents=True,exist_ok=True)
cases={}
def main(body, helpers='', slots=''):
    return helpers+'\nfunction @main() -> i64 [role=entry] {\n'+slots+'\nblock ^entry:\n'+body+'\n}\n'
def add(name, text, status=0): cases[name]=(text,status)
for typ,value in [('i1','1'),('i8','-73'),('u8','253'),('i16','-12345'),('u16','54321'),('i32','-123456789'),('u32','3456789012'),('i64','-12345678901234'),('i128','-123456789012345678901234567890'),('f32','1.25'),('f64','-3.5'),('f80','1.234567890123456789L'),('ptr','@cell')]:
    decl='global @cell : i64 = 9\n'
    for depth in [0,1,8]:
        helpers=decl
        for k in range(depth):
            body=f'throw {typ} %x' if k==0 else f'call void @raise{k-1}(%x)\nreturn void'
            helpers+=f'function @raise{k}(%x : {typ}) -> void {{\nblock ^entry:\n{body}\n}}\n'
        throwing=f'throw {typ} {value}' if depth==0 else f'call void @raise{depth-1}({value})\neh_end\nreturn i64 99'
        add(f'payload-{typ}-{depth}',main(f'eh_try ^caught\n{throwing}\nblock ^caught:\n%x = exception {typ}\n%bad = cmp ne {typ} %x, {value}\nreturn i64 %bad',helpers))
add('cleanup-chain',main('''eh_try ^caught
call void @middle()
eh_end
return i64 90
block ^caught:
%x = exception i64
%v = load i64 @log
%b0 = cmp ne i64 %x, 42
%b1 = cmp ne i64 %v, 123
%b = binary or i64 %b0, %b1
return i64 %b''','''global @log : i64 = 0
function @append(%digit : i64) -> void {
block ^entry:
%old = load i64 @log
%scaled = binary mul i64 %old, 10
%next = binary add i64 %scaled, %digit
store i64 %next, @log
return void
}
function @leaf() -> void {
block ^entry:
eh_cleanup ^cleanup
call void @append(1)
throw i64 42
block ^cleanup:
call void @append(2)
resume
}
function @middle() -> void {
block ^entry:
eh_cleanup ^cleanup
call void @leaf()
eh_end
return void
block ^cleanup:
call void @append(3)
resume
}'''))
add('normal-pop',main('''eh_try ^outer
eh_try ^inner
eh_end
throw i64 7
block ^inner:
return i64 90
block ^outer:
%x = exception i64
%b = cmp ne i64 %x, 7
return i64 %b'''))
add('return-active-region',main('''eh_try ^caught
%ignored = call i64 @early()
throw i64 8
block ^caught:
%x = exception i64
%b = cmp ne i64 %x, 8
return i64 %b''','''function @early() -> i64 {
block ^entry:
eh_try ^stale
return i64 0
block ^stale:
return i64 91
}'''))
add('pop-loop-bounded-stack',main('''%initial = stack_alloc 0
jump ^loop
block ^loop:
%n = phi i64 [^entry: 0, ^next: %inc]
eh_try ^caught
eh_end
%now = stack_alloc 0
%bad = cmp ne ptr %initial, %now
branch %bad, ^fail, ^next
block ^next:
%inc = binary add i64 %n, 1
%more = cmp lt i64 %inc, 10000
branch %more, ^loop, ^done
block ^done:
return i64 0
block ^fail:
return i64 1
block ^caught:
return i64 2'''))
add('allocation-lives-after-pop',main('''eh_try ^caught
%p = stack_alloc 33
store i64 42, %p
eh_end
%q = stack_alloc 64
zeroinit 64x1 %q
%v = load i64 %p
%b0 = cmp ne i64 %v, 42
%bits = copy i64 %p
%low = binary and i64 %bits, 15
%b1 = cmp ne i64 %low, 0
%b = binary or i64 %b0, %b1
return i64 %b
block ^caught:
return i64 2'''))
add('allocation-across-unwind',main('''%p = stack_alloc 37
store i64 42, %p
eh_try ^caught
%q = stack_alloc 128
zeroinit 128x8 %q
call void @deep()
eh_end
return i64 90
block ^caught:
%v = load i64 %p
%b = cmp ne i64 %v, 42
return i64 %b''','''function @deep() -> void {
block ^entry:
%p = stack_alloc 4096
zeroinit 4096x16 %p
throw i64 5
}'''))
add('allocation-normal-call',main('''%p = stack_alloc 48
store i64 37, %p
%v = call i64 @work(%p, 97)
%b = cmp ne i64 %v, 42
return i64 %b''','''function @work(%p : ptr, %n : i64) -> i64 {
block ^entry:
%temp = stack_alloc %n
store i64 5, %temp
%x = load i64 %p
%y = load i64 %temp
%r = binary add i64 %x, %y
return i64 %r
}'''))
add('allocation-lives-after-catch',main('''eh_try ^caught
%p = stack_alloc 64
store i64 42, %p
throw i64 9
block ^caught:
%q = stack_alloc 128
zeroinit 128x8 %q
%v = load i64 %p
%b = cmp ne i64 %v, 42
return i64 %b'''))
for alignment in [32,64,128,256,4096]:
    for unwind in [False,True]:
        transition='eh_try ^caught\ncall void @raise()\neh_end\nreturn i64 90\nblock ^caught:' if unwind else 'call void @nothing()'
        add(f'aligned-{alignment}-{unwind}',main(f'''%p = addr $object
%address = copy i64 %p
%bits = binary and i64 %address, {alignment-1}
%bad0 = cmp ne i64 %bits, 0
store f80 3.5L, %p
{transition}
%v = load f80 %p
%bad1 = cmp ne f80 %v, 3.5L
%bad = binary or i64 %bad0, %bad1
return i64 %bad''','''function @raise() -> void {
block ^entry:
throw i64 42
}
function @nothing() -> void {
slot $other : obj<4096x4096>
block ^entry:
%p = addr $other
zeroinit 4096x4096 %p
return void
}''',f'slot $object : obj<{alignment}x{alignment}>'))
# Handler uses six parameters after every incoming carrier has been overwritten.
for alignment in [32,64,256]:
    typ=f'obj<{alignment}x{alignment}>'
    for gp in [0,1,5,6,8]:
        params=', '.join([*(f'%p{k} : i64' for k in range(gp)),f'%object : {typ}','%tail : i64'])
        args=', '.join([*(str(k+1) for k in range(gp)),'$object','37'])
        helper=f'''function @identity({params}) -> {typ} {{
block ^entry:
return {typ} %object
}}'''
        add(f'aligned-abi-{alignment}-{gp}',main(f'''%base = addr $object
zeroinit {alignment}x{alignment} %base
store i64 42, %base
%result = call {typ} @identity({args})
%out = addr $out
copyobj {alignment}x{alignment} %result, %out
%v = load i64 %out
%bad = cmp ne i64 %v, 42
return i64 %bad''',helper,f'slot $object : {typ}\nslot $out : {typ}'))

params=', '.join(f'%p{k} : i64' for k in range(6))
body='eh_try ^caught\ncall void @clobber(90,91,92,93,94,95)\nthrow i64 42\nblock ^caught:\n'
for k in range(6): body+=f'%b{k} = cmp ne i64 %p{k}, {k+1}\n'
acc='%b0'
for k in range(1,6): body+=f'%s{k} = binary or i64 {acc}, %b{k}\n'; acc=f'%s{k}'
body+=f'return i64 {acc}'
helpers=f'function @clobber({params}) -> void {{\nblock ^entry:\nreturn void\n}}\nfunction @pressure({params}) -> i64 {{\nblock ^entry:\n{body}\n}}'
add('handler-incoming-homes',main('%r = call i64 @pressure(1,2,3,4,5,6)\nreturn i64 %r',helpers))
add('unhandled',main('throw i64 9'),1)
add('unhandled-resume',main('resume'),1)
results=[]
for name,(source,status) in cases.items():
    path=out/(name+'.lowir'); path.write_text(source); exe=out/name; mir=out/(name+'.mir')
    p=subprocess.run([str(cc),'--dump-machine-ir',str(mir),'-o',str(exe),str(path)],capture_output=True,text=True)
    row={'case':name,'compile':p.returncode,'expected':status}
    if p.returncode: row['error']=p.stderr
    else:
        try:
            r=subprocess.run([str(exe)],capture_output=True,timeout=10); row['run']=r.returncode
            row['pass']=r.returncode==status and not r.stdout
        except subprocess.TimeoutExpired: row['error']='timeout'
    results.append(row)
    if not row.get('pass'): print(json.dumps(row))
(out/'results.json').write_text(json.dumps(results,indent=2)+'\n')
print(f'{sum(bool(r.get("pass")) for r in results)}/{len(results)} runtime/frame cases passed; {out}')
sys.exit(not all(r.get('pass') for r in results))
