#!/usr/bin/env python3
"""Final-audit reducers for ownership across CFG, runtime and ABI boundaries."""
import hashlib
import itertools
import json
import pathlib
import subprocess
import sys

cc = pathlib.Path(sys.argv[1]).resolve()
out = pathlib.Path(sys.argv[2]).resolve()
out.mkdir(parents=True, exist_ok=True)
cases = {}


def program(body, helpers='', slots=''):
    return helpers + '\nfunction @main() -> i64 [role=entry] {\n' + slots + '\nblock ^entry:\n' + body + '\n}\n'


for transfer, typ, picked in itertools.product(('jump', 'branch', 'switch'), ('i8', 'u16', 'i64'), range(6)):
    helper = '''function @noise(%a:i64,%b:i64,%c:i64,%d:i64,%e:i64,%f:i64) -> i64 {
block ^entry:
%r = binary add i64 %a, %f
return i64 %r
}
function @choose(%a:i64,%b:i64,%c:i64,%d:i64,%e:i64,%f:i64) -> i64 {
block ^entry:
branch %a, ^hot, ^cold
block ^hot:
%noise = call i64 @noise(90,91,92,93,94,95)
TRANSFER
block ^join:
%first = phi i64 [^hot: %a]
%s1 = binary add i64 %first, %b
%s2 = binary add i64 %s1, %c
%s3 = binary add i64 %s2, %d
%s4 = binary add i64 %s3, %e
%s5 = binary add i64 %s4, %f
return i64 %s5
block ^fail:
return i64 99
block ^cold:
return i64 21
}
'''.replace('TRANSFER', {'jump': 'jump ^join', 'branch': 'branch %noise, ^join, ^fail',
                       'switch': 'switch %noise, ^fail, 185:^join'}[transfer])
    # Move each incoming carrier through the phi while using all six parameters
    # after the call; otherwise the preserved-register pool hides the home path.
    name = 'abcdef'[picked]
    helper = helper.replace('%first = phi i64 [^hot: %a]', f'%first = phi {typ} [^hot: %{name}]')
    helper = helper.replace('%first, %b', '%a, %b')
    prefix, tail = helper.split('%s1 =', 1)
    helper = prefix + '%s1 =' + tail.replace(f'%{name},', '%first,').replace(f', %{name}\n', ', %first\n')
    noise, choose = helper.split('function @choose', 1)
    helper = noise + 'function @choose' + choose.replace(f'%{name}:i64', f'%{name}:{typ}')
    cases[f'phi-parameter-{transfer}-{typ}-{picked}'] = program(
        '%r = call i64 @choose(1,2,3,4,5,6)\n%bad = cmp ne i64 %r, 21\nreturn i64 %bad', helper)

# EH registrations restore the receiving frame, including an aligned base and
# the lowest live dynamic allocation, while an intervening callee has its own.
cases['nested-aligned-unwind'] = program('''eh_try ^caught
%p = stack_alloc 37
store i64 42, %p
call void @thrower()
return i64 1
block ^caught:
%v = load i64 %p
%x = exception i64
%a = cmp ne i64 %v, 42
%b = cmp ne i64 %x, 7
%bad = binary or i64 %a, %b
return i64 %bad''', '''function @thrower() -> void {
slot $aligned : obj<64x64>
block ^entry:
%p = stack_alloc 129
store i64 99, %p
throw i64 7
}
''')

for typ, value in [('i8', -3), ('u16', 65533), ('i64', 42), ('i128', 2**90 + 7), ('f64', '42.5'), ('f80', '42.5L')]:
    cases['tls-eh-' + typ] = program(f'''eh_try ^caught
%x = load {typ} @cell
throw {typ} %x
block ^caught:
%v = exception {typ}
store {typ} %v, @output
%got = load {typ} @output
%bad = cmp ne {typ} %got, {value}
return i64 %bad''', f'global @cell : {typ} [storage=thread_local] = {value}\nglobal @output : {typ} [storage=thread_local] = zero\n')

rows = []
for name, source in cases.items():
    src, exe, mir = (out/(name + suffix) for suffix in ('.lowir', '.elf', '.mir'))
    src.write_text(source)
    build = subprocess.run([str(cc), '--stats', '--dump-machine-ir', str(mir), '-o', str(exe), str(src)], capture_output=True, text=True, timeout=20)
    row = dict(name=name, source_sha256=hashlib.sha256(src.read_bytes()).hexdigest(), compile=build.returncode, stderr=build.stderr)
    if not build.returncode:
        try:
            run = subprocess.run([str(exe)], capture_output=True, timeout=5)
            row.update(run=run.returncode, stdout=run.stdout.decode())
        except subprocess.TimeoutExpired:
            row['run'] = 'timeout'
    rows.append(row)
(out/'results.json').write_text(json.dumps(rows, indent=2) + '\n')
failures = [r for r in rows if r['compile'] or r.get('run') or r.get('stdout')]
print(f'{len(rows)-len(failures)}/{len(rows)} final-audit controls passed')
for row in failures:
    print(row['name'], row)
raise SystemExit(bool(failures))
