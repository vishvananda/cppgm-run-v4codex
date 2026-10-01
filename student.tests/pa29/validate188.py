#!/usr/bin/env python3
"""Run shared-report checks serially and record every command's status."""
import hashlib,json,pathlib,subprocess,sys
root=pathlib.Path(__file__).resolve().parents[2]
out=pathlib.Path(sys.argv[1]).resolve();out.mkdir(parents=True,exist_ok=True)
rows=[]
checks=[
 ('prior-final',['bash','-c','n=29; if [ "$n" -le 1 ]; then echo \'===== ALL TESTS PASSED SUCCESSFULLY! (0/0) =====\'; else make test-report-through-pa$((n - 1)); fi'],0),
 ('stage-final',['make','test-pa29'],2),
 ('through-final',['make','test-report-through-pa29'],2),
 ('file-audit-final',['perl','scripts/cppgm_file_audit.pl','--stage','pa29','--paths','dev/src'],0),
]
for name,cmd,status in checks:
 path=out/(name+'.log')
 with path.open('w') as f:p=subprocess.run(cmd,cwd=root,stdout=f,stderr=subprocess.STDOUT,timeout=300)
 rows.append(dict(command=cmd,status=p.returncode,path=str(path),sha256=hashlib.sha256(path.read_bytes()).hexdigest()))
 (out/'validation.json').write_text(json.dumps(rows,indent=2)+'\n')
 print(name,p.returncode,flush=True);assert p.returncode==status,(name,p.returncode)
