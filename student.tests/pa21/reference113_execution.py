#!/usr/bin/env python3
"""An external throwing constructor exposes the failed-new allocation obligation."""
from pathlib import Path
import json, subprocess, sys
from reference113 import ROOT, original, revised
WORK = Path(sys.argv[1]).resolve(); WORK.mkdir(parents=True, exist_ok=True)
shim = WORK/'runtime.cpp'
shim.write_text('''#include <cstdlib>
#include <new>
int allocations;
void* operator new(std::size_t size) { ++allocations; return std::malloc(size); }
void operator delete(void* p) noexcept { --allocations; std::free(p); }
struct S { S(); };
S::S() { throw 7; }
extern "C" int fixture_main();
int main() { try { fixture_main(); } catch (int n) { return n!=7 || allocations; } return 2; }
''')
rows = []
for name, ir, expected in (('original', original, 1), ('revised', revised, 0)):
    src = WORK/(name+'.lowir'); obj = src.with_suffix('.o'); exe = WORK/name
    src.write_text(ir.replace('function @main() -> i32 [role=entry, binding=strong, keep_alias=yes]',
                             'function @fixture_main() -> i32 [binding=strong, object=fixture_main]'))
    commands = []
    for cmd in ([ROOT/'dev/cppgm++-ref','-c','-O0','-o',obj,src],
                ['g++','-std=c++11','-no-pie',obj,shim,'-o',exe], [exe]):
        p = subprocess.run(list(map(str,cmd)),capture_output=True,text=True,timeout=30)
        commands.append(dict(argv=list(map(str,cmd)),exit=p.returncode,stdout=p.stdout,stderr=p.stderr))
        assert p.returncode == (expected if cmd == [exe] else 0), commands
    rows.append(dict(name=name,expected_exit=expected,commands=commands))
(WORK/'results.json').write_text(json.dumps(dict(shim=shim.read_text(),rows=rows),indent=2)+'\n')
print('Failed-new reference proof PASS (original leaks; revised releases)')
