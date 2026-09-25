#!/usr/bin/env python3
"""Runtime named-constant conversion summaries, effects and retained uses: CC WORK."""
from pathlib import Path
import sys
import ordering_controls as runner
runner.GOOD={}
runner.BAD={}
for typename,value,expected in [('bool','true','1'),('int','7','7'),('long','-9','-9'),('double','7.5','7.5')]:
 decl=f'static constexpr {typename} value={value};'
 base=f'struct X{{{decl} operator {typename}()const{{return value;}}}};'
 runner.GOOD['scalar_'+typename]=base+f'int main(){{X x;{typename} v=x;return v!={expected};}}'
 runner.GOOD['explicit_'+typename]=base+f'int main(){{X x;return x.operator {typename}()!={expected};}}'
 runner.GOOD['address_'+typename]=base+f'int main(){{X x;{typename}(X::*p)()const=&X::operator {typename};return (x.*p)()!={expected};}}'
 runner.GOOD['mixed_'+typename]=base+f'int main(){{X x;{typename} v=x;return v!={expected}||x.operator {typename}()!={expected};}}'
 runner.GOOD['template_'+typename]=f'template<class T>struct X{{{decl} operator T()const{{return value;}}}};int main(){{X<{typename}> x;{typename} v=x;return v!={expected};}}'
base='struct X{static const bool value=true;operator bool()const{return value;}};'
runner.GOOD.update({
 'conditional':base+'int main(){X x;return x?0:1;}',
 'false_conditional':'struct X{static const bool value=false;operator bool()const{return value;}};int main(){X x;return x?1:0;}',
 'if':base+'int main(){X x;if(x)return 0;return 1;}',
 'logical':base+'int main(){X x;return !(x&&true)||!(false||x)||!x;}',
 'branch_effects':base+'int hits;int f(){++hits;return 7;}int bad(){hits+=20;return 9;}int main(){X x;int n=x?f():bad();return n!=7||hits!=1;}',
 'receiver_effect':base+'int hits;X x;X&get(){++hits;return x;}int main(){return (get()?0:1)||hits!=1;}',
 'receiver_comma':base+'int hits;int main(){X x;int n=(++hits,x)?7:9;return n!=7||hits!=1;}',
 'receiver_subscript':base+'int hits;X x[2];int main(){int n=x[hits++]?7:9;return n!=7||hits!=1;}',
 'body_effect':'int hits;struct X{static const int value=7;operator int(){++hits;return value;}};int main(){X x;int n=x;return n!=7||hits!=1;}',
 'body_comma':'int hits;struct X{static const int value=7;operator int(){return (++hits,value);}};int main(){X x;int n=x;return n!=7||hits!=1;}',
 'volatile_read':'volatile int value=7;struct X{operator int(){return value;}};int main(){X x;int a=x;value=9;int b=x;return a!=7||b!=9;}',
 'reference_read':'int v=7;const int&r=v;struct X{operator int(){return r;}};int main(){X x;int a=x;v=9;int b=x;return a!=7||b!=9;}',
 'field_read':'struct X{int value;operator int(){return value;}};int main(){X x={7};int a=x;x.value=9;int b=x;return a!=7||b!=9;}',
 'virtual':'struct X{static const int value=7;virtual operator int(){return value;}};struct Y:X{operator int(){return 9;}};int f(X&x){return x;}int main(){Y y;return f(y)!=9;}',
 'receiver_lifetime':'int hits;struct X{static const bool value=true;X(){++hits;}~X(){hits+=2;}operator bool(){return value;}};int main(){int n=X()?hits:99;return n!=1||hits!=3;}',
 'condition_lifetime':'int hits;struct X{static const bool value=true;X(){++hits;}~X(){hits+=2;}operator bool(){return value;}};int main(){if(X x={}){if(hits!=1)return 1;}return hits!=3;}',
 'reference_temporary':'struct X{static const int value=65543;operator int(){return value;}};int main(){X x;const short&r=x;return r!=7;}',
 'floating_reference':'struct X{static constexpr double value=7.75;operator double(){return value;}};int main(){X x;const int&r=x;return r!=7;}',
 'conditional_reference':base+'int main(){X x;int a=1,b=2;int&r=x?a:b;r=7;return a!=7||b!=2;}',
 'conditional_class':base+'struct A{int n;A(int v):n(v){}};int main(){X x;A a=x?A(7):A(9);return a.n!=7;}',
 'conditional_cleanup':base+'int hits;struct A{int n;A(int v):n(v){++hits;}~A(){--hits;}};int main(){X x;{A a=x?A(7):A(9);if(a.n!=7||hits!=1)return 1;}return hits;}',
 'nested_conditional':base+'int main(){X x;return x?(x?0:1):2;}',
 'no_eager':'template<class T>struct X{operator int(){return T::missing;}};int main(){X<int>x;return sizeof(x)!=1;}',
 'noexcept':base+'static_assert(!noexcept(bool(X())),"");int main(){}',
 'return_conversion':'struct X{static const int value=257;operator unsigned char(){return value;}};int main(){X x;unsigned n=x;return n!=1;}',
 'enum_return':'struct X{enum{value=7};operator int(){return value;}};int main(){X x;return int(x)!=7;}',
 'out_of_class':'struct X{static const int value=7;operator int();};X::operator int(){return value;}int main(){X x;return int(x)!=7;}',
 'lifetime_cast':'int hits;struct X{static const int value=7;~X(){++hits;}operator int(){return value;}};int main(){const long&r=X();return r!=7||hits!=1;}',
})
runner.GOOD.update({
 'return_class_cleanup':base+'int live;struct A{int n;A(int v):n(v){++live;}A(const A&a):n(a.n){++live;}~A(){--live;}};A f(){X x;return x?A(7):A(9);}int main(){{A a=f();if(a.n!=7||live!=1)return 1;}return live;}',
 'return_receiver_cleanup':'int live;struct X{static const bool value=true;X(){++live;}~X(){--live;}operator bool(){return value;}};struct A{int n;A(int v):n(v){}~A(){}};A f(){return X()?A(7):A(9);}int main(){A a=f();return a.n!=7||live;}',
 'conditional_scalar_conversion':base+'struct A{int n;A(int v):n(v){}operator int(){return n;}~A(){}};int main(){X x;int a=x?A(7):A(9);return a!=7;}',
 'void_arms':base+'int n;void f(){n+=1;}void g(){n+=9;}int main(){X x;x?f():g();return n!=1;}',
 'nested_receiver_lifetime':'int live;struct X{static const bool value=false;X(){++live;}~X(){--live;}operator bool(){return value;}};int f(){return X()?(X()?1:2):(X()?3:live);}int main(){return f()!=2||live;}',
 'switch':'struct X{static const int value=7;operator int(){return value;}};int main(){X x;switch(x){case 7:return 0;default:return 1;}}',
 'while':base+'int main(){X x;int n=0;while(x){if(++n==7)break;}return n!=7;}',
 'conditional_cv':'struct X{static const bool value=true;operator bool()const volatile{return value;}};int main(){volatile X x;return x?0:1;}',
 'wrapped_name':'struct X{static const int value=7;operator int(){return (((value)));}};int main(){X x;return int(x)!=7;}',
 'separate_uses':'struct X{static const int value=7;operator int(){return value;}};int n;int f(){X x;n=x;return x.operator int();}int main(){return f()!=7||n!=7;}',
})
runner.BAD.update({
 'not_constexpr':base+'constexpr bool value=X();',
 'not_nttp':base+'template<bool>struct A{};A<X()> a;',
 'deleted':'struct X{static const int value=7;operator int()=delete;};int main(){X x;int n=x;}',
 'private':'class X{static const int value=7;operator int(){return value;}};int main(){X x;int n=x;}',
 'narrow_list':'struct X{static const int value=65543;operator int(){return value;}};int main(){short n{X()};}',
})
if __name__=='__main__':sys.exit(0 if runner.run(Path(sys.argv[1]).resolve(),Path(sys.argv[2])) else 1)
