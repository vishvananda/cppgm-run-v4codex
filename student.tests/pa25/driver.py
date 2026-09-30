#!/usr/bin/env python3
"""Explicit PA25 object/link controls. Host cc creates foreign test inputs only."""
import os, pathlib, shutil, struct, subprocess, tempfile
ROOT = pathlib.Path(__file__).resolve().parents[2]
CXX = str(ROOT / 'dev/cppgm++')
checks = 0

def command(args, expected=0, **kw):
    global checks
    p = subprocess.run(list(map(str,args)), stdout=subprocess.PIPE, stderr=subprocess.PIPE, **kw)
    assert p.returncode == expected, (args,p.returncode,p.stderr.decode(errors='replace'))
    checks += 1
    return p

with tempfile.TemporaryDirectory(prefix='pa25-driver-') as tmp:
    d = pathlib.Path(tmp)
    def source(name,text):
        p = d/name; p.write_text(text); return p
    a = source('a.cc', '''static int x=11; extern int g;
inline int twice(int x) { return x+x; }
int* local_a() { return &x; }
int helper(int x) { return twice(x)+g; }
''')
    b = source('b.cc', '''static int x=22; int g=5;
int* local_a(); int helper(int);
inline int twice(int x) { return x+x; }
int main(int argc,char**) { return argc==1 && *local_a()==11 && x==22 &&
 local_a()!=&x && helper(7)==19 && twice(g)==10 ? 0 : 1; }
''')
    ao,bo,exe = d/'a.obj', d/'b.obj', d/'program'
    command([CXX,'-c','-o',ao,a]); command([CXX,'-c','-o',bo,b])
    legacy = d/'legacy.obj'
    command([CXX,'-c','--object-format=private','-o',legacy,b])
    command([CXX,'-o',exe,ao,bo]); command([exe]); separate = exe.read_bytes()
    command([CXX,'-o',exe,a,b]); command([exe]); assert exe.read_bytes()==separate
    command([CXX,'-o',exe,ao,b]); command([exe]); assert exe.read_bytes()==separate
    a.unlink(); b.unlink()
    command([CXX,'-o',exe,ao,bo],env={'PATH':''}); command([exe])
    command([CXX,'-o',exe,ao],expected=1)
    command([CXX,'-o',exe,ao,bo,bo],expected=1)
    bad = source('unresolved.cc','extern int missing(); int main(){ return missing(); }')
    command([CXX,'-o',exe,bad],expected=1)
    command([CXX,'--target=not-a-target','-c','-o',d/'bad.obj',bad],expected=1)
    macros = source('macros.cc','''#if VALUE != 7 || defined(REMOVE)
#error command macro order
#endif
#define VALUE 9
int main(){return VALUE-9;}
''')
    command([CXX,'-DVALUE=2','-D','VALUE=7','-DREMOVE','-UREMOVE','-o',exe,macros]); command([exe])
    quoted = d/'quoted'; user = d/'user'; quoted.mkdir(); user.mkdir()
    source('quoted/choice.h','int quoted_value(){return 13;}')
    source('user/choice.h','int user_value(){return 17;}')
    inc = source('quoted/main.cc','''#line 1 "imaginary/different.cc"
#include "choice.h"
#include <choice.h>
int main(){return quoted_value()+user_value()-30;}
''')
    command([CXX,'-I',user,'-o',exe,inc]); command([exe])
    # ELF weak selection must not demand relocations in a discarded definition.
    weak = source('weak.c','extern int absent(void); __attribute__((weak)) int choose(void){return absent();}')
    strong = source('strong.cc','extern "C" int choose(){return 29;} int main(){return choose()-29;}')
    command(['cc','-c','-o',d/'weak.o',weak])
    for inputs in [(d/'weak.o',strong),(strong,d/'weak.o')]:
        command([CXX,'-o',exe,*inputs]); command([exe])
    # Data relocations and indirect calls in both compiler and foreign objects.
    foreign = source('foreign.c','int value=37; int add(int x){return x+value;} int (*fp)(int)=add; int invoke(int x){return fp(x);}')
    use = source('use.cc','extern "C" int invoke(int); int main(){return invoke(5)-42;}')
    command(['cc','-c','-o',d/'libforeign.o',foreign])
    command([CXX,'-L'+str(d),'-lforeign','-o',exe,use]); command([exe])
    if shutil.which('clang'):
        # Clang's normal PIE defaults use GOTPCRELX for an external object.
        got_source = source('got.c','extern int shared; int read_shared(void){return shared;}')
        got_user = source('got.cc','int shared=41; extern "C" int read_shared(); int main(){return read_shared()-41;}')
        command(['clang','-c','-o',d/'got.o',got_source])
        command([CXX,'-o',exe,d/'got.o',got_user]); command([exe])
    tls_a = source('tls-a.cc','thread_local int shared_tls=31; int read_tls(){return shared_tls;}')
    tls_b = source('tls-b.cc','extern thread_local int shared_tls; int read_tls(); int main(){ shared_tls+=11; return read_tls()-42;}')
    strlen_a = source('strlen-a.cc','unsigned long size_a(const char* p){return __builtin_strlen(p);}')
    strlen_b = source('strlen-b.cc','unsigned long size_a(const char*); int main(){return size_a("abc")+__builtin_strlen("de")-5;}')
    for left,right in [(tls_a,tls_b),(strlen_a,strlen_b)]:
        command([CXX,'-c','-o',d/'extra-a.obj',left]); command([CXX,'-c','-o',d/'extra-b.obj',right])
        command([CXX,'-o',exe,d/'extra-a.obj',d/'extra-b.obj']); command([exe])
        command([CXX,'-o',exe,left,right]); command([exe])
    aligned_c = source('aligned.c','__attribute__((aligned(4096))) int aligned_function(void){return 19;}')
    aligned_cpp = source('aligned.cc','extern "C" int aligned_function(); int main(){return ((unsigned long)&aligned_function % 4096) || aligned_function()!=19;}')
    command(['cc','-c','-o',d/'aligned.o',aligned_c])
    command([CXX,'-o',exe,aligned_cpp,d/'aligned.o']); command([exe])
    # Private version-4 corruption checks remain explicit after PA26 changed
    # the default object contract to ELF independently of output extension.
    original = legacy.read_bytes()
    # Truncations throughout each record class must reject, not assert or read OOB.
    for size in sorted(set([0,1,7,8,15,24,48,len(original)//3,len(original)//2,len(original)-1])):
        broken = d/'broken.obj'; broken.write_bytes(original[:size])
        command([CXX,'--object-format=private','-o',exe,broken],expected=1)
    for offset,value in [(8,99),(16,3),(24,0),(32,3),(40,2),(48,2**63)]:
        broken = d/'broken.obj'; data=bytearray(original); struct.pack_into('<Q',data,offset,value); broken.write_bytes(data)
        command([CXX,'--object-format=private','-o',exe,broken],expected=1)
    broken.write_bytes(original+b'junk'); command([CXX,'--object-format=private','-o',exe,broken],expected=1)
print(f'{checks} PA25 driver checks passed')
