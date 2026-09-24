#!/usr/bin/env python3
"""Full-expression regions: normal execution plus typed cleanup structure."""
from pathlib import Path
import json, sys
import entity_controls as runner

TRACK = 'int sink;struct S{int n;S(int x):n(x){}~S()noexcept{sink=sink*10+n;}int f()const{return n;}};'
runner.GOOD = {
 'local_call_sum': TRACK+'int run(){S x(1);S y(2);return x.f()+y.f();}int main(){return run()!=3||sink!=21;}',
 'local_nested_calls': TRACK+'int add(int a,int b){return a+b;}int run(){S x(1);return add(x.f(),x.f());}int main(){return run()!=2||sink!=1;}',
 'local_call_condition': TRACK+'int run(){S x(1);if(x.f())return x.f();return 0;}int main(){return run()!=1||sink!=1;}',
 'local_call_comma': TRACK+'int run(){S x(1);return (x.f(),x.f());}int main(){return run()!=1||sink!=1;}',
 'local_call_default': TRACK+'int run(){S x(1);int n=x.f();return n;}int main(){return run()!=1||sink!=1;}',
 'noexcept_temporary_receiver': 'int sink;struct S{S()noexcept{}~S()noexcept{++sink;}S next()const noexcept{return S();}void f()noexcept{++sink;}};int run(){S x;x.next().f();return sink;}int main(){return run()!=2||sink!=3;}',
 'noexcept_temporary_chain': 'int sink;struct S{S()noexcept{}~S()noexcept{++sink;}S next()const noexcept{return S();}void f()noexcept{++sink;}};int run(){S x;x.next().next().f();return sink;}int main(){return run()!=3||sink!=4;}',
 'conditional_no_live_prefix': 'int sink;struct S{S()noexcept{}S(const S&)noexcept{}~S()noexcept{++sink;}};S f()noexcept{return S();}int main(){bool c=true;S s=c?f():f();return sink>1;}',
 'temporary_argument': TRACK+'int read(S const&s){return s.f();}int run(){S x(1);int n=read(S(2));return n+sink;}int main(){return run()!=4||sink!=21;}',
 'loop_locals': TRACK+'int run(){int n=0;for(int i=1;i<3;++i){S x(i);n+=x.f();}return n;}int main(){return run()!=3||sink!=12;}',
}
runner.BAD = {}
if __name__ == '__main__':
 cc, work = Path(sys.argv[1]).resolve(), Path(sys.argv[2])
 ok = runner.run(cc,work)
 checks = {}
 for name in ('local_call_sum','local_nested_calls','noexcept_temporary_receiver','noexcept_temporary_chain'):
  text = (work/(name+'.lowir')).read_text() if (work/(name+'.lowir')).exists() else ''
  checks[name] = all(op in text for op in ('eh_try','eh_end','resume'))
  if name in ('local_call_sum','local_nested_calls'):
   # Both receiver calls belong to the same full-expression region, not
   # merely to two individually protected calls with equal cleanup targets.
   checks[name] &= any(part.split('eh_end',1)[0].count('call i32 @f(') == 2
                       for part in text.split('eh_try')[1:])
 checks['conditional_no_live_prefix'] = 'eh_try' not in (work/'conditional_no_live_prefix.lowir').read_text()
 (work/'structure.json').write_text(json.dumps(checks,indent=2)+'\n')
 print(checks)
 sys.exit(0 if ok and all(checks.values()) else 1)
