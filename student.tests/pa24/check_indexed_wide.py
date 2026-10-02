#!/usr/bin/env python3
"""Check both words of dynamic indexed loads/stores, with and without pressure."""
import pathlib
import subprocess
import sys
import tempfile

backend = pathlib.Path(sys.argv[1] if len(sys.argv) > 1 else 'dev/lowir2native').resolve()
optimizer = pathlib.Path(sys.argv[2] if len(sys.argv) > 2 else 'dev/lowiropt').resolve()
source = pathlib.Path('student.tests/pa24/indexed-wide.lowir').read_text()
with tempfile.TemporaryDirectory(prefix='pa24-indexed-wide-') as directory:
    out = pathlib.Path(directory)
    for stride in (16, 32, 64):
        text = source.replace('obj<32x16>', f'obj<{stride}x16>')
        text = text.replace('  zero 16\n', f'  zero {stride-16}\n' if stride > 16 else '')
        for pressure in (False, True):
            case = text
            if pressure:
                # A call forces the pointer/index parameters out of incoming
                # carriers, exercising a folded address with two scratch parts.
                case = 'function @barrier() -> void {\nblock ^entry:\nreturn void\n}\n'+case
                case = case.replace('    %address = index', '    call void @barrier()\n    %address = index')
            src = out/'input.lowir'; src.write_text(case)
            for level in ('-O0', '-O3'):
                ir = out/'optimized.lowir'
                subprocess.run([optimizer,level,src,'-o',ir],check=True)
                subprocess.run([backend,ir,'--dump-machine-ir',out/'program.mir','-o',out/'program'],check=True)
                subprocess.run([out/'program'],check=True,timeout=10)
print('indexed i128: three strides, load/store, incoming/spilled addresses, O0/O3 PASS')
