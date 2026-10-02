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
        for placement in ('incoming', 'call', 'stack'):
            case = text
            if placement == 'call':
                # A call forces parameters out of the incoming ABI carriers.
                case = 'function @barrier() -> void {\nblock ^entry:\nreturn void\n}\n'+case
                case = case.replace('    %address = index', '    call void @barrier()\n    %address = index')
            elif placement == 'stack':
                # Seven/eight are incoming stack parameters. Their folded
                # address needs both the base and index scratch registers.
                params = ', '.join(f'%p{i} : i64' for i in range(6))+', '
                for name in ('read', 'write'):
                    case = case.replace(f'function @{name}(%base', f'function @{name}({params}%base')
                    case = case.replace(f'@{name}(%base,', f'@{name}(0, 1, 2, 3, 4, 5, %base,')
            src = out/'input.lowir'; src.write_text(case)
            for level in ('-O0', '-O3'):
                ir = out/'optimized.lowir'
                subprocess.run([optimizer,level,src,'-o',ir],check=True)
                subprocess.run([backend,ir,'--dump-machine-ir',out/'program.mir','-o',out/'program'],check=True)
                subprocess.run([out/'program'],check=True,timeout=10)
print('indexed i128: three strides, load/store, incoming/preserved/stack addresses, O0/O3 PASS')
