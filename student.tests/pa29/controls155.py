#!/usr/bin/env python3
"""Hosted driver, lexical, probe and canonical-type controls; run explicitly."""
import hashlib, json, pathlib, subprocess, sys
root = pathlib.Path(__file__).resolve().parents[2]
compiler = (root/'dev/cppgm++').resolve()
work = pathlib.Path(sys.argv[1] if len(sys.argv)>1 else '/tmp/pa29-155/controls').resolve()
work.mkdir(parents=True,exist_ok=True)
rows=[]
def run(name,args,ok=True,contains=(),absent=(),cwd=None):
    p=subprocess.run(list(map(str,args)),capture_output=True,cwd=cwd or root,timeout=30)
    out=p.stdout.decode(errors='replace');err=p.stderr.decode(errors='replace')
    valid=(p.returncode==0)==ok and all(s in out for s in contains) and all(s not in out for s in absent)
    rows.append(dict(name=name,args=list(map(str,args)),status=p.returncode,stdout=out,stderr=err,passed=valid))
    if not valid: print(name,p.returncode,out[-1000:],err[-1000:],flush=True)
    return valid
def source(name,text):
    p=work/(name+'.cpp');p.write_text(text);return p
for p in sorted((root/'student.tests/pa29/controls155').glob('*.cpp')):
    obj=work/(p.stem+'.o');exe=work/p.stem
    if run(p.stem+' compile',[compiler,'-c',p,'-o',obj]):
        if run(p.stem+' host link',['g++',obj,'-o',exe]):run(p.stem+' runtime',[exe])
for flag in ['--version','-v','-dumpmachine','-dumpversion','-print-search-dirs']:
    run(flag,[compiler,flag])
s=source('macros','''#if VALUE != 9 || defined(GONE)
#error macro option ordering
#endif
#if __cplusplus != 201402L || !defined(__STRICT_ANSI__)
#error language mode
#endif
macro_ok
''')
run('ordered macro controls',[compiler,'-E','-DVALUE=2','-UVALUE','-D','VALUE=9','-DGONE','-U','GONE','-std=c++14',s],contains=['identifier macro_ok'])
a=source('first','''#define LOCAL 1
first
''');b=source('second','''#ifdef LOCAL
#error leaked macros across translation units
#endif
second
''')
run('independent preprocessing units',[compiler,'-E',a,b],contains=['preproc 2','identifier first','identifier second'])
run('multiple outputs rejected',[compiler,'-E',a,b,'-o',work/'bad'],False)
run('conflicting modes',[compiler,'-E','-c',a],False)
run('output failure',[compiler,'-E',a,'-o',work/'missing'/'out'],False)
for flag in ['-D','-U','-include','-isystem','-o','-stdlib','-std']:
    run('missing argument '+flag,[compiler,'-E',a,flag],False)
run('frozen standard library refusal',[compiler,'-E','-stdlib=definitely-not-a-library',a],False)
run('frozen standard library match',[compiler,'-E','-stdlib=libstdc++',a])
forced1=source('forced1','#define STAGE 1\n');forced2=source('forced2','#if STAGE != 1\n#error include order\n#endif\n#undef STAGE\n#define STAGE 2\n')
s=source('forced-use','#if STAGE != 2\n#error missing forced headers\n#endif\nint main(){return 0;}\n')
run('forced include output',[compiler,'-E','-include',forced1,'-include',forced2,s],contains=['identifier main'])
run('forced include object',[compiler,'-c','-include',forced1,'-include',forced2,s,'-o',work/'forced.o'])
user=work/'user';system=work/'system';user.mkdir(exist_ok=True);system.mkdir(exist_ok=True)
(user/'pick.h').write_text('user_header\n#include_next <pick.h>\n')
(system/'pick.h').write_text('system_header\n#if __has_include_next(<pick.h>)\n#error duplicate search directory\n#endif\n')
s=source('include-search','#define HEADER <pick.h>\n#include HEADER\n')
run('include ordering and macro angle',[compiler,'-E','-nostdinc','-isystem',system,'-I',user,'-I',user,s],contains=['identifier user_header','identifier system_header'])
for token in ['__has_cpp_attribute(123)','__has_cpp_attribute(a::b::c)','__has_builtin(a,b)','__has_feature(a::b)','__has_warning(name)','__has_declspec_attribute("name")']:
    s=source('bad-probe','#if '+token+'\n#endif\n')
    run('malformed '+token,[compiler,'-E',s],False)
for literal in ['0b','0b102','0x.p1','0x1.p','0x1p+','0x1.0','0x1p3z','1.0F128junk']:
    s=source('bad-literal',literal+'\n');run('invalid literal '+literal,[compiler,'-E',s],False)
s=source('comment','// placeholder\nint ok;\n');s.write_bytes(b'//\x97\xff\r\n/*\x80*/ int ok;\n')
run('discarded comment bytes',[compiler,'-E',s],contains=['identifier ok'])
s.write_bytes(b'int \x97;\n');run('invalid source bytes',[compiler,'-E',s],False)
for expression in ['__is_const(int,int)','__remove_cv()','__make_signed(bool)','__make_unsigned(float)']:
    s=source('bad-trait','using T = '+expression+';' if expression.startswith(('__remove','__make')) else 'static_assert('+expression+',"bad");')
    run('invalid trait '+expression,[compiler,'-c',s,'-o',work/'bad.o'],False)
s=source('constexpr-udl','''constexpr long double operator"" _fp(long double v){return v*2;}
template<char... C> constexpr int operator"" _length(){return sizeof...(C);}
static_assert(0x1.8p+1_fp == 6.0L,"floating UDL");
static_assert(0b101_length == 5,"raw spelling");
template<unsigned long long N> struct A {static const int value=N;};
constexpr unsigned long long operator"" _n(unsigned long long n){return n;}
static_assert(A<0b11_n>::value==3,"literal query");
int main(){return 0;}
''')
run('constexpr literal categories',[compiler,'-c',s,'-o',work/'udl.o'])
a=source('multi-a','int a(){return 1;}');b=source('multi-b','int b(){return 2;}')
run('multiple compile inputs',[compiler,'-c',a,b],cwd=work)
assert (work/'multi-a.o').exists() and (work/'multi-b.o').exists()
for name in ['literals','traits','operations','shape-extension']:
    src=root/('student.tests/pa29/controls155/'+name+'.cpp');ir=work/(name+'.lowir');roundtrip=work/(name+'-roundtrip.lowir');exe=work/(name+'-lowir')
    if run(name+' hosted LowIR',[compiler,'-c','--emit-lowir','--validate-lowir',src,'-o',ir]):
        if run(name+' LowIR roundtrip',[root/'dev/lowir','-o',roundtrip,ir]):
            if run(name+' native inspection',[root/'dev/lowir2native','--dump-machine-ir',work/(name+'.mir'),'-o',exe,roundtrip]):
                run(name+' LowIR runtime',[exe])
result=dict(compiler_sha256=hashlib.sha256(compiler.read_bytes()).hexdigest(),checks=rows,passed=sum(r['passed'] for r in rows),total=len(rows))
(root/'student.tests/pa29/evidence155/controls.json').write_text(json.dumps(result,indent=2)+'\n')
print(result['passed'], '/',result['total'])
sys.exit(result['passed']!=result['total'])
