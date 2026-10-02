#!/usr/bin/env python3
"""Explicit checkpoint-210 reducers for typed facts and ABI carriers.

LowIR permits integer operands converted to an instruction's width. A proof
about that converted value is not a proof about discarded bits. Switch and
variadic/by-address calls instead consume the operand's own type. Exercise
all three replacement owners and both native paths, with independent expected
results. No reference compiler supplies these answers.
"""
import os
import pathlib
import subprocess
import tempfile

ROOT = pathlib.Path(__file__).resolve().parents[2]
OPT = pathlib.Path(os.environ.get('LOWIROPT', ROOT / 'dev/lowiropt'))
functions, calls = [], []


def run(*args):
    result = subprocess.run(list(map(str, args)), capture_output=True, text=True, timeout=60)
    assert result.returncode == 0, (args, result.returncode, result.stderr[-4000:])
    return result


def case(name, body, expected, parameters='', actuals='', metadata='no_inline=yes'):
    functions.append(f'function @{name}({parameters}) -> i64 [{metadata}] {{\n{body}\n}}')
    calls.append((name, actuals, expected))


for ty, width in [('i8', 8), ('u8', 8), ('i16', 16), ('u16', 16), ('i32', 32), ('u32', 32)]:
    for x in [0, 1, 1 << width, (1 << width) + 1, -(1 << width)]:
        suffix = f'{ty}_{len(calls)}'
        low_zero = x % (1 << width) == 0
        case('edge_' + suffix, f'''block ^entry:
 %c = cmp eq {ty} %x, 0
 branch %c, ^yes, ^no
block ^yes: branch %x, ^nonzero, ^zero
block ^nonzero: return i64 22
block ^zero: return i64 11
block ^no: return i64 33''', (22 if x else 11) if low_zero else 33, '%x : i64', str(x))
        case('reverse_' + suffix, f'''block ^entry: branch %x, ^yes, ^no
block ^yes:
 %c = cmp ne {ty} %x, 0
 branch %c, ^nonzero, ^zero
block ^nonzero: return i64 22
block ^zero: return i64 11
block ^no: return i64 33''', (11 if low_zero else 22) if x else 33, '%x : i64', str(x))
        case('diamond_' + suffix, f'''block ^entry:
 %c = cmp eq {ty} %x, 0
 branch %c, ^yes, ^no
block ^yes: jump ^end
block ^no: jump ^end
block ^end:
 %r = phi i64 [^yes: 0, ^no: %x]
 return i64 %r''', 0 if low_zero else x, '%x : i64', str(x))

for ty in ['u8', 'u16', 'u32', 'i128']:
    wide = ty == 'i128'
    number = 1 << 64 if wide else (1 << int(ty[1:])) - 1
    arms = f'0:^wrong, {number}:^yes' if wide else '-1:^yes'
    tail = '''block ^wrong: return i64 11
block ^yes: return i64 22
block ^no: return i64 33'''
    case('switch_' + ty, f'''block ^entry:
 %x = const {ty} {number}
 switch %x, ^no, {arms}
{tail}''', 22)
    # Closed-world call propagation at O2 must retain the same typed selector.
    case('call_switch_' + ty, f'''block ^entry:
 switch %x, ^no, {arms}
{tail}''', 22, f'%x : {ty}', str(number), 'binding=internal, no_inline=yes')
    # Edge equality publishes an exact fact, then visits the switch consumer.
    case('edge_switch_' + ty, f'''block ^entry:
 %c = cmp eq {ty} %x, {number}
 branch %c, ^switch, ^no
block ^switch: switch %x, ^no, {arms}
{tail}''', 22, f'%x : {ty}', str(number))

for source_type, target_type, actual, rounded in [
        ('f64', 'f32', '16777217.0', '16777216.0'),
        ('f80', 'f32', '16777217.0L', '16777216.0'),
        ('f32', 'f64', '1.5', '1.5'),
        ('f80', 'f64', '1.5L', '1.5')]:
    for condition in [0, 1]:
        # The source value changes after the store. Promotion must preserve
        # both the original snapshot and the floating format conversion.
        name = f'float_{source_type}_{target_type}_{condition}'
        functions.append(f'''function @{name}(%x : {source_type}, %c : i64) -> i64 [no_inline=yes] {{
 slot $s : {target_type}
 block ^entry:
  store {target_type} %x, $s
  %x = copy {source_type} 9.0
  branch %c, ^a, ^b
 block ^a:
  %y = load {target_type} $s
  %a = cmp eq {target_type} %y, {rounded}
  return i64 %a
 block ^b:
  %z = load {target_type} $s
  %b = cmp eq {target_type} %z, {rounded}
  return i64 %b
}}''')
        calls.append((name, f'{actual}, {condition}', 1))

functions.append('''function @take(%tag : i64) -> i64 [arity=variadic, no_inline=yes] {
slot $list : obj<24x8>
block ^entry:
 %p = addr $list
 va_start %p
 %v = va_arg i128 %p
 %high = binary ushr i128 %v, 64
 %result = copy i64 %high
 return i64 %result
}''')
functions.append('''function @indirect_value(%p : ptr [pass=by_address, object_bytes=16]) -> i64 [no_inline=yes] {
block ^entry:
 %v = load i128 %p
 %high = binary ushr i128 %v, 64
 %result = copy i64 %high
 return i64 %result
}''')
number = 7 << 64
for owner in ['scalar', 'call', 'edge']:
    for boundary in ['variadic', 'address', 'indirect']:
        prefix = f'block ^entry: %x = const i128 {number}'
        params, args, metadata = '', '', 'no_inline=yes'
        if owner != 'scalar':
            params, args = '%x : i128', str(number)
            prefix = 'block ^entry:'
            if owner == 'call':
                metadata += ', binding=internal'
            else:
                prefix += f''' %c = cmp eq i128 %x, {number}
 branch %c, ^yes, ^no
block ^no: return i64 0
block ^yes: '''
        call = ('call i64 @take(0, %x, 0)' if boundary == 'variadic' else
                'call i64 @indirect_value(%x)' if boundary == 'address' else
                'call i64 %callee(0, %x, 0) as (i64) -> i64 [arity=variadic]')
        if boundary == 'indirect':
            prefix += '\n %callee = addr @take'
        case(owner + '_' + boundary, prefix + f'\n %r = {call}\n return i64 %r',
             7, params, args, metadata)

main = ['function @main() -> i64 [role=entry] {', 'block ^entry:', '%bad0 = const i64 0']
for n, (name, args, expected) in enumerate(calls):
    main += [f'%r{n} = call i64 @{name}({args})', f'%c{n} = cmp ne i64 %r{n}, {expected}',
             f'%bad{n+1} = binary or i64 %bad{n}, %c{n}']
main += [f'return i64 %bad{len(calls)}', '}']
with tempfile.TemporaryDirectory(prefix='pa32-audit-') as directory:
    tmp = pathlib.Path(directory)
    source = tmp / 'input.lowir'
    source.write_text('\n'.join(functions + main))
    for level in range(4):
        optimized = tmp / f'o{level}.lowir'
        run(OPT, f'-O{level}', '-o', optimized, source)
        run(ROOT / 'dev/lowir', '-o', tmp / 'validated.lowir', optimized)
        run(ROOT / 'dev/lowir2native', '-o', tmp / 'native', optimized)
        run(tmp / 'native')
        run(ROOT / 'dev/cppgm++', '-O0', '-o', tmp / 'object-exe', optimized)
        run(tmp / 'object-exe')
print(f'PA32 audit: PASS ({len(calls)} execution cases x 4 levels x 2 native paths)')
