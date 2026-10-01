#!/usr/bin/env python3
"""Views, source-string demand, telemetry equivalence and native relocation checks."""
import hashlib,json,pathlib,re,subprocess,sys
root=pathlib.Path(__file__).resolve().parents[2]
out=pathlib.Path(sys.argv[1]).resolve();out.mkdir(parents=True,exist_ok=True)
cc=root/'dev/cppgm++';rows=[]
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def run(args):
 p=subprocess.run(list(map(str,args)),cwd=root,capture_output=True,text=True,timeout=90)
 rows.append(dict(command=list(map(str,args)),status=p.returncode,stdout=p.stdout,stderr=p.stderr));assert p.returncode==0,rows[-1]
 return p
for name in ['locations','queries','strings','noexcept']:
 source=root/('student.tests/pa29/source175/'+name+'.cpp')
 # The PA7 semantic renderer does not instantiate the NTTP class in queries.
 # Its template behavior is checked by the production path and LowIR adapter.
 for mode in (['--emit-ast','--emit-lowir'] if name=='queries' else ['--emit-ast','--emit-semantics','--emit-lowir']):
  dest=out/(name+mode+'.txt');run([cc,mode,source,'-o',dest])
  if mode=='--emit-lowir':
   run([root/'dev/lowir',dest,'-o',out/(name+'.roundtrip')])
   text=dest.read_text();assert not re.search(r'call[^\n]*@__builtin_(LINE|FILE|FUNCTION|COLUMN)\(',text)
   if name=='noexcept':
    body=re.search(r'function @intrinsic_only\([^}]*\}',text).group();assert 'landing' not in body and 'terminate' not in body and 'call ' not in body
 plain=out/(name+'.o');stats=out/(name+'.stats.o')
 run([cc,'-c','-O0',source,'-o',plain]);run([cc,'-c','-O0','--stats',source,'-o',stats]);assert sha(plain)==sha(stats)
 undefined=run(['nm','-u',plain]).stdout;assert not re.search(r'__builtin_(LINE|FILE|FUNCTION|COLUMN)',undefined)
 run(['readelf','-r',plain]);run(['objdump','-dr',plain])
line=out/'line-only.cpp';line.write_text('constexpr int line(int n=__builtin_LINE()){return n;} int main(){return line()==1?0:1;}\n')
text=out/'line-only.lowir';run([cc,'--emit-lowir',line,'-o',text]);assert '@__source_string_' not in text.read_text()
read=out/'read-only.cpp';read.write_text('int n=__builtin_FILE()[0]; int main(){return n==0;}\n')
text=out/'read-only.lowir';run([cc,'--emit-lowir',read,'-o',text]);assert '@__source_string_' not in text.read_text()
(out/'inspection.json').write_text(json.dumps(dict(compiler_sha256=sha(cc),checks=rows),indent=2)+'\n')
print('inspection passed:',len(rows),'commands plus source/telemetry/relocation assertions')
