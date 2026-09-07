#!/usr/bin/env python3
"""Exercise independent boundaries and runtime loops through the supplied backend."""
from pathlib import Path
import subprocess, tempfile, sys
from bench_inputs import runtime_source
ROOT=Path(__file__).resolve().parents[2]
binary=Path(sys.argv[1]).resolve() if len(sys.argv)>1 else ROOT/'dev/lowir'
with tempfile.TemporaryDirectory(prefix='pa8-native-') as tmp:
    tmp=Path(tmp)
    for group,count in [('sum',1000001),('swap',10000),('call',10000),('floating',10000)]:
        files=[]
        if group!='floating':
            helper=tmp/f'{group}-helper.lowir'
            subprocess.run([binary,'--exercise',group,'-o',helper],check=True)
            files.append(helper)
        harness=tmp/f'{group}-harness.lowir';harness.write_text(runtime_source(group,count));files.append(harness)
        combined=tmp/f'{group}.lowir'
        subprocess.run([binary,'-o',combined,*files],check=True)
        program=tmp/f'{group}.program'
        subprocess.run([ROOT/'dev/lowir2native-ref','-O0','-o',program,combined],check=True)
        result=subprocess.run([program],capture_output=True,timeout=30)
        assert result.returncode==0 and result.stdout==b'',(group,result)
print('PASS: complete sum domain, repeated aliased swap, dynamic callbacks, floating memory workloads')
