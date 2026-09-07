#!/usr/bin/env python3
"""Validate live AST links directly for course successes and audit regressions."""
import pathlib
import subprocess
import sys
import tempfile
from check_audit import cases

root = pathlib.Path(__file__).resolve().parents[2]
api = pathlib.Path(sys.argv[1]).resolve()
sources = []
for source in sorted((root/'pa5/tests').rglob('*.t')):
    if source.with_suffix('.ref.exit_status').read_text().strip() == 'EXIT_SUCCESS':
        sources.append(source)
        sources.extend(sorted(p for p in source.parent.glob(source.name+'*')
                              if p.name[len(source.name):].isdigit()))
course_count = len(sources)
with tempfile.TemporaryDirectory(prefix='pa5-graph-audit-') as tmp:
    for name, text, _ in cases:
        source = pathlib.Path(tmp)/(name+'.cpp')
        source.write_text(text)
        sources.append(source)
    run = subprocess.run([api, *sources], capture_output=True, text=True, timeout=60)
    assert run.returncode == 0, (run.returncode, run.stderr)
    assert not any(s in run.stderr for s in ('Sanitizer', 'runtime error:')), run.stderr
    print(run.stdout, end='')
print(f'PA5 direct graphs: {course_count} course TUs + {len(cases)} audit TUs pass')
