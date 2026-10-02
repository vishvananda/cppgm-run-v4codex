#!/usr/bin/env python3
"""Exact target-format facts, rounding boundaries and conservative FP effects."""
import pathlib, subprocess, tempfile
root=pathlib.Path(__file__).resolve().parents[2]
def run(*args):
 p=subprocess.run(list(map(str,args)),capture_output=True,text=True,timeout=120)
 assert p.returncode==0,(args,p.returncode,p.stdout,p.stderr)
 return p
with tempfile.TemporaryDirectory(prefix='pa32-floating-') as d:
 t=pathlib.Path(d); src=t/'facts.lowir'
 functions=[]; checks=[]
 def add(ty,code,expected):
  n=len(checks); functions.append(f'function @f{n}() -> {ty} [no_inline=yes] {{ block ^entry: {code} }}');checks.append((ty,expected))
 for ty in ['f32','f64','f80']:
  for op,answer in [('eq',0),('ne',1),('lt',0),('le',0),('gt',1),('ge',1)]:
   add('i64',f'%x = const {ty} inf %z = convert sitofp {ty} i32 0 %c = cmp {op} {ty} %x, %z return i64 %c',answer)
  add('i64',f'%z = const {ty} -0.0 %v = unary neg {ty} %z %c = cmp eq {ty} %v, 0.0 return i64 %c',1)
  add('i64',f'%z = const {ty} -0.0 %r = binary div {ty} 1.0, %z %c = cmp lt {ty} %r, 0.0 return i64 %c',1)
  add('i64',f'%z = const {ty} nan %c = cmp ne {ty} %z, %z return i64 %c',1)
 for ty,exact in [('f32',16777216),('f64',9007199254740992)]:
  add('i64',f'%x = const {ty} {exact+1}.0 %c = cmp eq {ty} %x, {exact}.0 return i64 %c',1)
  add('i64',f'%x = convert sitofp {ty} i64 {exact+1} %c = cmp eq {ty} %x, {exact}.0 return i64 %c',1)
 main=['function @main() -> i64 [role=entry] { block ^entry: %bad0 = const i64 0']
 for n,(ty,v) in enumerate(checks):main += [f'%v{n} = call {ty} @f{n}()',f'%c{n} = cmp ne {ty} %v{n}, {v}',f'%bad{n+1} = binary or i64 %bad{n}, %c{n}']
 src.write_text('\n'.join(functions+main+[f'return i64 %bad{len(checks)}','}']))
 for level in range(4):
  out=t/f'o{level}.lowir';run(root/'dev/lowiropt',f'-O{level}','-o',out,src)
  run(root/'dev/lowir','-o',t/'valid',out)
  run(root/'dev/lowir2native','-o',t/'native',out);run(t/'native')
  run(root/'dev/cppgm++',f'-O{level}','-o',t/'direct',src);run(t/'direct')
  run(root/'dev/cppgm++','-O0','-o',t/'replay',out);run(t/'replay')
  if level:
   text=out.read_text();assert 'return i64 1' in text
 # Observable rounding and exception flags remain runtime operations when
 # their fold would require an assumption about the dynamic environment.
 ir=t/'environment.lowir';ir.write_text('''function @rounded() -> f32 [linkage=c, no_inline=yes] { block ^entry:
 %x = convert sitofp f32 i64 16777217 return f32 %x }
function @signaling() -> i64 [linkage=c, no_inline=yes] { block ^entry:
 %x = cmp eq f64 snan, 0.0 return i64 %x }
''')
 host=t/'host.cpp';host.write_text('''extern "C" int fesetround(int);extern "C" int feclearexcept(int);extern "C" int fetestexcept(int);
extern "C" float rounded();extern "C" long signaling();
int main(){fesetround(0x800);float x=rounded();fesetround(0);feclearexcept(0x3f);long s=signaling();int flags=fetestexcept(1);return x!=16777218.0f||s||!flags;}
''')
 run(root/'dev/cppgm++','-O0','-c',host,'-o',t/'host.o')
 for level in range(4):
  run(root/'dev/cppgm++',f'-O{level}','-c',ir,'-o',t/'env.o')
  run('g++',t/'host.o',t/'env.o','-lm','-o',t/'env');run(t/'env')
print(f'PA32 floating facts: PASS ({len(checks)} cases x four levels x three execution paths; dynamic rounding and signaling flags)')
