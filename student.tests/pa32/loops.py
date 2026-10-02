#!/usr/bin/env python3
"""Executed affine-loop reducers, four levels and three object/execution paths."""
import json,pathlib,random,subprocess,tempfile
ROOT=pathlib.Path(__file__).resolve().parents[2]
def run(*args):
    r=subprocess.run(list(map(str,args)),capture_output=True,text=True,timeout=60)
    assert r.returncode==0,(args,r.returncode,r.stdout[-2000:],r.stderr[-4000:])
    return r
functions=[];checks=[]
def case(body,expected,params='',args='',slots='',result='i64',extra=''):
    name='loop_case'+str(len(checks))
    functions.append(f'function @{name}({params}) -> {result} [no_inline=yes] {{\n{slots}\n{body}\n}}\n'+extra)
    checks.append((name,args,expected,result))
# Every signed/unsigned predicate, either comparison order, either branch arm,
# 8/16/32/64-bit inductions, zero/one/four/longer trip counts and negative steps.
rng=random.Random(212)
for ty,width,signed in [('i8',8,True),('u8',8,False),('i16',16,True),('u16',16,False),('i32',32,True),('u32',32,False),('i64',64,True)]:
    for descending in [False,True]:
        for trips in [0,1,4,7]:
            start=20 if descending else 2
            delta=-2 if descending else 2
            limit=start+trips*delta
            pred=('gt' if descending else 'lt') if signed else ('ugt' if descending else 'ult')
            swap=rng.randrange(2);invert=rng.randrange(2)
            if swap:pred={'gt':'lt','lt':'gt','ugt':'ult','ult':'ugt'}[pred]
            if invert:pred={'gt':'le','lt':'ge','ugt':'ule','ult':'uge'}[pred]
            left,right=('%i',str(limit)) if not swap else (str(limit),'%i')
            # Escaping accumulator prevents effect-free induction-only deletion.
            body=f'''block ^entry: jump ^head
block ^head:
 %i = phi {ty} [^entry: {start}, ^body: %next]
 %acc = phi i64 [^entry: %seed, ^body: %sum]
 %more = cmp {pred} {ty} {left}, {right}
 branch %more, {'^exit, ^body' if invert else '^body, ^exit'}
block ^body:
 %wide = copy i64 %i
 %sum = binary add i64 %acc, %wide
 %next = binary {'sub' if descending else 'add'} {ty} %i, 2
 jump ^head
block ^exit: return i64 %acc'''
            case(body,11+sum(start+k*delta for k in range(trips)),'%seed : i64','11')
# Non-strict, equality, inequality, endpoints close to overflow.
for ty,start,limit,delta,pred,trips in [('i8',124,126,1,'le',3),('u8',252,254,1,'ule',3),('i8',-124,-127,-1,'gt',3),('i16',9,1,-2,'ne',4),('i32',7,7,1,'eq',1),('i64',8,7,0,'lt',0)]:
    case(f'''block ^entry: jump ^head
block ^head:
 %i = phi {ty} [^entry: {start}, ^body: %next]
 %more = cmp {pred} {ty} %i, {limit}
 branch %more, ^body, ^exit
block ^body:
 %next = binary add {ty} %i, {delta}
 jump ^head
block ^exit:
 %r = copy i64 %i
 return i64 %r''',start+trips*delta)
# Cross-phi parallel edges and body values exported separately.
for trips in [0,1,2,3,4]:
    case(f'''block ^entry: jump ^head
block ^head:
 %i = phi i64 [^entry: 0, ^body: %next]
 %a = phi i64 [^entry: %x, ^body: %b]
 %b = phi i64 [^entry: %y, ^body: %a]
 %more = cmp lt i64 %i, {trips}
 branch %more, ^body, ^exit
block ^body:
 %next = binary add i64 %i, 1
 jump ^head
block ^exit:
 %t = binary mul i64 %a, 100
 %r = binary add i64 %t, %b
 return i64 %r''',307 if trips%2==0 else 703,'%x : i64, %y : i64','3, 7')
# Loads/stores, aliases, volatile accesses, and calls retain per-iteration order.
functions.append('global @observed : i64 = 0')
functions.append('''function @touch(%n : i64) -> void [no_inline=yes] {
block ^entry: %x = load volatile i64 @observed
 %y = binary mul i64 %x, 10
 %z = binary add i64 %y, %n
 store volatile i64 %z, @observed
 return void }''')
for trips in [0,1,4,5]:
    case(f'''block ^entry:
 store volatile i64 0, @observed
 jump ^head
block ^head:
 %i = phi i64 [^entry: 1, ^body: %next]
 %more = cmp le i64 %i, {trips}
 branch %more, ^body, ^exit
block ^body:
 call void @touch(%i)
 %next = binary add i64 %i, 1
 jump ^head
block ^exit:
 %r = load volatile i64 @observed
 return i64 %r''',int(''.join(str(i) for i in range(1,trips+1)) or '0'))
# Floating-point state retains rounding and evaluation counts.
case('''block ^entry: jump ^head
block ^head:
 %i = phi i64 [^entry: 0, ^body: %next]
 %a = phi f64 [^entry: 1.0, ^body: %sum]
 %more = cmp lt i64 %i, 4
 branch %more, ^body, ^exit
block ^body:
 %sum = binary mul f64 %a, 1.5
 %next = binary add i64 %i, 1
 jump ^head
block ^exit:
 %ok = cmp eq f64 %a, 5.0625
 return i64 %ok''',1)
# Exit phi retains its edge identity when another path reaches the same join.
for take in [0,1]:
    case('''block ^entry: branch %take, ^pre, ^other
block ^pre: jump ^head
block ^head:
 %i = phi i64 [^pre: 0, ^body: %next]
 %more = cmp lt i64 %i, 4
 branch %more, ^body, ^exit
block ^body:
 %next = binary add i64 %i, 1
 jump ^head
block ^other: jump ^exit
block ^exit:
 %r = phi i64 [^head: %i, ^other: 19]
 return i64 %r''',4 if take else 19,'%take : i64',str(take))
# A source-later incoming definition is valid in a phi, but not in an ordinary copy.
case('''block ^entry: jump ^pre
block ^head:
 %i = phi i64 [^pre: 0, ^body: %next]
 %a = phi i64 [^pre: %seed, ^body: %sum]
 %more = cmp lt i64 %i, 4
 branch %more, ^body, ^exit
block ^body:
 %sum = binary add i64 %a, 1
 %next = binary add i64 %i, 1
 jump ^head
block ^pre:
 %seed = load volatile i64 @observed
 jump ^head
block ^exit: return i64 %a''',12349)
# Explicit long affine loop: analytic deletion must not simulate its trip count.
case('''block ^entry: jump ^head
block ^head:
 %i = phi i64 [^entry: 0, ^body: %next]
 %more = cmp lt i64 %i, 100000
 branch %more, ^body, ^exit
block ^body:
 %next = binary add i64 %i, 1
 jump ^head
block ^exit: return i64 %i''',100000)
# A dynamically terminating wraparound loop must remain (overflow-free proof fails).
case('''block ^entry: jump ^head
block ^head:
 %i = phi u8 [^entry: 254, ^body: %next]
 %more = cmp ne u8 %i, 1
 branch %more, ^body, ^exit
block ^body:
 %next = binary add u8 %i, 1
 jump ^head
block ^exit:
 %r = copy i64 %i
 return i64 %r''',1)
# A mutable induction carrier is not an SSA fact.
case('''block ^entry: jump ^head
block ^head:
 %i = phi i64 [^entry: 0, ^body: %next]
 %more = cmp lt i64 %i, 4
 branch %more, ^body, ^exit
block ^body:
 %i = copy i64 3
 %next = binary add i64 %i, 1
 jump ^head
block ^exit: return i64 %i''',4)
# Guards: nonterminating parity/overflow, effectful loop and exceptional state.
guards={
'parity':'''function @guard(%start : i64) -> i64 {block ^entry: jump ^head
block ^head: %i = phi i64 [^entry: %start, ^body: %next]
%c = cmp ne i64 %i, 7
branch %c, ^body, ^exit
block ^body: %next = binary add i64 %i, 2
jump ^head
block ^exit: return i64 %i}''',
'overflow':'''function @guard() -> i64 {block ^entry: jump ^head
block ^head: %i = phi u8 [^entry: 254, ^body: %next]
%c = cmp ule u8 %i, 255
branch %c, ^body, ^exit
block ^body: %next = binary add u8 %i, 1
jump ^head
block ^exit: return i64 0}'''}
with tempfile.TemporaryDirectory(prefix='pa32-loops-') as temp:
    d=pathlib.Path(temp);source=d/'cases.lowir'
    main=['function @main() -> i32 [role=entry] {block ^entry:']
    for n,(name,args,expected,ty) in enumerate(checks):
        main += [f'%v{n} = call {ty} @{name}({args})',f'%c{n} = cmp eq {ty} %v{n}, {expected}']
        if n: main.append(f'%ok{n} = binary and i64 '+('%c0' if n==1 else f'%ok{n-1}')+f', %c{n}')
    main += [f'%bad = cmp eq i64 %ok{len(checks)-1}, 0','%r = convert trunc i32 i64 %bad','return i32 %r','}']
    source.write_text('\n'.join(functions+main))
    for level in range(4):
        ir=d/f'O{level}.lowir'
        run(ROOT/'dev/lowiropt',f'-O{level}','-o',ir,source)
        for path in ['original','replay','native']:
            exe=d/f'{path}{level}'
            if path=='native':
                run(ROOT/'dev/lowir2native','-o',exe,ir)
            else:
                obj=d/'cases.o';run(ROOT/'dev/cppgm++','-c',f'-O{level}' if path=='original' else '-O0','-o',obj,source if path=='original' else ir)
                run('g++',obj,'-o',exe)
            run(exe)
        print(f'{len(checks)} executed loop cases O{level}: three paths PASS',flush=True)
    for name,body in guards.items():
        inp=d/(name+'.lowir');inp.write_text(body)
        for level in range(1,4):
            out=d/'guard.lowir';run(ROOT/'dev/lowiropt',f'-O{level}','-o',out,inp)
            assert 'phi ' in out.read_text(),(name,level,out.read_text())
    print('unproven termination/overflow guard preservation PASS',flush=True)
