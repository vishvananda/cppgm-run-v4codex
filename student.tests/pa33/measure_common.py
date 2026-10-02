#!/usr/bin/env python3
"""Use the unchanged inherited benchmarks with frozen same-level A/B compilers."""
import json,os,pathlib,subprocess
root=pathlib.Path(__file__).resolve().parents[2]
art=pathlib.Path(os.environ['RALPH_ARTIFACT_DIR'])/'pa33-219'
rows=[]
for lane,script,level in [('common-o0','common_levels.py','0'),('common-o2','common_levels.py','2'),('selfhost','selfhost_performance.py',None)]:
    cmd=['python3',str(root/'student.tests/pa32'/script),str(art/lane),str(art/'entry/cppgm++'),str(art/'final/cppgm++')]
    env=dict(os.environ,PERF_CPU='2')
    if level is not None:env.update(PA32_BASE_LEVEL=level,PA32_FINAL_LEVEL=level)
    with (art/(lane+'.log')).open('w') as log:p=subprocess.run(cmd,cwd=root,env=env,stdout=log,stderr=subprocess.STDOUT)
    rows.append(dict(lane=lane,command=cmd,status=p.returncode,environment={k:env[k] for k in ['PERF_CPU','PA32_BASE_LEVEL','PA32_FINAL_LEVEL'] if k in env}))
    (art/'common-commands.json').write_text(json.dumps(rows,indent=2)+'\n')
    print(lane,p.returncode,flush=True)
    assert p.returncode==0,lane
