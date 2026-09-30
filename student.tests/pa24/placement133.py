#!/usr/bin/env python3
"""Independent numeric checks for call placement, permutations and Boolean facts."""
import hashlib, itertools, json, pathlib, subprocess, sys

root = pathlib.Path(__file__).resolve().parents[2]
cc = pathlib.Path(sys.argv[1]).resolve()
out = pathlib.Path(sys.argv[2]).resolve()
out.mkdir(parents=True, exist_ok=True)
cases = {}

def main(body, helpers=''):
    return helpers + '\nfunction @main() -> i64 [role=entry] { block ^entry:\n' + body + '\n}\n'

# Weighted sums distinguish every position, including register cycles, stack
# overflow and mixed XMM/GPR placement. Targets travel through ordinary pointer
# parameters in every possible position, including on the stack.
for mixed, count in itertools.product((False, True), (2, 5, 7, 10)):
    types = [('f64' if mixed and i % 2 else 'i64') for i in range(count)]
    decl = ', '.join(f'%a{i} : {t}' for i, t in enumerate(types))
    terms = []
    for i, t in enumerate(types):
        if t == 'f64': terms.append(f'%c{i} = convert fptosi i64 f64 %a{i}')
        terms.append(f'%t{i} = binary mul i64 '+(f'%c{i}' if t == 'f64' else f'%a{i}')+f', {i+1}')
    prior = '%t0'
    for i in range(1, count):
        terms.append(f'%s{i} = binary add i64 {prior}, %t{i}'); prior = f'%s{i}'
    helper = f'function @weighted({decl}) -> i64 {{ block ^entry:\n'+'\n'.join(terms)+f'\nreturn i64 {prior}\n}}\n'
    for shift in range(count):
        order = [(i+shift) % count for i in range(count)]
        # Forwarders have integer inputs; conversion to the callee's floating
        # arguments exercises explicit numeric facts before parallel setup.
        setup = [f'%f{i} = convert sitofp f64 i64 %v{order[i]}' for i, t in enumerate(types) if t == 'f64']
        args = ', '.join(f'%f{i}' if t == 'f64' else f'%v{order[i]}' for i, t in enumerate(types))
        expected = sum((i+1)*(order[i]+3) for i in range(count))
        for target_position in (0, count//2, count):
            params = [f'%v{i} : i64' for i in range(count)]
            actual = [str(i+3) for i in range(count)]
            params.insert(target_position, '%target : ptr'); actual.insert(target_position, '@weighted')
            forward = 'function @forward('+', '.join(params)+') -> i64 { block ^entry:\n'+'\n'.join(setup)
            forward += f'\n%r = call i64 %target({args}) as ({decl}) -> i64\nreturn i64 %r\n}}\n'
            cases[f'permutation-{mixed}-{count}-{shift}-{target_position}'] = main(
                '%r = call i64 @forward('+', '.join(actual)+f')\n%bad = cmp ne i64 %r, {expected}\nreturn i64 %bad', helper+forward)

# A source comparison's 0/1 range survives every scalar integer conversion;
# wide destinations still require two-word storage. Test both outcomes and a
# call/CFG interval in addition to adjacent returns.
for mode, truth, lifetime in itertools.product(('i8', 'u8', 'i16', 'u16', 'i32', 'u32', 'sext64', 'zext64', 'i128'), (0, 1), ('return', 'call', 'edge')):
    typ = 'i64' if mode.endswith('64') else mode
    op = 'zext' if typ == 'i128' else 'trunc'
    body = f'%b = cmp eq i64 %x, {truth}\n%c = convert {op} {typ} i64 %b\n'
    if mode.endswith('64'):
        body = f'%b = cmp eq i64 %x, {truth}\n%small = convert trunc i32 i64 %b\n%c = convert {mode[:-2]} i64 i32 %small\n'
    if lifetime == 'call': body += '%unused = call i64 @clobber(97, 42, 13, 3, 7, 9, 11)\n'
    if lifetime == 'edge': body += 'jump ^done\nblock ^done:\n'
    body += f'return {typ} %c'
    helper = f'function @convert(%x : i64) -> {typ} {{ block ^entry:\n{body}\n}}\n'
    helper += 'function @clobber(%a:i64,%b:i64,%c:i64,%d:i64,%e:i64,%f:i64,%g:i64) -> i64 { block ^entry: %r = binary div i64 %a, %d return i64 %r }\n'
    cases[f'boolean-{mode}-{truth}-{lifetime}'] = main(f'%r = call {typ} @convert(1)\n%bad = cmp ne {typ} %r, {truth}\nreturn i64 %bad', helper)

for typ in ('i8', 'u16', 'i32', 'i64'):
    helper = f'''function @reuse(%a:i64,%b:i64,%c:{typ}) -> i64 {{
block ^entry:
%flag = cmp eq i64 %a, %b
%small = convert trunc i32 i64 %flag
%shift = binary shl i64 1, %a
%third = copy i64 %c
%sum = binary add i64 %third, %small
%r = binary add i64 %sum, %shift
return i64 %r
}}
'''
    cases['boolean-fixed-effect-'+typ] = main('%r = call i64 @reuse(1, 1, 7)\n%bad = cmp ne i64 %r, 10\nreturn i64 %bad', helper)

# Address arguments retain bases/indexes until their scheduled read. A large
# stack copy also exercises capture before REP overwrites ABI carriers.
for size in (16, 80):
    helper = f'''global @data = {{ i64 19 i64 23 zero 80 }}
function @consume(%a:ptr,%b:ptr,%c:i64,%d:i64,%e:i64,%f:i64,%object:obj<{size}x8>) -> i64 {{
slot $received : obj<{size}x8>
block ^entry:
%x = load i64 %a
%y = load i64 %b
copyobj {size}x8 %object, $received
%z = addr $received
%w = load i64 %z
%xy = binary add i64 %x, %y
%r = binary add i64 %xy, %w
return i64 %r
}}
function @forward(%target:ptr,%p:ptr) -> i64 {{
block ^entry:
%q = index i64 %p, 1
%object = load obj<{size}x8> %p
%r = call i64 %target(%q,%p,1,2,3,4,%object) as (%a:ptr,%b:ptr,%c:i64,%d:i64,%e:i64,%f:i64,%object:obj<{size}x8>) -> i64
return i64 %r
}}
'''
    cases[f'address-bulk-{size}'] = main('%r = call i64 @forward(@consume,@data)\n%bad = cmp ne i64 %r, 61\nreturn i64 %bad', helper)

rows = []
cases['debug-boolean'] = '''function @identity(%x:i64) -> i64 {
block ^entry:
return i64 %x
}
function @main() -> i32 [role=entry] !dbg("placement.cpp", 3, 1) {
block ^entry:
%r = call i64 @identity(5) !dbg("placement.cpp", 4, 1)
%bad = cmp ne i64 %r, 5 !dbg("placement.cpp", 5, 1)
%result = convert trunc i32 i64 %bad !dbg("placement.cpp", 6, 1)
return i32 %result !dbg("placement.cpp", 7, 1)
}
'''
for name, source in cases.items():
    src = out/(name+'.lowir'); exe = out/name; mir = out/(name+'.mir')
    src.write_text(source)
    build = subprocess.run([str(cc), '--stats', '--dump-machine-ir', str(mir), '-o', str(exe), str(src)], capture_output=True, text=True)
    row = {'name':name, 'source_sha256':hashlib.sha256(src.read_bytes()).hexdigest(), 'compile':build.returncode, 'stderr':build.stderr}
    if build.returncode == 0:
        run = subprocess.run([str(exe)], capture_output=True, timeout=5)
        row.update(run=run.returncode, stdout=run.stdout.decode())
    rows.append(row)
    if name == 'debug-boolean' and not row['compile'] and not row['run']:
        text = mir.read_text()
        for line in (3, 4, 5, 6, 7):
            assert f'!dbg("placement.cpp", {line}, 1)' in text, (line, text)
        plain = out/'without-view'
        subprocess.run([str(cc), '-o', str(plain), str(src)], check=True)
        assert plain.read_bytes() == exe.read_bytes(), 'view changed native output'
(out/'results.json').write_text(json.dumps(rows, indent=2)+'\n')
failures = [r['name'] for r in rows if r['compile'] or r.get('run') or r.get('stdout')]
print(f'{len(rows)-len(failures)}/{len(rows)} call/placement cases passed')
if failures: print('\n'.join(failures))
raise SystemExit(bool(failures))
