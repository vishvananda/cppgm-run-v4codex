#!/usr/bin/env python3
"""Alternate direct/copy empty-list facts under both query orders."""
from pathlib import Path
import hashlib,json,subprocess,sys
ROOT=Path(__file__).resolve().parents[2]
WORK=Path(sys.argv[1]).resolve();WORK.mkdir(parents=True,exist_ok=True)
objects=Path(sys.argv[2]).resolve() if len(sys.argv)>2 else ROOT/'obj/dev'
san=len(sys.argv)>3 and sys.argv[3]=='sanitized'
def run(cmd):
 p=subprocess.run(list(map(str,cmd)),capture_output=True,text=True,cwd=ROOT,timeout=180)
 assert p.returncode==0,(cmd,p.returncode,p.stdout,p.stderr)
 assert 'Sanitizer' not in p.stderr and 'runtime error:' not in p.stderr
 return p.stdout
def sha(p):return hashlib.sha256(Path(p).read_bytes()).hexdigest()
names=run(['make','-s','-C','dev','--eval=probe-objects:\n\t@echo $(FRONTEND_OBJ_BASENAMES_cppgm++)','probe-objects']).split()
flags=['-fsanitize=address,undefined','-fno-pie','-no-pie'] if san else []
exe=WORK/'probe';source=ROOT/'student.tests/pa14/initialization-modes.cc'
run(['g++','-std=c++11','-O2',*flags,'-I'+str(ROOT/'dev/src'),source,*[objects/(n+'.o') for n in names],'-o',exe])
path=WORK/'explicit.cpp';path.write_text('struct C{explicit C(){}};')
rows=[]
for order in ['forward','reverse']:
 output=run([exe,path,order]);log=WORK/(order+'.log');log.write_text(output)
 rows.append(dict(order=order,exit_code=0,log=str(log),log_sha256=sha(log)));print(order,output,flush=True)
(WORK/'checks.json').write_text(json.dumps(dict(harness_sha256=sha(__file__),probe_source_sha256=sha(source),checks=rows),indent=2)+'\n')
