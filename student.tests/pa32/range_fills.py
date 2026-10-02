#!/usr/bin/env python3
"""Dynamic range fills: aliasing, zero trips, snapshots, exports and replay."""
import pathlib,subprocess,tempfile
ROOT=pathlib.Path(__file__).resolve().parents[2]
def run(*args):
    p=subprocess.run(list(map(str,args)),capture_output=True,text=True,timeout=60)
    assert p.returncode==0,(args,p.returncode,p.stdout[-2000:],p.stderr[-4000:])
    return p

def fill(name,width=1,reference=False,twin=False,volatile=False,mutable=False,pattern=None,step=None):
    ty={1:'u8',2:'u16',4:'u32',8:'i64'}[width]
    if pattern is None:pattern=sum(0xab<<(8*k) for k in range(width))
    if width==8 and pattern>=1<<63:pattern-=1<<64
    return f'''function @{name}(%first : ptr, %n : i64, %value : ptr) -> ptr [no_inline=yes] {{
block ^entry:
 %bytes = binary mul i64 %n, {width}
 %last = index i8 %first, %bytes
 {'%bytes = copy i64 0' if mutable else ''}
 jump ^head
block ^head:
 %p = phi ptr [^entry: %first, ^body: %next]
 {'%twin = phi ptr [^entry: %first, ^body: %next]' if twin else ''}
 %more = cmp ne ptr %p, %last
 branch %more, ^body, ^done
block ^body:
 {f'%v = load {"volatile " if volatile else ""}{ty} %value' if reference else ''}
 store {'volatile ' if volatile else ''}{ty} {'%v' if reference else pattern}, %p !dbg(range.cpp, 20, 3)
 %next = index i8 %p, {width if step is None else step}
 jump ^head
block ^done:
 return ptr {'%twin' if twin else '%p'}
}}
'''
with tempfile.TemporaryDirectory(prefix='pa32-fill-') as temp:
    d=pathlib.Path(temp)
    functions=[fill('ref',reference=True,twin=True),fill('zero',width=4,pattern=0),fill('mutable',mutable=True),fill('volatile',reference=True,volatile=True)]
    functions += [fill('pattern'+str(w),width=w,twin=True) for w in [1,2,4,8]]
    functions += [fill('nonrepeat',width=4,pattern=258),fill('wordref',width=4,reference=True)]
    joined=fill('exit_phi').replace('jump ^head\nblock ^head:', 'branch %take, ^pre, ^other\nblock ^pre: jump ^head\nblock ^head:',1).replace('[^entry: %first,','[^pre: %first,').replace('%value : ptr)', '%value : ptr, %take : i64)').replace('block ^done:\n return ptr %p', 'block ^other: jump ^done\nblock ^done:\n %result = phi ptr [^head: %p, ^other: %first]\n return ptr %result')
    functions += [joined,joined.replace('@exit_phi(', '@exit_phi_skip(')]
    functions += [fill('runtime_byte',pattern='%value').replace('%value : ptr)', '%value : i64)')]
    slot=fill('slot_reference',reference=True).replace('block ^entry:', 'slot $byte : u8\nblock ^entry: %address = addr $byte\ncall void @set_byte(%address)',1).replace('load u8 %value','load u8 $byte')
    functions += [slot, 'function @set_byte(%p : ptr) -> void [no_inline=yes] {block ^entry: store u8 141, %p return void}']
    main=['function @main() -> i32 [role=entry] {slot $buf : obj<80x8>','block ^entry:','%buf = addr $buf']
    checks=[]
    def check(expr):
        n=len(checks);main.append(f'%check{n} = {expr}');checks.append('%check'+str(n))
    def invoke(name,n,source,expected,bytes):
        main.append('zeroinit 80x8 $buf')
        if source=='alias':
            main.append('%ref'+str(len(checks))+' = index i8 %buf, 3')
            source='%ref'+str(len(checks));main.append(f'store u8 {expected}, {source}')
        tag=len(checks);extra=', 0' if name=='exit_phi_skip' else ', 1' if name=='exit_phi' else ''
        main.append(f'%r{tag} = call ptr @{name}(%buf, {n}, {source}{extra})')
        main.append(f'%end{tag} = index i8 %buf, {bytes}')
        check(f'cmp eq ptr %r{tag}, %end{tag}')
        for k in range(bytes):
            tag=len(checks);main.extend([f'%a{tag} = index i8 %buf, {k}',f'%v{tag} = load u8 %a{tag}'])
            check(f'cmp eq u8 %v{tag}, {(expected>>(8*(k%4)))&255 if name=="nonrepeat" else expected}')
        tag=len(checks);main.extend([f'%a{tag} = index i8 %buf, {bytes}',f'%v{tag} = load u8 %a{tag}'])
        # Alias preparation at offset three remains outside an empty range.
        check(f'cmp eq u8 %v{tag}, {expected if source.startswith("%ref") and bytes==3 else 0}')
    for n in [0,1,3,7]:
        for w in [1,2,4,8]:invoke('pattern'+str(w),n,'nullptr',171,n*w)
        invoke('zero',n,'nullptr',0,n*4)
        invoke('mutable',n,'nullptr',171,n)
        invoke('nonrepeat',n,'nullptr',258,n*4)
    for n in [1,4,7]:
        for value in [0,128,255]:invoke('ref',n,'alias',value,n)
    invoke('exit_phi',7,'nullptr',171,7)
    invoke('exit_phi_skip',7,'nullptr',0,0)
    invoke('slot_reference',7,'nullptr',141,7)
    for value in [-1,0,128,256,511]:invoke('runtime_byte',7,str(value),value&255,7)
    main.append('%null = call ptr @ref(nullptr, 0, nullptr)')
    check('cmp eq ptr %null, nullptr')
    for n,c in enumerate(checks[1:],1):main.append(f'%ok{n} = binary and i64 '+(checks[0] if n==1 else f'%ok{n-1}')+f', {c}')
    main += [f'%bad = cmp eq i64 %ok{len(checks)-1}, 0','%result = convert trunc i32 i64 %bad','return i32 %result','}']
    src=d/'fills.lowir';src.write_text(''.join(functions)+'\n'.join(main))
    for level in range(4):
        opt=d/f'O{level}.lowir';run(ROOT/'dev/lowiropt',f'-O{level}','-o',opt,src)
        if level:
            text=opt.read_text()
            for name in ['ref','zero','exit_phi','exit_phi_skip','runtime_byte','slot_reference']+['pattern'+str(w) for w in [1,2,4,8]]:
                body=text.split('function @'+name+'(')[1].split('\n}')[0]
                assert body.count('phi ')==(1 if name.startswith('exit_phi') else 0) and 'call void @opt_fill_' in body,(name,body)
                assert '!dbg(range.cpp, 20, 3)' in body,body
            for name in ['mutable','volatile','nonrepeat','wordref']:
                body=text.split('function @'+name+'(')[1].split('\n}')[0]
                assert 'phi ' in body,(name,body)
        for path in ['original','replay','native']:
            exe=d/f'{path}{level}'
            if path=='native':run(ROOT/'dev/lowir2native','-o',exe,opt)
            else:
                obj=d/f'{path}.o';run(ROOT/'dev/cppgm++','-c',f'-O{level}' if path=='original' else '-O0','-o',obj,src if path=='original' else opt)
                run('g++',obj,'-o',exe)
            run(exe)
        # Serialize before optimization, then run the same level, including
        # helper identities, line locations and direct ELF emission.
        replay=d/'replay.o';run(ROOT/'dev/cppgm++','-c',f'-O{level}','-o',replay,d/'O0.lowir')
        assert replay.read_bytes()==(d/'original.o').read_bytes(),level
        print(f'range fills O{level}: {len(checks)} checked values x three paths; replay/debug PASS',flush=True)
