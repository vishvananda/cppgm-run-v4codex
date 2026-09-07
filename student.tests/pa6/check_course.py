#!/usr/bin/env python3
"""Exercise all unchanged PA6 contracts directly, including multiple primary TUs."""
import pathlib
import subprocess
import sys
import tempfile
root=pathlib.Path(__file__).resolve().parents[2]
compiler=pathlib.Path(sys.argv[1]).resolve()
with tempfile.TemporaryDirectory(prefix='pa6-course-') as tmp:
    out=pathlib.Path(tmp)/'out'
    count=0
    for case in sorted((root/'pa6/tests').rglob('*.t')):
        inputs=[case,*sorted(case.parent.glob(case.name+'[2-9]*'))]
        expected=0 if case.with_suffix('.ref.exit_status').read_text().strip()=='EXIT_SUCCESS' else 1
        run=subprocess.run([compiler,'--emit-types','-o',out,*inputs],capture_output=True,text=True)
        assert run.returncode==expected,(case,run.returncode,run.stderr)
        assert 'runtime error:' not in run.stderr,(case,run.stderr)
        if not expected:
            assert out.read_bytes()==case.with_suffix('.ref').read_bytes(),case
        count+=1
    print('PA6 unchanged course contracts:',count,'passed')
