#!/usr/bin/env python3
"""Independent cross-handoff controls; preserve failing reducers in evidence."""
import json, pathlib, subprocess, sys, tempfile

root = pathlib.Path(__file__).resolve().parents[2]
compiler = pathlib.Path(sys.argv[1]).resolve() if len(sys.argv) > 1 else root/'dev/lowir2native'
evidence = pathlib.Path(sys.argv[2]) if len(sys.argv) > 2 else None
if evidence:
    evidence.mkdir(parents=True,exist_ok=True)
cases = {}

def program(body, helpers='', slots=''):
    return helpers+'\nfunction @main() -> i64 [role=entry] {\n'+slots+'\nblock ^entry:\n'+body+'\n}\n'

for op, answer in [('add',4), ('sub',10), ('and',5), ('or',-1), ('xor',-6)]:
    cases['wide-rhs-'+op] = program(f'%small = const i8 -3\n%x = binary {op} i128 7, %small\n%bad = cmp ne i128 %x, {answer}\nreturn i64 %bad')
cases['wide-slot-actual'] = program('store i8 -3, $small\n%x = binary add i128 7, $small\n%bad = cmp ne i128 %x, 4\nreturn i64 %bad',slots='slot $small : i8')

for register, param, operation in [('rdx','%c','%q = binary div i64 84, 2'), ('rcx','%d','%q = binary shl i64 21, %a')]:
    helper = f'''function @f(%a : i64, %b : i64, %c : i64, %d : i64) -> i64 {{
block ^entry:
%held = binary add i64 {param}, 1
{operation}
%bad0 = cmp ne i64 %held, 8
%bad1 = cmp ne i64 %q, 42
%bad = binary or i64 %bad0, %bad1
return i64 %bad
}}
'''
    cases['reused-fixed-'+register] = program('%r = call i64 @f(1, 0, 7, 7)\nreturn i64 %r',helper)

# A zero index keeps the original pointer as a value. Parallel argument setup
# must consume it before another argument overwrites that incoming register.
helper = '''global @cell : i64 = 42
function @read(%unused : i64, %p : ptr) -> i64 {
block ^entry:
%v = load i64 %p
%bad = cmp ne i64 %v, 42
return i64 %bad
}
function @forward(%p : ptr) -> i64 {
block ^entry:
%derived = index i64 %p, 0
%r = call i64 @read(0, %derived)
return i64 %r
}
'''
cases['call-address-dependency'] = program('%r = call i64 @forward(@cell)\nreturn i64 %r',helper)

cases['indexed-spilled-store'] = program('''%base = addr @array
jump ^body
block ^body:
%index = phi i64 [^entry: 1]
%value = phi i64 [^entry: 42]
%address = index i64 %base, %index
store i64 %value, %address
%second = index i64 %base, 1
%got = load i64 %second
%bad = cmp ne i64 %got, 42
return i64 %bad''', 'global @array = { zero 512 }')

for source_type, target_type in [('i8','i64'),('i8','i128'),('f32','f64'),('f64','f80')]:
    cases['indexed-store-'+source_type+'-'+target_type] = program(f'''%base = addr @array
jump ^body
block ^body:
%index = phi i64 [^entry: 1]
%value = phi {source_type} [^entry: 42]
%address = index {target_type} %base, %index
store {target_type} %value, %address
%second = index {target_type} %base, 1
%got = load {target_type} %second
%bad = cmp ne {target_type} %got, 42
return i64 %bad''', 'global @array = { zero 1024 }')

cases['indexed-folded-compare'] = program('''%base = addr @array
jump ^body
block ^body:
%index = phi i64 [^entry: 1]
%address = index i64 %base, %index
%value = load i64 %address
%bad = cmp ne i64 42, %value
return i64 %bad''', 'global @array = { i64 0 i64 42 zero 512 }')

body = 'jump ^body\nblock ^body:\n%byte = phi u8 [^entry: 7]\n'
body += '\n'.join(f'%v{k} = binary add i64 {k}, 10' for k in range(11))
body += '\n%product = binary mul u8 3, %byte\n%bad0 = cmp ne u8 %product, 21\n'
prior = '%v0'
for k in range(1,11):
    body += f'%sum{k} = binary add i64 {prior}, %v{k}\n'
    prior = f'%sum{k}'
body += f'%bad1 = cmp ne i64 {prior}, 165\n%bad = binary or i64 %bad0, %bad1\nreturn i64 %bad'
cases['carry-byte-multiply-effect'] = program(body)

for t, negative_zero in [('f32',1<<31),('f64',1<<63)]:
    bits = 'u32' if t == 'f32' else 'i64'
    cases['negation-bits-'+t] = program(f'''%positive = unary neg {t} -0.0
store {t} %positive, $value
%bits = load {bits} $value
%bad0 = cmp ne {bits} %bits, 0
%negative = unary neg {t} 0.0
store {t} %negative, $value
%neg_bits = load {bits} $value
%bad1 = cmp ne {bits} %neg_bits, {negative_zero}
%bad = binary or i64 %bad0, %bad1
return i64 %bad''',slots=f'slot $value : {t}')

for dst in ['i64','i128']:
    cases['x87-rounding-restored-'+dst] = program(f'''%integer = convert fptosi {dst} f80 42.75L
%next = binary add f80 1.0L, 0.00000000000000000008131516293641283255055896006524562835693359375L
%bad0 = cmp ne f80 %next, 1.000000000000000000108420217248550443400745280086994171142578125L
%bad1 = cmp ne {dst} %integer, 42
%bad = binary or i64 %bad0, %bad1
return i64 %bad''')

cases['literal-payload-roundtrip'] = program('''%p = addr @payload
%upper = index i8 %p, 8
%high = load i64 %upper
%bad0 = cmp ne i64 %high, -1
%single = addr @single
%float_bits = load u32 %single
%bad1 = cmp ne u32 %float_bits, 1065353217
%double = addr @double
%double_bits = load i64 %double
%bad2 = cmp ne i64 %double_bits, 4607182418800017409
%ab = binary or i64 %bad0, %bad1
%bad = binary or i64 %ab, %bad2
return i64 %bad''', '''global @payload : i128 = 340282366920938463463374607431768211455
global @single : f32 = 1.000000059604644775390625000000001
global @double : f64 = 1.000000000000000111022302462515654042363166809082031250000001
''')

with tempfile.TemporaryDirectory(prefix='pa24-audit130-') as directory:
    d = pathlib.Path(directory)
    results = []
    for name, source in cases.items():
        path, exe, mir = d/'input.lowir', d/'program', d/'program.mir'
        path.write_text(source)
        if mir.exists():
            mir.unlink()
        compile_run = subprocess.run([str(compiler),'-o',str(exe),'--dump-machine-ir',str(mir),str(path)],capture_output=True,text=True)
        run = subprocess.run([str(exe)],timeout=10) if compile_run.returncode == 0 else None
        row = {'name':name,'compile':compile_run.returncode,'run':run.returncode if run else None}
        if row['compile'] or row['run']:
            row.update(source=source,stderr=compile_run.stderr,mir=mir.read_text() if mir.exists() else '')
        if evidence:
            (evidence/(name+'.lowir')).write_text(source)
            if mir.exists():
                (evidence/(name+'.mir')).write_bytes(mir.read_bytes())
            if compile_run.returncode == 0:
                (evidence/(name+'.elf')).write_bytes(exe.read_bytes())
        results.append(row)
    print(json.dumps(results,indent=2))
    sys.exit(any(row['compile'] or row['run'] for row in results))
