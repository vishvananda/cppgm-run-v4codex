#!/usr/bin/env python3
"""Declaration type facts preserve unused-body constraints and query access."""
from pathlib import Path
import subprocess, tempfile, sys
ROOT=Path(__file__).resolve().parents[2]
BINARY=Path(sys.argv[1]).resolve() if len(sys.argv)>1 else ROOT/'dev/cppgm++'
# N3485 [expr.call] requires matching arguments even for an indirect call;
# [expr.ass] requires a modifiable lvalue; [expr.add] excludes void operands.
# [class.access] applies inside unevaluated decltype operands as well.
REJECTIONS=[
    'template<class T>int f(){int (*p)(int)=0;return p();}',
    'template<class T>int f(){int (*p)(int)=0;return p(1,2);}',
    'template<class T>int f(){int (*const p)(int)=0;p=0;return 0;}',
    'template<class T>int f(){void (*p)(int)=0;return p(1)+1;}',
    'template<class T>struct C{int (*p)(int);int f(){return p();}};',
    'class Secret{int value;};template<class T>int read(T& v){using V=decltype(v.value);return sizeof(V);}int main(){Secret s;return read(s);}',
]
if __name__=='__main__':
    with tempfile.TemporaryDirectory(prefix='pa14-declaration-types-') as tmp:
        work=Path(tmp)
        for i,source in enumerate(REJECTIONS):
            src=work/f'reject-{i}.cpp';src.write_text(source)
            r=subprocess.run([BINARY,'--emit-lowir','-O0','-o',work/'out',src],capture_output=True,text=True)
            assert r.returncode==1,(i,r.returncode,r.stderr)
            assert not any(s in r.stderr for s in ('AddressSanitizer','runtime error:','UndefinedBehaviorSanitizer')),(i,r.stderr)
        print(len(REJECTIONS),'declaration-type rejections PASS')
