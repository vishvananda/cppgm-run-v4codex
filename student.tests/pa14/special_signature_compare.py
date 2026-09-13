"""Use the unchanged course LowIR comparator on frozen benchmark outputs."""
from pathlib import Path
import hashlib,os,shutil,subprocess
ROOT=Path(__file__).resolve().parents[2]
def sha(path):return hashlib.sha256(Path(path).read_bytes()).hexdigest()
def compare(source,outputs,directory,name):
 directory=Path(directory);directory.mkdir(parents=True,exist_ok=True)
 base=directory/name;test=base.with_suffix('.t');shutil.copyfile(source,test)
 copies=[]
 for b,out in enumerate(outputs):
  copy=Path(str(base)+f'.{b}.lowir');shutil.copyfile(out['path'],copy)
  Path(str(copy)+'.exit_status').write_text('EXIT_SUCCESS\n')
  copies.append(dict(path=str(copy),sha256=sha(copy)))
 canonical=[Path(str(base)+f'.{b}.canonical') for b in (0,1)]
 command=['perl','-I'+str(ROOT/'scripts'),str(ROOT/'student.tests/pa14/special_signature_compare.pl'),str(ROOT/'scripts/compare_results_common.pl'),*(row['path'] for row in copies),str(test),*map(str,canonical)]
 env=dict(os.environ,CPPGM_CHECK_MODE='1',KEEP_GOING='0')
 result=subprocess.run(command,capture_output=True,text=True,cwd=ROOT/'pa14',env=env,timeout=300)
 log=Path(str(base)+'.log');log.write_text(result.stdout+result.stderr)
 assert result.returncode==0,(name,'comparison failed; see '+str(log))
 return dict(command=command,cwd=str(ROOT/'pa14'),exit_code=0,log=str(log),log_sha256=sha(log),
  source_path=str(test),source_sha256=sha(test),copies=copies,canonical=[dict(path=str(p),sha256=sha(p)) for p in canonical])
