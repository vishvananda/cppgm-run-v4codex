#!/usr/bin/env python3
"""Explicit PA16 floating value, conversion and storage controls."""
from pathlib import Path
import subprocess,sys,tempfile
ROOT=Path(__file__).resolve().parents[2]
CC=Path(sys.argv[1]).resolve() if len(sys.argv)>1 else ROOT/'dev/cppgm++'
GOOD = {'floating_frames': 'constexpr double f(double x){return '
                    'x/2.0;}static_assert(f(8)==4,"");static_assert(f(3)==1.5,"");static_assert(f(8)==4,"");int '
                    'main(){return f(9)==4.5?0:1;}',
 'floating_mutation': 'constexpr double f(double x){double y=x;for(int '
                      'i=0;i<3;++i)y+=0.5;if(y<0)return -y;return '
                      'y;}static_assert(f(-5)==3.5,"");static_assert(f(1)==2.5,"");int '
                      'main(){return f(2)==3.5?0:1;}',
 'precision': 'constexpr float f(float x){return '
              'x+1.0f;}static_assert(f(16777216.0f)==16777216.0f,"");constexpr float '
              'v=16777217.0;static_assert(v==16777216.0f,"");int main(){return '
              'v==16777216.0f?0:1;}',
 'intermediate_precision': 'constexpr float '
                           'x=(16777216.0f+1.0f)-16777216.0f;static_assert(x==0.0f,"");constexpr '
                           'double '
                           'y=(9007199254740992.0+1.0)-9007199254740992.0;static_assert(y==0,"");int '
                           'main(){return 0;}',
 'wide_unsigned': 'static_assert(18446744073709551615ULL>1.0e19L,"");constexpr long double '
                  'v=18446744073709551615ULL;static_assert(v==18446744073709551615.0L,"");int '
                  'main(){return 0;}',
 'truth': 'static_assert(0.25,"");static_assert(!0.0,"");static_assert(!(-0.0),"");static_assert(0.5&&0.25,"");static_assert(!(0.0||-0.0),"");static_assert((0.0?2:3)==3,"");static_assert((0.5?2:3)==2,"");int '
          'main(){return 0;}',
 'casts': 'constexpr int f(double v){return '
          'int(v);}static_assert(f(-2.9)==-2,"");static_assert(f(2.9)==2,"");static_assert(static_cast<unsigned>(-0.9)==0,"");static_assert(static_cast<bool>(0.25),"");static_assert(double(3)==3.0,"");int '
          'main(){return 0;}',
 'float_templates': 'template<class T>constexpr T f(T v){return v/2;}template<int N>struct '
                    'C{static const int '
                    'n=N;};static_assert(C<int(f(8.0))>::n==4,"");static_assert(C<int(f(3.0))>::n==1,"");int '
                    'main(){return 0;}',
 'floating_defaults': 'constexpr double f(double x=1.25){return '
                      'x+0.5;}static_assert(f()==1.75,"");int main(){return f()==1.75?0:1;}',
 'floating_static_member': 'struct C{static constexpr double n=1.25;};constexpr double '
                           'C::n;static_assert(C::n==1.25,"");int main(){return C::n==1.25?0:1;}',
 'zero_init': 'constexpr double f(){double x{};return x;}static_assert(f()==0,"");int '
              'main(){return f()==0?0:1;}',
 'return_rounding': 'constexpr float f(){return 16777217.0;}static_assert(f()==16777216.0f,"");int '
                    'main(){return f()==16777216.0f?0:1;}',
 'negative_zero': 'constexpr double f(double n){return '
                  '-n;}static_assert(f(0.0)==0.0,"");static_assert(f(-0.0)==0.0,"");constexpr '
                  'double minus=f(0.0);int main(){return 0;}'}
BAD = {'nonconstant_float': 'double get();constexpr double x=get();',
 'const_float_not_constexpr': 'const double x=1.25;static_assert(x==1.25,"");',
 'float_missing_initializer': 'constexpr double x;',
 'float_overflow': 'constexpr float x=3.4e38f*2;',
 'float_divzero': 'constexpr double x=2.0/0.0;',
 'float_to_int_overflow': 'constexpr int x=static_cast<int>(2147483648.0);',
 'float_to_unsigned_overflow': 'constexpr unsigned long long x=static_cast<unsigned long '
                               'long>(18446744073709551616.0L);',
 'float_to_unsigned_negative': 'constexpr unsigned x=static_cast<unsigned>(-1.0);',
 'float_unused_bad_initializer': 'double get();constexpr int f(){double x=get();return '
                                 '1;}static_assert(f()==1,"");'}
GOOD.update({
 'constant_arrays': 'constexpr double a[4]={1.5,2.5};static_assert(a[0]==1.5 && a[1]==2.5 && a[3]==0, "");int main(){return a[1]==2.5?0:1;}',
 'nested_arrays': 'constexpr int a[2][3]={{1,2},{4,5,6}};constexpr int f(){int i=0;return (a[i++][0],i);}static_assert(f()==1, "");static_assert(a[1][2]==6, "");int main(){return 0;}',
 'array_parameter_index': 'constexpr double a[]={1.5,2.5};constexpr double f(int i){return a[i];}static_assert(f(0)==1.5 && f(1)==2.5, "");int main(){return 0;}',
  'string_array': "constexpr char a[7]=\"hi\";static_assert(a[0]=='h' && a[6]==0, \"\");int main(){return 0;}",
 'zero_range': 'constexpr int a[1000000]={3};static_assert(a[999999]==0, "");int main(){return 0;}',
})
BAD.update({
 'array_one_past': 'constexpr int a[]={1,2};static_assert(a[2]==0, "");',
 'array_negative': 'constexpr int a[]={1,2};static_assert(a[-1]==0, "");',
 'nonconstexpr_array': 'int a[]={1,2};static_assert(a[0]==1, "");',
 'duplicate_initializer': 'struct A{static constexpr double a=1.5;};constexpr double A::a=2.5;',
})
failed=[]
with tempfile.TemporaryDirectory(prefix='pa16-scalar-') as td:
 for name,source in {**GOOD,**BAD}.items():
  src=Path(td)/(name+'.cpp');ir=src.with_suffix('.lowir');exe=src.with_suffix('.exe');src.write_text(source)
  r=subprocess.run([CC,'--emit-lowir','-O0','--validate-lowir','-o',ir,src],capture_output=True,text=True,timeout=20)
  okay=(r.returncode==0)==(name in GOOD)
  if okay and name=='storage_two_classes':okay=sum(line.startswith('global ') for line in ir.read_text().splitlines())==5
  if okay and name=='storage_later_definition':okay=sum(line.startswith('global ') for line in ir.read_text().splitlines())==2
  if okay and name in GOOD:
   r=subprocess.run([ROOT/'dev/lowir2native-ref','-O0','-o',exe,ir],capture_output=True,text=True);okay=r.returncode==0
   if okay:r=subprocess.run([exe],capture_output=True,text=True,timeout=10);okay=r.returncode==0
  print(name,'PASS' if okay else 'FAIL',r.returncode,r.stderr.strip(),flush=True)
  if not okay:failed.append(name)
assert not failed,failed
print(f'{len(GOOD)} native and {len(BAD)} rejection controls passed')
