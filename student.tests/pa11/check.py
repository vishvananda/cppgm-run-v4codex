#!/usr/bin/env python3
"""Execute personal PA11 object/call and construction-order checks explicitly."""
from pathlib import Path
import subprocess
import tempfile
ROOT = Path(__file__).resolve().parents[2]
with tempfile.TemporaryDirectory(prefix='pa11-personal-') as directory:
    scratch = Path(directory)
    for name in ('member-addresses', 'construction-order', 'lexical-lifetime', 'array-lifetime', 'temporary-lifetime', 'access-adl', 'operator-calls'):
        source = Path(__file__).parent / (name + '.cpp')
        ir, exe = scratch / (name + '.lowir'), scratch / name
        for command in ([ROOT/'dev/cppgm++', '--emit-lowir', '-O0', '--validate-lowir', '-o', ir, source],
                        [ROOT/'dev/lowir2native-ref', '-O0', '-o', exe, ir], [exe]):
            subprocess.run([str(x) for x in command], check=True, timeout=60)
        print(name + ': validated LowIR, native exit 0')
    for name in ('access-selected-bad', 'access-conversion-bad', 'hidden-qualified-bad'):
        source = Path(__file__).parent / (name + '.cpp')
        result = subprocess.run([str(ROOT/'dev/cppgm++'), '--emit-lowir', '-O0', '-o', str(scratch/'bad.lowir'), str(source)],
                                capture_output=True, timeout=60)
        assert result.returncode, name + ': invalid program accepted'
        print(name + ': rejected')
