#!/usr/bin/env python3
"""Sequential frozen measurements of all accumulated owners and common controls."""
import json,os,pathlib,subprocess,sys
root=pathlib.Path(__file__).resolve().parents[2]
out=pathlib.Path(sys.argv[1]).resolve();out.mkdir(parents=True,exist_ok=True)
art=pathlib.Path(os.environ['RALPH_ARTIFACT_DIR']);after=out/'final-cppgm++'
assert after.exists()
commands=[]
def measure(name,script,before,extra=None):
 env=dict(os.environ,PERF_CPU='2');env.update(extra or {})
 command=['python3',str(root/'student.tests/pa32'/script),str(out/name),str(before),str(after)]
 with (out/(name+'.log')).open('w') as log:r=subprocess.run(command,env=env,stdout=log,stderr=subprocess.STDOUT,cwd=root)
 commands.append(dict(name=name,command=command,env={k:env[k] for k in ['PERF_CPU',*(extra or {})]},exit_code=r.returncode))
 (out/'performance-commands.json').write_text(json.dumps(commands,indent=2)+'\n')
 assert r.returncode==0,(name,r.returncode)
 print(name,'PASS',flush=True)
measure('objects','objects_performance.py',art/'pa32-211/before-cppgm')
measure('loops','loops_performance.py',art/'pa32-212/A')
measure('memory','memory_performance.py',art/'pa32-213/A')
for level in [0,1,3]:measure('common-o'+str(level),'common_levels.py',out/'entry-cppgm++',dict(PA32_BASE_LEVEL=str(level),PA32_FINAL_LEVEL=str(level)))
measure('selfhost','selfhost_performance.py',out/'entry-cppgm++')
