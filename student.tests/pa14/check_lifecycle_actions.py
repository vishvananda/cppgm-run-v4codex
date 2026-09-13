#!/usr/bin/env python3
"""Actions must precede omission facts; failed actions/transfers stay terminal."""
from pathlib import Path
import json,subprocess,sys
ROOT=Path(__file__).resolve().parents[2]
WORK=Path(sys.argv[1]).resolve();WORK.mkdir(parents=True,exist_ok=True)
objects=Path(sys.argv[2]).resolve() if len(sys.argv)>2 else ROOT/'obj/dev'
san=len(sys.argv)>3 and sys.argv[3]=='sanitized'
def run(cmd):
 p=subprocess.run(list(map(str,cmd)),cwd=ROOT,capture_output=True,text=True,timeout=180)
 assert p.returncode==0,(cmd,p.returncode,p.stdout,p.stderr)
 return p.stdout
names=run(['make','-s','-C','dev','--eval=probe-objects:\n\t@echo $(FRONTEND_OBJ_BASENAMES_cppgm++)','probe-objects']).split()
flags=['-fsanitize=address,undefined','-fno-pie','-no-pie'] if san else []
for kind,macro in [('actions','EXPECT_ACTION_PUBLICATION'),('transfers','EXPECT_TRANSFER_FACTS')]:
 run(['g++','-std=c++11','-O2','-D'+macro,*flags,'-I'+str(ROOT/'dev/src'),ROOT/f'student.tests/pa14/lifecycle-{kind}.cc',*[objects/(n+'.o') for n in names],'-o',WORK/kind])
CASES={
 'ctor': ('actions','ctor','struct Target{int field=3;Target()=default;};Target object;'),
 'dtor': ('actions','dtor','int count;struct Member{~Member(){++count;}};struct Target{Member field;~Target()=default;};Target object;'),
 'ctor-failure': ('actions','ctor-failure','struct Target{int& field;Target()=default;};Target object;'),
 'dtor-failure': ('actions','dtor-failure','class Member{~Member(){}};struct Target{Member field;~Target()=default;};extern Target object;void destroy(){object.~Target();}'),
 'transfer-failure': ('transfers','failure','struct Target{Target(const Target&)=default;Target field;};'),
 'transfer-deleted': ('transfers','deleted','struct Target{const int field;Target&operator=(const Target&)=default;};'),
}
rows=[]
for name,(probe,mode,source) in CASES.items():
 path=WORK/(name+'.cpp');path.write_text(source)
 text=run([WORK/probe,path,mode]);log=WORK/(name+'.log');log.write_text(text)
 rows.append(dict(name=name,probe=probe,mode=mode,source=source,log=str(log),exit_code=0))
 print(name,'PASS',flush=True)
(WORK/'checks.json').write_text(json.dumps(rows,indent=2)+'\n')
