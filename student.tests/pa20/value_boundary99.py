#!/usr/bin/env python3
"""Separate class argument/result boundaries, including template demand. Run CC WORK."""
from pathlib import Path
import json, re, sys
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'pa18'))
import ordering_controls as runner

runner.GOOD = {
 'move_only': 'struct V{int n;V(int n):n(n){}V(V&&)=default;};V make(int n){return V(n);}int take(V v){return v.n;}int main(){V v=make(7);return v.n!=7||take(make(8))!=8;}',
 'copyable': 'struct V{int n;V(int n):n(n){}};V make(int n){return V(n);}int take(V v){return v.n;}int main(){V v=make(7);return v.n!=7||take(make(8))!=8;}',
 'nontrivial_copy': 'struct V{int n;V(int n):n(n){}V(const V&v):n(v.n+1){}V(V&&)=default;};V make(int n){return V(n);}int take(V v){return v.n;}int main(){V v=make(7);return v.n!=7||take(v)!=8||take(make(9))!=9;}',
 'large_copyable': 'struct V{long a,b,c;V(int n):a(n),b(n+1),c(n+2){}};V make(int n){return V(n);}long take(V v){return v.a+v.b+v.c;}int main(){V v=make(7);return take(v)!=24;}',
 'template_nested_return': 'template<class T>struct V{T n;V(T n):n(n){}V(V&&)=default;};template<class T>V<T>make(T n){return V<T>(n);}template<class T>V<T>forward(T n){return make(n);}int main(){V<int>a=forward(7);V<long>b=forward(8L);V<int>c=forward(9);return a.n!=7||b.n!=8||c.n!=9;}',
 'dependent_alias': 'template<class T>struct V{T n;V(T n):n(n){}V(V&&)=default;};template<class T>struct Owner{typedef V<T> type;static type make(T n){return type(n);}};template<class T,class O=Owner<T>>typename O::type make(T n){return O::make(n);}int main(){auto a=make(7);auto b=make(8L);return a.n!=7||b.n!=8;}',
 'lambda_result': 'struct V{int n;V(int n):n(n){}V(V&&)=default;};int main(){auto f=[](int n)->V{return V(n);};V v=f(7);return v.n!=7;}',
 'return_local': 'struct V{int n;V(int n):n(n){}V(V&&)=default;};V make(int n){V v(n);return v;}int main(){V v=make(7);return v.n!=7;}',
 'return_parameter': 'struct V{int n;V(int n):n(n){}V(V&&)=default;};V pass(V v){return v;}int main(){V v=pass(V(7));return v.n!=7;}',
 'class_member': 'struct V{int n;V(int n):n(n){}V(V&&)=default;};struct S{V v;};S make(int n){return {V(n)};}int main(){S s=make(7);return s.v.n!=7;}',
 'deleted_move_copy_fallback': 'struct V{int n;V(int n):n(n){}V(const V&)=default;};V make(int n){return V(n);}int main(){V v=make(7);return v.n!=7;}',
}
runner.BAD = {
 'copy_deleted_by_move': 'struct V{int n;V(int n):n(n){}V(V&&)=default;};V bad(V&v){return v;}',
 'deleted_move_result': 'struct V{V(){}V(V&&)=delete;};V bad(){return V();}',
}
if __name__=='__main__':
    cc,work=Path(sys.argv[1]).resolve(),Path(sys.argv[2])
    passed=runner.run(cc,work)
    shapes=[]
    for name,indirect in [('move_only',True),('copyable',False),('nontrivial_copy',True),('large_copyable',True)]:
        ir=(work/(name+'.lowir')).read_text()
        make=re.search(r'^function @make\(.*$',ir,re.M)[0]
        take=re.search(r'^function @take\(.*$',ir,re.M)[0]
        good=('pass=indirect_result' in make)==indirect and 'obj<' in take and 'pass=by_address' not in take
        shapes.append(dict(name=name,result=make,parameter=take,passed=good))
        passed &= good
    (work/'boundaries.json').write_text(json.dumps(shapes,indent=2)+'\n')
    sys.exit(0 if passed else 1)
