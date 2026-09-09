#!/usr/bin/env python3
"""Fixed structural work evidence for combined-source linkage, without timing claims."""
from pathlib import Path
import hashlib
import json
import subprocess
import tempfile
ROOT=Path(__file__).resolve().parents[2]
compiler=ROOT/'dev/cppgm++'
source=''.join(f'int shared{i}();\n' for i in range(512))
records=[]
with tempfile.TemporaryDirectory(prefix='pa10-linkage-growth-') as directory:
    work=Path(directory)
    for count in (4,16):
        paths=[]
        for i in range(count):
            path=work/f'{i}.cpp'
            path.write_text(source+('int main(){return 0;}\n' if i==0 else ''))
            paths.append(path)
        output=work/'out.lowir'
        result=subprocess.run([compiler,'--emit-lowir','--validate-lowir','--stats','-O0','-o',output,*paths],capture_output=True,text=True,timeout=30)
        assert result.returncode==0,result.stderr
        lines=output.read_text().splitlines()
        assert sum(line.startswith('declare function ') for line in lines)==512
        assert sum(line.startswith('function ') for line in lines)==1
        phases=[json.loads(line) for line in result.stderr.splitlines()]
        records.append(dict(translation_units=count,declarations_per_unit=512,
            source_hashes=[hashlib.sha256(p.read_bytes()).hexdigest() for p in paths],
            output_sha256=hashlib.sha256(output.read_bytes()).hexdigest(),totals=phases[-1]))
print(json.dumps(dict(compiler_sha256=hashlib.sha256(compiler.read_bytes()).hexdigest(),
    flags=['--emit-lowir','--validate-lowir','--stats','-O0'],records=records),indent=2))
