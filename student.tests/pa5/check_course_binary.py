#!/usr/bin/env python3
"""Run an isolated compiler (including sanitizers) against unchanged PA5 contracts."""
import os
import pathlib
import subprocess
import sys
import tempfile

root=pathlib.Path(__file__).resolve().parents[2]
compiler=pathlib.Path(sys.argv[1]).resolve()
env=dict(os.environ,ASAN_OPTIONS='detect_leaks=1')
count=0
with tempfile.TemporaryDirectory(prefix='pa5-course-binary-') as tmp:
    output=pathlib.Path(tmp)/'tree.ast'
    for source in sorted((root/'pa5/tests').rglob('*.t')):
        companions=sorted(p for p in source.parent.glob(source.name+'*')
                          if p.name[len(source.name):].isdigit())
        result=subprocess.run([compiler,'--emit-ast','-o',output,source,*companions],
                              env=env,capture_output=True,text=True,timeout=30)
        assert not any(x in result.stderr for x in ('AddressSanitizer','LeakSanitizer','runtime error:')),(source,result.stderr)
        status=source.with_suffix('.ref.exit_status').read_text().strip()
        assert result.returncode==(0 if status=='EXIT_SUCCESS' else 1),(source,result.returncode,result.stderr)
        if status=='EXIT_SUCCESS':
            assert output.read_bytes()==source.with_suffix('.ref').read_bytes(),source
        count+=1
print(f'PA5 isolated compiler: {count}/{count} contracts pass')
