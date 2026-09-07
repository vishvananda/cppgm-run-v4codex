#!/usr/bin/env python3
"""Run every unchanged PA7 fixture explicitly against an alternate binary."""
import pathlib
import subprocess
import sys
import tempfile
root = pathlib.Path(__file__).resolve().parents[2]
compiler = pathlib.Path(sys.argv[1]).resolve()
with tempfile.TemporaryDirectory(prefix='pa7-course-') as tmp:
    out = pathlib.Path(tmp)/'out'
    cases = sorted((root/'pa7/tests').rglob('*.t'))
    for case in cases:
        expected = 0 if case.with_suffix('.ref.exit_status').read_text().strip() == 'EXIT_SUCCESS' else 1
        inputs = [case, *sorted(case.parent.glob(case.name+'[2-9]*'))]
        run = subprocess.run([compiler, '--emit-semantics', '-o', out, *inputs], capture_output=True, text=True)
        assert run.returncode == expected, (case, run.returncode, run.stderr)
        assert 'runtime error:' not in run.stderr and 'Sanitizer' not in run.stderr, (case, run.stderr)
        if expected == 0:
            assert out.read_bytes() == case.with_suffix('.ref').read_bytes(), case
    print(f'{len(cases)} unchanged PA7 course cases passed')
