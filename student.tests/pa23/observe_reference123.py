#!/usr/bin/env python3
"""Reproduce the fixed-offset virtual-reference defect; never rewrite an oracle."""
from pathlib import Path
import hashlib,json,subprocess,sys
ROOT=Path(__file__).resolve().parents[2];WORK=Path(sys.argv[1]).resolve();WORK.mkdir(parents=True,exist_ok=True)
source=ROOT/'student.tests/pa23/reducers/nonpoly-virtual-reference.cpp';ir=WORK/'reference.lowir';obj=WORK/'reference.o';exe=WORK/'reference.exe'
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
commands=[[str(ROOT/'dev/cppgm++-ref'),'--emit-lowir','-O0','-o',str(ir),str(source)],[str(ROOT/'dev/cppgm++-ref'),'-c','-O0','-o',str(obj),str(ir)],['g++','-no-pie',str(obj),'-o',str(exe)],[str(exe)]]
rows=[]
for i,cmd in enumerate(commands):
 p=subprocess.run(cmd,capture_output=True,text=True);rows.append(dict(command=cmd,exit=p.returncode,stdout=p.stdout,stderr=p.stderr));assert p.returncode==(1 if i==3 else 0),rows[-1]
print(json.dumps(dict(bundle_revision='c2f713cd70d06170632bfde3e75dd6fe1aa44d98',source=str(source.relative_to(ROOT)),source_sha256=sha(source),expected_source_exit=0,reference_exit=1,reference_lowir_sha256=sha(ir),reference_lowir=ir.read_text(),commands=rows,oracle_changes=[]),indent=2))
