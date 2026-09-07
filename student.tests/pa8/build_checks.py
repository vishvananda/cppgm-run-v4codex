#!/usr/bin/env python3
"""Build personal API and standalone sanitizer tool outside the repository."""
from pathlib import Path
import subprocess, sys
ROOT=Path(__file__).resolve().parents[2]
sys.path.insert(0,str(ROOT/'student.tests/pa8'))
out=Path(sys.argv[1] if len(sys.argv)>1 else '/tmp/pa8-sanitize')
out.mkdir(parents=True,exist_ok=True)
sources=[ROOT/'dev/src/preprocess/source.cpp',ROOT/'dev/src/preprocess/identifier_table.cpp',*sorted((ROOT/'dev/src/lowir').glob('*.cpp'))]
flags=['g++','-std=gnu++11','-Wall','-O1','-g','-fsanitize=address,undefined','-fno-omit-frame-pointer','-fno-pie','-no-pie','-I'+str(ROOT/'dev/src')]
# Compile the shared implementation once, then link two entry points.
objects=[]
for n,source in enumerate(sources):
    obj=out/f'{n}.o';objects.append(obj)
    subprocess.run([*flags,'-c',source,'-o',obj],check=True)
for source,name in [(ROOT/'student.tests/pa8/check_api.cpp','api'),(ROOT/'dev/lowir.cpp','lowir')]:
    subprocess.run([*flags,source,*objects,'-o',out/name],check=True)
subprocess.run([out/'api'],check=True)
subprocess.run([sys.executable,ROOT/'student.tests/pa8/check.py',out/'lowir'],check=True)
