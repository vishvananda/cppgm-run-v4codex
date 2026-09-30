#!/usr/bin/env python3
"""Host TLS boundaries, optional initialization hooks and per-thread storage."""
import hashlib,json,pathlib,subprocess,sys
root=pathlib.Path(__file__).resolve().parents[2]
out=pathlib.Path(sys.argv[1]).resolve();out.mkdir(parents=True,exist_ok=True)
compiler=root/'dev/cppgm++'; records=[]
def run(args):
 p=subprocess.run(list(map(str,args)),capture_output=True,timeout=30)
 records.append(dict(command=list(map(str,args)),status=p.returncode,stdout=p.stdout.decode(),stderr=p.stderr.decode()))
 assert p.returncode==0,records[-1]
 return p.stdout.decode()
def source(name,text):
 p=out/(name+'.cpp');p.write_text(text);return p
hostmain='''#include <pthread.h>
namespace item {extern thread_local int value;}
extern "C" int read_value();
void* thread(void*) {if(read_value()!=7)return (void*)1;item::value=19;return read_value()==19?0:(void*)2;}
int main(){if(read_value()!=7)return 1;item::value=11;
pthread_t t;if(pthread_create(&t,0,thread,0))return 2;void* r;
if(pthread_join(t,&r)||r)return 3;return read_value()==11?0:4;}
'''
main=source('host-main',hostmain)
for dynamic in (False,True):
 definition='int seed(){return 7;} namespace item {thread_local int value='+('seed()' if dynamic else '7')+';}\n'
 provider=source('provider',definition)
 reader=source('reader','namespace item {extern thread_local int value;} extern "C" int read_value(){return item::value;}\n')
 for direction in ('import','export'):
  for level in ('-O0','-O2'):
   for name,p in [('provider',provider),('reader',reader)]:
    binary=compiler if (name=='reader')==(direction=='import') else 'g++'
    run([binary,level,'-std=c++11','-c',p,'-o',out/(name+'.o')])
   run(['g++','-std=c++11',main,out/'reader.o',out/'provider.o','-pthread','-o',out/'program'])
   run([out/'program'])
# A host-built dynamic initializer must also run when both imported wrapper
# and exporting wrapper are present, in either linker input order.
provider=source('student-provider','int seed(){return 7;} namespace item {thread_local int value=seed();}\n')
reader=source('student-reader','namespace item {extern thread_local int value;} extern "C" int read_value(){return item::value;}\n')
run([compiler,'-c',provider,'-o',out/'provider.o']);run([compiler,'-c',reader,'-o',out/'reader.o'])
for objects in ((out/'provider.o',out/'reader.o'),(out/'reader.o',out/'provider.o')):
 run(['g++',main,*objects,'-pthread','-o',out/'both']);run([out/'both'])
(out/'tls-controls.json').write_text(json.dumps(dict(binary_sha256=hashlib.sha256(compiler.read_bytes()).hexdigest(),commands=records),indent=2)+'\n')
print('PA27 TLS controls PASS:',len(records),'commands')
