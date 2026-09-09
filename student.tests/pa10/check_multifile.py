#!/usr/bin/env python3
"""PA10's combined-program driver: typed linkage survives semantic TU release."""
from pathlib import Path
import subprocess
import sys
import tempfile
ROOT = Path(__file__).resolve().parents[2]
COMPILER = Path(sys.argv[1]).resolve() if len(sys.argv) > 1 else ROOT/'dev/cppgm++'
SOURCES = [r'''
namespace N {int f(int); int f(bool); extern int value; extern int data[];}
namespace C {extern "C" int cfun(int);}
inline int twice(int n){return n*2;}
static int local=1; static int get(){return local;}
int other(); int third();
int main(){return N::f(3)==10 && N::f(true)==5 && N::value==7 && N::data[1]==9
 && C::cfun(4)==6 && get()==1 && other()==14 && third()==11 && twice(2)==4 ? 0:1;}
''', r'''
namespace N {int value=7; int data[2]={8,9}; int f(int n){return n+value;} int f(bool n){return n?5:0;}}
extern "C" int cfun(int n){return n+2;}
inline int twice(int n){return n*2;}
static int local=2; static int get(){return local;}
static const int&r=6;
int other(){return twice(get())+r+4;}
''', r'''
namespace N {extern int value; int f(int);}
static int local=3; static int get(){return local;}
static const int&r=8;
int third(){return get()+r;}
''']
with tempfile.TemporaryDirectory(prefix='pa10-multifile-') as directory:
    work = Path(directory)
    paths=[]
    for i, source in enumerate(SOURCES):
        path=work/f'{i}.cpp';path.write_text(source);paths.append(path)
    # Definition before/after declaration, repeated inline definitions, and
    # three TUs with same-name internal objects/functions/reference temporaries.
    for order in (paths, list(reversed(paths))):
        ir=work/'program.lowir'; exe=work/'program'
        result=subprocess.run([COMPILER,'--emit-lowir','--validate-lowir','-O0','-o',ir,*order],capture_output=True,text=True,timeout=30)
        assert result.returncode==0,result.stderr
        result=subprocess.run([ROOT/'dev/lowir2native-ref','-O0','-o',exe,ir],capture_output=True,text=True,timeout=30)
        assert result.returncode==0,(result.stderr,ir.read_text())
        assert subprocess.run([exe],timeout=10).returncode==0
    print('PASS: multi-file native linkage in both input orders')
