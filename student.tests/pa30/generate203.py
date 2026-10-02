#!/usr/bin/env python3
"""Differential execution controls against the host's intrinsic instructions.
Each TU isolates one public operation; runtime input blocks constant folding.
No compiler output or expected result is part of the implementation.
"""
import pathlib,re
root=pathlib.Path(__file__).resolve().parents[2]
out=root/'student.tests/pa30/generated203';out.mkdir(exist_ok=True)
types={'Void':'void','I16':'short','I32':'int','U32':'unsigned','I64':'long long','U64':'unsigned long long',
 'Float4':'float __attribute__((vector_size(16)))','Double2':'double __attribute__((vector_size(16)))',
 'Int2':'int __attribute__((vector_size(8)))','Int4':'int __attribute__((vector_size(16)))','Long1':'long long __attribute__((vector_size(8)))','Long2':'long long __attribute__((vector_size(16)))',
 'Byte8':'char __attribute__((vector_size(8)))','Byte16':'char __attribute__((vector_size(16)))','Short4':'short __attribute__((vector_size(8)))','Short8':'short __attribute__((vector_size(16)))'}
preamble='''extern "C" int printf(const char*,...);
unsigned long long hash=1469598103934665603ULL;
template<class T> void consume(T value) {
 unsigned char data[sizeof(T)];__builtin_memcpy(data,&value,sizeof(T));
 for(unsigned i=0;i<sizeof(T);++i) {hash^=data[i];hash*=1099511628211ULL;}
}
'''
def make(name,first,second,form,extra=False):
 t=types[first]
 text=preamble+'typedef '+t.replace(' __attribute__',' A __attribute__')+(';' if '__attribute__' in t else ' A;')+'\n'
 if second!='Void':
  t=types[second];text+='typedef '+t.replace(' __attribute__',' B __attribute__')+(';' if '__attribute__' in t else ' B;')+'\n'
 text+='int main(int argc,char**){ A a={};'
 if first in ['Float4','Double2']:
  text+='for(unsigned i=0;i<sizeof(a)/sizeof(a[0]);++i)a[i]=(i%2 ? -1.25 : 2.5)+(argc-1);'
 else:text+='for(unsigned i=0;i<sizeof(a)/sizeof(a[0]);++i)a[i]=('+first_lane(first)+')((i*32771u)^((unsigned)argc*0x8f01u));'
 if second!='Void':
  text+='B b={};'
  if '__attribute__' in types[second]:
   text+='for(unsigned i=0;i<sizeof(b)/sizeof(b[0]);++i)b[i]=('+first_lane(second)+')((i%2 ? -3 : 5)+argc);'
  else:text+='b=argc+1;'
 calls=[]
 if form=='Shuffle':calls=[f'{name}(a,b,{k})' for k in [0,27,255]]
 elif form=='DynamicCompare':calls=[f'{name}(a,b,{k})' for k in range(32)]
 elif form=='Insert':calls=[f'{name}(a,argc,{k})' for k in [0,1,3]]
 elif form=='Extract':calls=[f'{name}(a,{k})' for k in [0,1]]
 elif form in ['ByteShift','ShuffleOne']:calls=[f'{name}(a,{k})' for k in [0,8,64,128,2040 if form=='ByteShift' else 255]]
 else:calls=[name+'(a'+(',b' if second!='Void' else '')+')']
 text+=''.join('consume('+c+');' for c in calls)+'printf("%llu\\n",hash);return 0;}\n'
 (out/(name.removeprefix('__builtin_ia32_')+'.cpp')).write_text(text)
def first_lane(t):return types[t].split(' __attribute__')[0]
pattern=r'\{"(__builtin_ia32_\w+)",T::(\w+),T::(\w+),T::(\w+),F::(\w+),'
for name,result,first,second,form in re.findall(pattern,(root/'dev/src/support/x86_builtins.cpp').read_text()):
 if form in ['Binary','Unary','Compare','Mask','PairToFloat','FloatToPair','DynamicCompare','Shuffle','ShuffleOne','ByteShift','Insert','Extract'] and first in types and second in types:
  make(name,first,second,form)
# Include every integer builtin descriptor with independent hardware execution.
for name,op,width,lane,res,immediate in re.findall(r'\{"(__builtin_ia32_\w+)",PackedOp::(\w+),(\d+),(\d+),(\d+),(true|false)\}',(root/'dev/src/support/packed_builtins.cpp').read_text()):
 width,lane=int(width),int(lane)
 t={1:('Byte8','Byte16'),2:('Short4','Short8'),4:('Int2','Int4'),8:('Long1','Long2')}[lane][width==16]
 make(name,t,'I32' if immediate=='true' else t,'ShuffleOne' if immediate=='true' else 'Binary')
print(len(list(out.glob('*.cpp'))),'differential sources')
