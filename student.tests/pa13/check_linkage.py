#!/usr/bin/env python3
"""Internal polymorphic identities remain distinct across translation units."""
from pathlib import Path
import subprocess, sys, tempfile

root = Path(__file__).resolve().parents[2]
compiler = Path(sys.argv[1]).resolve() if len(sys.argv) > 1 else root/'dev/cppgm++'

def run(command):
    result = subprocess.run(list(map(str, command)), capture_output=True, text=True, timeout=60)
    assert result.returncode == 0, (command, result.returncode, result.stderr)

with tempfile.TemporaryDirectory(prefix='pa13-linkage-') as tmp:
    work = Path(tmp)
    for mode in ('anonymous', 'static-local'):
        sources = []
        for value in (1, 2):
            cls = 'struct B{virtual int f(){return '+str(value)+';}virtual ~B(){}};'
            body = 'B*p=new B;int answer=p->f();delete p;return answer;'
            source = work/(mode+str(value)+'.cpp')
            if mode == 'anonymous':
                text = 'namespace{'+cls+'}int run'+str(value)+'(){'+body+'}'
            else:
                text = 'static int local(){'+cls+body+'}int run'+str(value)+'(){return local();}'
            if value == 2:
                text += 'int run1();int main(){return run1()!=1 || run2()!=2;}'
            source.write_text(text); sources.append(source)
        ir, repeat, exe = work/'out.lowir', work/'repeat.lowir', work/'exec'
        for output in (ir, repeat):
            run([compiler, '--emit-lowir', '-O0', '--validate-lowir', '-o', output, *sources])
        assert ir.read_bytes() == repeat.read_bytes(), mode
        run([root/'dev/lowir2native-ref', '-O0', '-o', exe, ir]); run([exe])
        print(mode, 'support, methods, deleting entries, aliases and deterministic output passed')
