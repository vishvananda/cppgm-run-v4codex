#!/usr/bin/env python3
"""Explicit PA16 scalar execution controls, including mutation and bounded failures."""
from pathlib import Path
import subprocess,sys,tempfile
ROOT=Path(__file__).resolve().parents[2]
CC=Path(sys.argv[1]).resolve() if len(sys.argv)>1 else ROOT/'dev/cppgm++'
GOOD = {'loop_cache': 'constexpr int f(int n){int s=0;for(int i=0;i<n;++i)s+=i;return '
               's;}static_assert(f(6)==15,"");static_assert(f(3)==3,"");static_assert(f(6)==15,"");int '
               'main(){return f(4)-6;}',
 'parameter_mutation': 'constexpr int f(int x){x+=3;return '
                       'x;}static_assert(f(1)==4,"");static_assert(f(7)==10,"");int main(){return '
                       'f(4)-7;}',
 'compound_operators': 'constexpr int f(){int '
                       'a=9;a*=4;a/=3;a%=7;a|=8;a&=13;a^=3;a<<=2;a>>=1;a-=1;return '
                       'a;}static_assert(f()==27,"");int main(){return f()-27;}',
 'postfix': 'constexpr int f(){int i=2;int a=i++;int b=--i;return '
            'a*10+b;}static_assert(f()==22,"");int main(){return f()-22;}',
 'nested_loop': 'constexpr int f(){int s=0;for(int i=0;i<4;++i){if(i==2)continue;int '
                'j=0;while(j<4){++j;if(j==3)break;s+=i+j;}}return s;}static_assert(f()==17,"");int '
                'main(){return f()-17;}',
 'do_continue': 'constexpr int f(){int n=0;do{++n;continue;}while(n<3);return '
                'n;}static_assert(f()==3,"");int main(){return f()-3;}',
 'condition_scopes': 'constexpr int f(int x){int s=0;for(int i=x;int j=i;i-=1){s+=j;}if(int '
                     'y=s-6)return y;else return '
                     '19;}static_assert(f(3)==19,"");static_assert(f(2)==-3,"");int main(){return '
                     'f(2)+3;}',
 'nested_calls': 'constexpr int f(int x){x+=2;return x;}constexpr int g(int x){int s=0;for(int '
                 'i=0;i<x;++i)s+=f(i);return s+x;}static_assert(g(4)==18,"");int main(){return '
                 'g(5)-25;}',
 'shadowing': 'constexpr int f(int x){int s=x;{int x=3;s+=x;}return '
              'x+s;}static_assert(f(7)==17,"");int main(){return f(2)-7;}',
 'zero_init': 'constexpr int f(){int x{};return x+3;}static_assert(f()==3,"");int main(){return '
              'f()-3;}',
 'short_circuit_effect': 'constexpr int f(){int x=2;false && ++x;true || ++x;return '
                         'x;}static_assert(f()==2,"");int main(){return f()-2;}',
 'unsigned_wrap': 'constexpr unsigned f(unsigned x){++x;return '
                  'x;}static_assert(f(4294967295u)==0,"");int main(){return f(4294967295u);}',
 'skipped_bad_declaration': 'int get(){return 4;}constexpr int f(int n){if(n){int x=get();return '
                            'x;}return 7;}static_assert(f(0)==7,"");int main(){return 0;}'}
BAD = {'unused_initializer': 'int get();constexpr int f(){int x=get();return '
                       '7;}static_assert(f()==7,"");',
 'self_init': 'constexpr int f(){int x=x;return 7;}static_assert(f()==7,"");',
 'write_global': 'int n;constexpr int f(){n=3;return n;}static_assert(f()==3,"");',
 'volatile_local': 'constexpr int f(){volatile int x=3;return x;}static_assert(f()==3,"");',
 'volatile_parameter': 'constexpr int f(volatile int x){return x;}static_assert(f(3)==3,"");',
 'infinite_loop': 'constexpr int f(){while(true){}return 0;}static_assert(f()==0,"");',
 'local_static': 'constexpr int f(){static int x=1;return x;}static_assert(f()==1,"");',
 'compound_overflow': 'constexpr int f(int x){x+=1;return x;}static_assert(f(2147483647)==0,"");',
 'compound_zero_divide': 'constexpr int f(int x){x/=0;return x;}static_assert(f(3)==0,"");',
 'nonconstant_for_initializer': 'int get();constexpr int f(){for(int x=get();false;){}return '
                                '2;}static_assert(f()==2,"");'}
GOOD.update({
 'function_then_tag': 'constexpr int item(int x){return x+2;}struct item{static const int n=5;};static_assert(::item(3)==5, "");static_assert(item::n==5, "");int main(){return item(4)-6;}',
 'tag_then_function': 'struct item{static const int n=5;};constexpr int item(int x){return x+2;}static_assert(::item(3)==5, "");static_assert(item::n==5, "");int main(){return item(4)-6;}',
 'enum_then_value': 'enum item { one=1 };int item=3;static_assert(item::one==1, "");int main(){return item-3;}',
 'value_then_enum': 'int item=3;enum item { one=1 };static_assert(item::one==1, "");int main(){return item-3;}',
})
GOOD.update({
 'nested_forward_class': 'class B{int n;public:struct D;};struct B::D:B{int f(){return n=0;}};int main(){B::D d;return d.f();}',
 'nested_template_definition': 'template<class... T>struct A{struct B;};template<class... U>struct A<U...>::B{static_assert(sizeof...(U)==2, "");};int main(){return sizeof(A<int,long>::B)==1?0:1;}',
})
GOOD.update({
 'empty_scalar_initializers': 'constexpr int x{};constexpr int f(){return {};}static_assert(x==0 && f()==0, "");int main(){return f();}',
 'empty_argument': 'constexpr int f(int x){return x+2;}static_assert(f({})==2, "");int main(){return f({})-2;}',
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
