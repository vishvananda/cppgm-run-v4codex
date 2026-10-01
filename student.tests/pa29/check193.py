#!/usr/bin/env python3
"""Explicit ABI-definition controls; all sources are personal, never auto-discovered."""
import hashlib,json,pathlib,subprocess,sys
root=pathlib.Path(__file__).resolve().parents[2];src=root/'student.tests/pa29/source193'
out=pathlib.Path(sys.argv[1]).resolve();out.mkdir(parents=True,exist_ok=True)
cc=pathlib.Path(sys.argv[2]).resolve() if len(sys.argv)>2 else root/'dev/cppgm++'
rows=[];failures=[];properties=[]
def save():
 (out/'checks.json').write_text(json.dumps(dict(compiler_sha256=hashlib.sha256(cc.read_bytes()).hexdigest(),commands=rows,properties=properties,failures=failures),indent=2)+'\n')
def check(value,description):
 properties.append(dict(description=description,passed=bool(value)))
 if not value:failures.append(description)
 save()
def run(args,ok=True):
 args=list(map(str,args));p=subprocess.run(args,cwd=root,capture_output=True,text=True,timeout=90)
 rows.append(dict(command=args,status=p.returncode,expected_success=ok,stdout=p.stdout,stderr=p.stderr))
 if (p.returncode==0)!=ok:failures.append(rows[-1])
 save();return p
names=[p.stem for p in sorted(src.glob('*.cpp')) if not p.name.endswith(('.reject.cpp','-host.cpp')) and p.stem!='extern']
for name in names:
 source=src/(name+'.cpp');ref=out/(name+'.host.o')
 p=run(['clang++','-std=c++11','-O0','-c',source,'-o',ref])
 if p.returncode:continue
 expected={s.split()[-1] for s in run(['nm',ref]).stdout.splitlines() if '_Z' in s}
 for level in ['-O0','-O2']:
  obj=out/(name+level+'.o');exe=out/(name+level)
  p=run([cc,'-std=c++11',level,'-c',source,'-o',obj])
  if p.returncode:continue
  symbols={s.split()[-1] for s in run(['nm',obj]).stdout.splitlines() if '_Z' in s}
  check(symbols==expected,name+level+' exact ABI names match observed hosted policy')
  if not name.endswith('-declaration'):
   p=run(['g++',obj,'-o',exe])
   if not p.returncode:run([exe])
for source in sorted(src.glob('*.reject.cpp')):
 for compiler in [cc,'clang++']:
  run([compiler,'-std=c++11','-c',source,'-o',out/'reject.o'],False)
for level in ['-O0','-O2']:
 run([cc,level,'-c',src/'extern.cpp','-o',out/'extern.o'])
 run(['clang++','-std=c++11','-c',src/'extern-host.cpp','-o',out/'peer.o'])
 p=run(['g++',out/'extern.o',out/'peer.o','-o',out/'extern'])
 if not p.returncode:run([out/'extern'])
# Roundtrip explicit LowIR and inspect the same ABI names and relocations.
objects=run(['make','-s','-C',root/'dev','--eval','print-objects: ; @echo $(call frontend_objs,cppgm++) $(call frontend_objs,lowir)','print-objects']).stdout.split()
objects=list(dict.fromkeys(str((root/'dev'/p).resolve()) for p in objects))
p=run(['g++','-std=c++11','-I'+str(root/'dev/src'),root/'student.tests/pa29/source192/lowir-host.cpp',*objects,'-o',out/'lowir-host'])
if not p.returncode:
 for name in ['nested-outside','ordering','overloads','specialization','explicit-member']:
  lir=out/(name+'.lowir');canonical=out/(name+'.canonical.lowir')
  run([cc,'-c','--emit-lowir','--validate-lowir',src/(name+'.cpp'),'-o',lir])
  run([root/'dev/lowir',lir,'-o',canonical])
  p=run([out/'lowir-host',canonical,out/'adapted.o'])
  if not p.returncode:
   syms=lambda obj:{s.split()[-1] for s in run(['nm',obj]).stdout.splitlines() if '_Z' in s}
   check(syms(out/'adapted.o')==syms(out/(name+'-O0.o')),name+' direct/adapter ABI identity')
   p=run(['g++',out/'adapted.o','-o',out/'adapted'])
   if not p.returncode:run([out/'adapted'])
  run([root/'dev/lowir2native','--dump-machine-ir',out/(name+'.mir'),canonical])
for args in [['readelf','-SWsrg',out/'overloads-O0.o'],['readelf','--debug-dump=frames',out/'overloads-O0.o'],['objdump','-dr',out/'overloads-O0.o']]:run(args)
p=run([cc,'--stats','-O0','-c',src/'overloads.cpp','-o',out/'stats.o'])
check((out/'stats.o').read_bytes()==(out/'overloads-O0.o').read_bytes(),'telemetry does not change object bytes')
print(len(rows),'commands,',len(properties),'properties,',len(failures),'failures')
for f in failures:print(f)
sys.exit(bool(failures))
