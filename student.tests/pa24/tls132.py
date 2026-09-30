#!/usr/bin/env python3
"""Exercise TLS through existing semantic controls and independent address cases."""
import hashlib, json, pathlib, re, subprocess, sys
root = pathlib.Path(__file__).resolve().parents[2]
compiler = pathlib.Path(sys.argv[1]).resolve()
out = pathlib.Path(sys.argv[2]).resolve(); out.mkdir(parents=True, exist_ok=True)
cases = {}
for lane in ('strict', 'structural', 'behavior'):
    for source in sorted((root/'pa24/tests'/lane).glob('*.t')):
        base = source.with_suffix('')
        if pathlib.Path(str(base)+'.ref.impl.exit_status').read_text().strip() != '0': continue
        text = source.read_text()
        if not re.search(r'^global .*=', text, re.M): continue
        def tls(m):
            line = m[0].replace(' readonly ', ' ')
            if 'thread_local' in line: return line
            if 'storage=readonly' in line: return line.replace('storage=readonly','storage=thread_local')
            if '[' in line: return line.replace('[','[storage=thread_local, ',1)
            return line.replace(' =',' [storage=thread_local] =',1)
        text = re.sub(r'^global [^\n]*=', tls, text, flags=re.M)
        cases[lane+'-'+source.stem] = (text, int(pathlib.Path(str(base)+'.ref.program.exit_status').read_text()),
                                      pathlib.Path(str(base)+'.ref.program.stdout').read_bytes())
def case(name, globals, body, helpers=''):
    cases[name] = (globals+'\n'+helpers+'\nfunction @main() -> i64 [role=entry] {\nblock ^entry:\n'+body+'\n}\n',0,b'')
for typ, value in [('i8','-42'),('u16','65000'),('i64','-123456'),('i128','18446744073709551658'),
                   ('f32','1.25'),('f64','1.25'),('f80','1.25')]:
    case('scalar-'+typ, f'global @g : {typ} [storage=thread_local] = 0',
         f'store {typ} {value}, @g\n%a = addr @g\n%v = load {typ} %a\n%bad = cmp ne {typ} %v, {value}\nreturn i64 %bad')
case('wrapper-addresses','''global @g : i64 [storage=thread_local] = 7
global @h : i64 [storage=thread_local] = 9
declare function @gw() -> ptr [tls_for=@g]
declare function @hw() -> ptr [tls_for=@h]
global @fn : ptr = addr @gw''','''%fp = load ptr @fn
%a = call ptr %fp() as () -> ptr
%b = call ptr @hw()
%r = call i64 @sum(@g, @h)
%bad0 = cmp ne ptr %a, @g
%bad1 = cmp ne ptr %b, @h
%bad2 = cmp ne i64 %r, 16
%bad3 = binary or i64 %bad0, %bad1
%bad = binary or i64 %bad3, %bad2
return i64 %bad''','''function @sum(%a : ptr, %b : ptr) -> i64 {
block ^entry:
%x = load i64 %a
%y = load i64 %b
%r = binary add i64 %x, %y
return i64 %r
}''')
case('bulk-two-symbols','''global @g [storage=thread_local] = { i64 42 zero 56 }
global @h [storage=thread_local] = { zero 64 }''','''copyobj 64x8 @g, @h
%v = load i64 @h
%bad = cmp ne i64 %v, 42
return i64 %bad''')
for target in ['i8','u16','i32','u32','i64','i128','f32','f64','f80']:
    for source in ['i8','u16','i32','u32','i64','i128','f32','f64','f80']:
        if source == target or (not source.startswith('f') and not target.startswith('f') and source[1:]==target[1:]): operation=f'%x = copy {target} 42'
        else:
            sf,tf = source.startswith('f'),target.startswith('f')
            width=lambda t: int(t[1:])
            op = ('fpext' if width(target)>width(source) else 'fptrunc') if sf and tf else ('fptoui' if target.startswith('u') else 'fptosi') if sf else ('uitofp' if source.startswith('u') else 'sitofp') if tf else ('trunc' if width(target)<width(source) else 'zext' if source.startswith('u') else 'sext')
            operation=f'%x = convert {op} {target} {source} 42'
        case('conversion-'+source+'-'+target, f'global @g : {target} [storage=thread_local] = 0',
             f'{operation}\nstore {target} %x, @g\n%y = load {target} @g\n%bad = cmp ne {target} %y, 42\nreturn i64 %bad')
case('derived-offset', 'global @g [storage=thread_local] = { i64 1 i64 42 }',
     '%p = index i64 @g, 1\nstore i64 73, %p\n%x = load i64 %p\n%bad = cmp ne i64 %x, 73\nreturn i64 %bad')
case('tls-function-table', 'global @g : ptr [storage=thread_local] = addr @value',
     '%v = call i64 @g() as () -> i64\n%bad = cmp ne i64 %v, 42\nreturn i64 %bad',
     'function @value() -> i64 {\nblock ^entry:\nreturn i64 42\n}')
records=[]
for name, (source, expected, stdout) in cases.items():
    src=out/(name+'.lowir'); src.write_text(source); exe=out/(name+'.exe'); mir=out/(name+'.mir')
    p=subprocess.run([str(compiler),'-O0','--stats','--dump-machine-ir',str(mir),'-o',str(exe),str(src)],capture_output=True)
    rec={'case':name,'compile_exit':p.returncode,'diagnostics':p.stderr.decode(),'source_sha256':hashlib.sha256(src.read_bytes()).hexdigest()}
    if not p.returncode:
        try:
            r=subprocess.run([str(exe)],capture_output=True,timeout=5)
            rec.update(exit=r.returncode,expected=expected,pass_=r.returncode==expected and r.stdout==stdout)
        except subprocess.TimeoutExpired: rec.update(pass_=False,timeout=True)
    else: rec['pass_']=False
    records.append(rec)
(out/'results.json').write_text(json.dumps(records,indent=2)+'\n')
failed=[r for r in records if not r['pass_']]
print(json.dumps(failed,indent=2)); print(f'TLS controls: {len(records)-len(failed)}/{len(records)}')
sys.exit(bool(failed))
