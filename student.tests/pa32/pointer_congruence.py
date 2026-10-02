#!/usr/bin/env python3
"""Reduced proof witness for the PA32 even-stride reference correction.

A timeout is only an execution observation. The invariant end-8*k == end
modulo 8 proves nontermination for differing residues without timing assumptions.
"""
import pathlib, subprocess, tempfile
root=pathlib.Path(__file__).resolve().parents[2]
def run(*args):
    p=subprocess.run(list(map(str,args)),capture_output=True,text=True,timeout=60)
    assert p.returncode==0,(args,p.returncode,p.stdout,p.stderr)
    return p
fixture=root/'pa32/tests/o1/500-effect-free-loop-deleted-twin-backward'
reducer='''function @walk(%begin : ptr, %end : ptr) -> ptr {
 block ^entry: jump ^head
 block ^head:
  %p = phi ptr [^entry: %end, ^body: %next]
  %more = cmp ne ptr %begin, %p
  branch %more, ^body, ^done
 block ^body:
  %next = index i8 %p, -8
  jump ^head
 block ^done: return ptr %begin
}
'''
with tempfile.TemporaryDirectory(prefix='pa32-congruence-') as tmp:
    d=pathlib.Path(tmp)
    for label,body,name in [('reduced',reducer,'walk'),('fixture',fixture.with_suffix('.t').read_text(),'destroy_backward'),
                            ('corrected',fixture.with_suffix('.ref').read_text(),'destroy_backward')]:
        for begin,end,returns in [(0,0,True),(0,16,True),(1,17,True),(1,16,False)]:
            src=d/'input.lowir'
            src.write_text(body+f'''function @main() -> i32 [role=entry] {{
 slot $a : obj<32x8>
 block ^entry:
  %a = addr $a
  %begin = index i8 %a, {begin}
  %end = index i8 %a, {end}
  %r = call ptr @{name}(%begin, %end)
  %bad = cmp ne ptr %r, %begin
  %result = convert trunc i32 i64 %bad
  return i32 %result
}}
''')
            for level in range(4):
                ir=d/'optimized.lowir';exe=d/'exe'
                run(root/'dev/lowiropt',f'-O{level}','-o',ir,src)
                run(root/'dev/lowir','-o',d/'validated',ir)
                for backend in ['cppgm++','lowir2native']:
                    flags=['-O0'] if backend=='cppgm++' else []
                    run(root/'dev'/backend,*flags,'-o',exe,ir)
                    if returns:run(exe)
                    else:
                        try:
                            p=subprocess.run([str(exe)],capture_output=True,timeout=.1)
                        except subprocess.TimeoutExpired: pass
                        else:raise AssertionError((label,level,backend,'noncongruent call returned',p.returncode))
    # Exhaust the finite 8-bit model as an independent arithmetic check.
    for begin in range(256):
        for end in range(256):
            reaches=any((end-8*k)%256==begin for k in range(32))
            assert reaches==((begin-end)%8==0)
print('pointer congruence: PASS (reducer/input/corrected reference, four cases x four levels x two backends; 65536 residue pairs)')
