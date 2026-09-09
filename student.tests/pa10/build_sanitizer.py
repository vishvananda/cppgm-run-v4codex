#!/usr/bin/env python3
"""Build the actual driver and registered phases with ASan/UBSan outside checkout."""
from concurrent.futures import ThreadPoolExecutor
from pathlib import Path
import subprocess
import sys
ROOT=Path(__file__).resolve().parents[2]
work=Path(sys.argv[1] if len(sys.argv)>1 else '/tmp/pa10-sanitize').resolve()
work.mkdir(parents=True,exist_ok=True)
names=subprocess.check_output(['make','-s','--no-print-directory','-C',ROOT/'dev',
    '--eval=pa10-print-sources:;@echo $(FRONTEND_OBJ_BASENAMES_cppgm++)','pa10-print-sources'],text=True).split()
sources=[ROOT/'dev/cppgm++.cpp']+[ROOT/'dev/src'/f'{n}.cpp' for n in dict.fromkeys(names)]
flags=['-std=gnu++11','-O1','-g','-fsanitize=address,undefined','-fno-omit-frame-pointer','-fno-pie','-no-pie','-I'+str(ROOT/'dev/src')]
def compile_one(pair):
    index,source=pair;obj=work/f'{index}.o'
    subprocess.run(['g++',*flags,'-c',source,'-o',obj],check=True)
    return obj
with ThreadPoolExecutor(max_workers=4) as pool:
    objects=list(pool.map(compile_one,enumerate(sources)))
subprocess.run(['g++',*flags,*objects,'-o',work/'cppgm-sanitize'],check=True)
print(work/'cppgm-sanitize')
