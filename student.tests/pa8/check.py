#!/usr/bin/env python3
"""Independent PA8 semantic probes, writer fixed points, and CLI failures."""
from pathlib import Path
import subprocess, tempfile, sys
ROOT = Path(__file__).resolve().parents[2]
BINARY = Path(sys.argv[1]).resolve() if len(sys.argv)>1 else ROOT/'dev/lowir'

def fn(body, params='', result='i64', metadata=''):
    return f'function @probe({params}) -> {result} {metadata} {{ block ^entry: {body} }}\n'

valid = {
 'helper-only': 'declare global @g : i64\ndeclare function @f(%x : ptr) -> f80\n',
 'empty': '',
 'forward-cross-file': [fn('%x = call i64 @later(2) return i64 %x'), 'function @later(%v : i64) -> i64 {block ^entry:return i64 %v}'],
 'globals-cross-file': ['global @fp : ptr = addr @later + 8\nalias object exported = @later', 'function @later() -> void {block ^x:return void}'],
 'data-addends': 'global @g = { i8 0x80 zero 3 f32 -0.0f f64 0x1.8p+2 f80 1.234567890123456789L ptr addr @g - 8 }',
 'all-metadata': 'global @g : i64 [section=.data_custom, storage=thread_local, object=object_data, binding=weak] = zero\n' + fn('return i64 0',metadata='[tls_for=@g, keep_alias=yes, prefer_local=yes, object_root=yes, force_inline=yes, inline_hint=yes, no_inline=yes]'),
 'stable-query': 'declare function @q(%p : ptr [object_bytes=8], %n : u32) -> f64 [query=stable_prefix]\n',
 'indirect-object': fn('%x = call obj<8x4> %fp($x) as (%v : obj<8x4>) -> obj<8x4> return obj<8x4> %x', '%fp : ptr', 'obj<8x4>').replace('block ^entry:', 'slot $x : obj<8x4> block ^entry:'),
 'indirect-variadic': fn('%x = call i64 %fp(1,2,3) as (%x : i64) -> i64 [arity=variadic, effects=readnone, unwind=no] return i64 %x','%fp : ptr'),
 'copy-elision': 'declare function @c(%dst : ptr, %src : ptr) -> void\n'+fn('call void @c(%a,%b) [elision=copy] return i64 0','%a : ptr, %b : ptr'),
 'phi-parallel': 'function @loop() -> i64 { block ^a: jump ^b block ^b: %x = phi i64 [^a:1,^b:%y] %y = phi i64 [^a:2,^b:%x] branch 0,^b,^c block ^c: return i64 %x }',
 'phi-deduplicated-edges': 'function @f() -> i64 {block ^a: branch 1,^b,^b block ^b: %x = phi i64 [^a:9] return i64 %x}',
 'phi-switch-edges': 'function @f() -> i64 {block ^a: switch 1,^b,1:^b,2:^b block ^b: %x = phi i64 [^a:3] return i64 %x}',
 'narrow-truth': fn('%c = cmp eq i16 1,1 %v = binary and i1 %c,1 %r = convert zext i64 i1 %v return i64 %r'),
 'reassignment': fn('%v = const i64 1 %v = binary add i64 %v,2 return i64 %v'),
 'integer-retype': fn('%v = const u32 0xffffffff %x = copy i32 %v %r = convert sext i64 i32 %x return i64 %r'),
 'volatile': fn('%x = load volatile i64 %p store volatile i64 %x,%p return i64 %x','%p : ptr'),
 'va-stack': fn('%s = stack_alloc 64 va_start %s %x = va_arg i64 %s return i64 %x',metadata='[arity=variadic]'),
 'eh-records': 'declare global @rtti : ptr\n'+fn('eh_try ^handler eh_catch @rtti,7 eh_filter @rtti,8 eh_catch_all,9 eh_cleanup eh_end return i64 0 block ^handler: %s = exception_selector i64 resume'),
 'debug': fn('%x = const i64 1 !dbg(src/input.cc,4,5) return i64 %x !dbg(src/input.cc,5,6)',metadata='!dbg(src/input.cc,3,1)'),
 'all-conversions': fn('%a = convert sext i64 i8 -1 %b = convert zext i32 u8 0xff %c = convert trunc i16 i64 %a %d = convert sitofp f32 i64 %a %e = convert uitofp f64 i32 %b %f = convert fpext f80 f32 %d %g = convert fptrunc f32 f80 %f %h = convert fptosi i64 f32 %g %i = convert fptoui i64 f64 %e return i64 %h'),
 'wide-integer-float': fn('%a = const f64 18446744073709551615 %b = const f80 -9223372036854775808 return i64 0'),
 'floating-specials': fn('%a = const f80 -inf %b = const f64 NAN %c = const f32 SNAN return i64 0'),
 'legacy-storage': 'declare global @a readonly : i64\nglobal @b thread_local : i32 = 2',
}
invalid = {
 'unknown-symbol': fn('%p = addr @missing return i64 0'),
 'missing-temp': fn('return i64 %missing'),
 'forward-ordinary': fn('%x = copy i64 %y %y = const i64 2 return i64 %x'),
 'self-first-definition': fn('%x = binary add i64 %x,1 return i64 %x'),
 'type-changing-reassignment': fn('%x = const i64 1 %x = const f64 1.0 return i64 0'),
 'duplicate-global': 'global @x : i64 = 0\nglobal @x : i64 = 1',
 'cross-kind-symbol': 'declare global @x\ndeclare function @x() -> void',
 'duplicate-files': ['declare global @x','declare global @x'],
 'missing-block': 'function @x() -> void {}',
 'empty-block': 'function @x() -> void {block ^x:}',
 'foreign-block': 'function @x() -> void {block ^x:jump ^y}\nfunction @y() -> void {block ^y:return void}',
 'phi-nonpredecessor': 'function @x() -> i64 {block ^a:jump ^b block ^b:%x=phi i64 [^c:1] return i64 %x block ^c:return i64 0}',
 'phi-duplicate': 'function @x() -> i64 {block ^a:branch 0,^b,^c block ^c:jump ^b block ^b:%x=phi i64 [^a:1,^a:2] return i64 %x}',
 'phi-undefined': 'function @x() -> i64 {block ^a:jump ^b block ^b:%x=phi i64 [^a:%missing] return i64 %x}',
 'phi-EH-not-ordinary': fn('eh_try ^b return i64 0 block ^b:%x=phi i64 [^entry:1] return i64 %x'),
 'call-arity': 'declare function @f(%a:i64)->i64\n'+fn('%x=call i64 @f() return i64 %x'),
 'call-parameter': 'declare function @f(%a:f64)->i64\n'+fn('%a=const i64 1 %x=call i64 @f(%a) return i64 %x'),
 'call-result': 'declare function @f()->f64\n'+fn('%x=call i64 @f() return i64 %x'),
 'call-nonptr': fn('%x=call i64 %n() as ()->i64 return i64 %x','%n:i64'),
 'call-slot-address': 'declare function @f(%p:ptr)->void\n'+fn('call void @f($x) return i64 0').replace('block ^entry:', 'slot $x:i64 block ^entry:'),
 'void-parameter': 'declare function @f(%x:void)->void',
 'bad-result-destination': fn('%x = store i64 1,%p return i64 0','%p:ptr'),
 'missing-result-destination': fn('const i64 1 return i64 0'),
 'return-mismatch': fn('return f64 1.0'),
 'store-mismatch': fn('%x=const f64 1.0 store i64 %x,%p return i64 0','%p:ptr'),
 'load-nonptr': fn('%v=load i64 %x return i64 %v','%x:i64'),
 'addr-temp': fn('%p=addr %x return i64 0','%x:i64'),
 'index-noninteger': fn('%p=index i64 %x,1.0 return i64 0','%x:ptr'),
 'float-bitwise': fn('%p=binary and f64 1.0,2.0 return i64 0'),
 'operator-family': fn('%p=binary eq i64 1,2 return i64 %p'),
 'same-width-sext': fn('%p=convert sext i64 i64 1 return i64 %p'),
 'wrong-float-direction': fn('%p=convert fpext f32 f80 1.0L return i64 0'),
 'floating-int-literal': fn('%p=const i64 1.25 return i64 %p'),
 'null-int-literal': fn('%p=const i64 nullptr return i64 %p'),
 'duplicate-role': 'declare function @a()->void [role=init]\ndeclare function @b()->void [role=init]',
 'role-kind': 'declare global @g [role=entry]',
 'metadata-duplicate': 'declare function @f()->void [effects=readnone] [effects=readonly]',
 'metadata-global-boundary': 'declare global @g [effects=readnone]',
 'metadata-global-function': 'declare global @g [force_inline=yes]',
 'metadata-bad-flag': 'declare function @f()->void [keep_alias=no]',
 'metadata-unknown': 'declare function @f()->void [mystery=yes]',
 'metadata-conflict-storage': 'global @g readonly : i64 [storage=thread_local] = 0',
 'metadata-object-size-float': 'declare function @f(%p:ptr [object_bytes=1.5])->void',
 'metadata-debug-column': fn('return i64 0',metadata='!dbg(x.cc,1,0)'),
 'metadata-signature-symbol': fn('%x=call i64 %p() as ()->i64 [object=thing] return i64 %x','%p:ptr'),
 'bulk-size-zero': fn('zeroinit 0x8 %p return i64 0','%p:ptr'),
 'bulk-align-nonpower': fn('zeroinit 8x6 %p return i64 0','%p:ptr'),
 'bulk-object-mismatch': fn('copyobj 8x8 %v,%p return i64 0','%v:obj<16x8>,%p:ptr'),
 'object-align': 'declare function @f(%p:obj<8x3>)->void',
 'scalar-address-init': 'global @g:i64=addr @g',
 'object-global-item': 'global @g={obj<8x8> 0}',
 'atomic-order': fn('%x=atomic_load i64 %p,9 return i64 %x','%p:ptr'),
}

def run_case(directory, label, source, success):
    parts = source if isinstance(source,list) else [source]
    files=[]
    for i, text in enumerate(parts):
        path=directory/f'{label}-{i}.lowir';path.write_text(text);files.append(path)
    output=directory/f'{label}.out'
    result=subprocess.run([BINARY,'-o',output,*files],capture_output=True,text=True,timeout=10)
    expected=0 if success else 1
    assert result.returncode==expected,(label, result.returncode, result.stderr)
    assert 'Sanitizer' not in result.stderr,(label,result.stderr)
    if success:
        second=directory/f'{label}.second'
        again=subprocess.run([BINARY,'-o',second,output],capture_output=True,text=True,timeout=10)
        assert again.returncode==0,(label,again.stderr)
        assert output.read_bytes()==second.read_bytes(),(label,'writer not at fixed point')
        return output.read_text()

with tempfile.TemporaryDirectory(prefix='pa8-personal-') as tmp:
    directory=Path(tmp)
    outputs={name:run_case(directory,name,text,True) for name,text in valid.items()}
    for name,text in invalid.items():run_case(directory,name,text,False)
    assert 'f64 18446744073709551615' in outputs['wide-integer-float']
    # Preserve the semantic distinction between quiet and signalling NaN.
    assert 'snan' in outputs['floating-specials'].lower(), 'writer lost signalling NaN'
    for args in ([],['--exercise','wrong','-o',directory/'bad'],['-o',directory/'bad','missing'],['--exercise','sum','-o'],['--unknown']):
        result=subprocess.run([BINARY,*args],capture_output=True,text=True)
        assert result.returncode==1,(args,result)
    for exercise in ('sum','swap','call'):
        out=directory/f'{exercise}.lowir'
        assert subprocess.run([BINARY,'--exercise',exercise,'-o',out],capture_output=True).returncode==0
        assert '@main' not in out.read_text()
print(f'PASS: {len(valid)} valid + {len(invalid)} invalid semantic cases, fixed points and CLI checks')
