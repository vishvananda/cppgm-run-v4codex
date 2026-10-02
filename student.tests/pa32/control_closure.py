#!/usr/bin/env python3
"""Partial slot facts and eliminated diamonds, including unresolved-path guards."""
import pathlib,subprocess,tempfile
ROOT=pathlib.Path(__file__).resolve().parents[2]
def run(*args):
    p=subprocess.run(list(map(str,args)),capture_output=True,text=True,timeout=60)
    assert p.returncode==0,(args,p.returncode,p.stdout,p.stderr)
    return p
with tempfile.TemporaryDirectory(prefix='pa32-control-') as temp:
    d=pathlib.Path(temp)
    original=(ROOT/'pa32/tests/o1/500-phi-integrity-survivors.t').read_text()
    original=original.replace('declare function @observe(%value : i64) -> void','''function @observe(%value : i64) -> void [no_inline=yes] {
block ^entry: return void
}''')
    # A mixed unknown/known reaching fact must retain stores, unlike a missing
    # entry fact whose only readers precede every definition.
    guard='''function @mixed(%take : i64) -> i64 [no_inline=yes] {
slot $s : i64
block ^entry: branch %take, ^left, ^right
block ^left: store i64 17, $s
jump ^join
block ^right: jump ^join
block ^join: %x = load i64 $s
return i64 %x
}
'''
    # An unresolved entry read must not taint stores and phis feeding a later
    # loop, nor permit forwarding mutable operands without store snapshots.
    loop='''function @partial_loop(%seed : i64) -> i64 [no_inline=yes] {
slot $s : i64
block ^entry: %u = load i64 $s
call void @observe(%u)
store i64 %seed, $s
%seed = copy i64 99
jump ^head
block ^head: %i = phi i64 [^entry: 0, ^body: %next]
%v = load i64 $s
%c = cmp lt i64 %i, 4
branch %c, ^body, ^done
block ^body: %v1 = binary add i64 %v, 3
store i64 %v1, $s
%next = binary add i64 %i, 1
jump ^head
block ^done: return i64 %v
}
'''
    cases=[]
    for arg in [0,1,2,-1,256,-256]:
        cases += [('repair_fold_target_phi',arg,11 if arg else 22),('cross_slot_needed_phi',arg,5 if arg else 9),('partial_loop',arg,arg+12)]
    cases += [('mixed',1,17)]
    main=['function @main() -> i32 [role=entry] {block ^entry:']
    for n,(name,arg,result) in enumerate(cases):
        main += [f'%v{n} = call i64 @{name}({arg})',f'%c{n} = cmp eq i64 %v{n}, {result}']
        if n:main += [f'%ok{n} = binary and i64 '+('%c0' if n==1 else f'%ok{n-1}')+f', %c{n}']
    main += [f'%bad = cmp eq i64 %ok{len(cases)-1}, 0','%r = convert trunc i32 i64 %bad','return i32 %r','}']
    src=d/'cases.lowir';src.write_text(original+guard+loop+'\n'.join(main))
    for level in range(4):
        opt=d/'optimized.lowir';run(ROOT/'dev/lowiropt',f'-O{level}','-o',opt,src)
        if level:
            text=opt.read_text()
            mixed=text.split('function @mixed(')[1].split('\n}')[0]
            assert 'store i64 17' in mixed and 'load i64' in mixed,mixed
            partial=text.split('function @cross_slot_needed_phi(')[1].split('\n}')[0]
            assert 'store ' not in partial and partial.count('load ')==1,partial
        for path in ['original','replay','native']:
            exe=d/f'{path}{level}'
            if path=='native':run(ROOT/'dev/lowir2native','-o',exe,opt)
            else:
                obj=d/'cases.o';run(ROOT/'dev/cppgm++','-c',f'-O{level}' if path=='original' else '-O0','-o',obj,src if path=='original' else opt)
                run('g++',obj,'-o',exe)
            run(exe)
    print(f'control closure: {len(cases)} cases x four levels x three paths; mixed-fact retention PASS')
