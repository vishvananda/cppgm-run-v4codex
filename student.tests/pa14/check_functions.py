#!/usr/bin/env python3
"""Independent first-tier template behavior, validated and executed via PA8."""
from pathlib import Path
import subprocess, tempfile
root = Path(__file__).resolve().parents[2]
with tempfile.TemporaryDirectory(prefix='pa14-functions-') as tmp:
    for source in sorted((root/'student.tests/pa14').glob('*.cpp')):
        ir = Path(tmp)/(source.stem+'.lowir')
        exe = Path(tmp)/source.stem
        for command in ([root/'dev/cppgm++','--emit-lowir','-O0','--validate-lowir','-o',ir,source],
                        [root/'dev/lowir2native-ref','-O0','-o',exe,ir], [exe]):
            result = subprocess.run(list(map(str,command)), capture_output=True, text=True, timeout=60)
            assert result.returncode == 0, (source.name, command, result.returncode, result.stderr)
        print(source.name, 'PASS')
