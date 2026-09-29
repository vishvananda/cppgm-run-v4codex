#!/usr/bin/env python3
"""Cross-handoff RTTI/list/capture controls. Run CC WORK explicitly."""
from pathlib import Path
import sys
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'pa18'))
import ordering_controls as runner
from list104 import LIB
INFO='namespace std{class type_info{public:bool operator==(const type_info&)const;bool operator!=(const type_info&)const;};}'
BASE='struct B{virtual int f(){return 1;}};struct D:B{int f(){return 2;}};'
runner.GOOD={
 'fixed_static_type':INFO+BASE+'template<int N>bool f(){return typeid(B)==typeid(B);}int main(){return !f<0>()||!f<1>();}',
 'fixed_static_operand':INFO+'int calls;int value(){++calls;return 1;}template<int N>bool f(){return typeid(value())==typeid(int);}int main(){return !f<0>()||!f<1>()||calls;}',
 'fixed_dynamic_operand':INFO+BASE+'template<int N>bool f(B&b){return typeid(b)==typeid(D);}int main(){B b;D d;return f<0>(b)||!f<1>(d);}',
 'fixed_dynamic_call':INFO+BASE+'int calls;B&get(B&b){++calls;return b;}template<int N>bool f(B&b){return typeid(get(b))==typeid(D);}int main(){B b;D d;return f<0>(b)||!f<1>(d)||calls!=2;}',
 'fixed_dynamic_cast':BASE+'template<int N>D*f(B*p){return dynamic_cast<D*>(p);}int main(){B b;D d;return f<0>(&b)!=0||f<1>(&d)!=&d||f<2>(0)!=0;}',
 'fixed_upcast':BASE+'template<int N>B*f(D*p){return dynamic_cast<B*>(p);}int main(){D d;return f<0>(&d)!=&d||f<1>(0)!=0;}',
 'dependent_local_identity':INFO+'template<int N>const std::type_info&f(){struct Local{};return typeid(Local);}int main(){return f<0>()==f<1>()||f<0>()!=f<0>();}',
 'fixed_capture_typeid':INFO+'template<int N>int f(int x){auto g=[x](){return typeid(x)==typeid(int);};return !g();}int main(){return f<0>(1)||f<1>(2);}',
 'fixed_capture_cast':BASE+'template<int N>bool f(B*p){auto g=[p](){return dynamic_cast<D*>(p);};return g()!=0;}int main(){B b;D d;return f<0>(&b)||!f<1>(&d);}',
 'fixed_list_typeid':INFO+LIB+'template<int N>bool f(){std::initializer_list<int>x{1,2};return typeid(x)==typeid(std::initializer_list<int>);}int main(){return !f<0>()||!f<1>();}',
 'nested_list_capture':LIB+'int main(){std::initializer_list<std::initializer_list<int>>x{{1,2},{3,4}};auto f=[x](){int n=0;for(auto row:x)for(auto v:row)n+=v;return n;};return f()!=10;}',
 'list_template_ctor':LIB+'struct S{int n;template<class T>S(std::initializer_list<T>x):n(x.size()){}};int main(){S s{1,2,3};return s.n!=3;}',
 'list_default_argument':LIB+'struct S{int n;S(std::initializer_list<int>x,int extra=7):n(x.size()+extra){}};int main(){S s{1,2,3};return s.n!=10;}',
 'list_query_default':LIB+'struct S{S(std::initializer_list<int>,int=7);};template<class T>decltype(S{1,2}) make(T);int main(){}',
}
runner.BAD={
 'fixed_cast_cv':BASE+'template<int N>D*f(const B*p){return dynamic_cast<D*>(p);}int main(){return f<0>(0)!=0;}',
 'list_deleted_copy':LIB+'struct S{S(){}S(const S&)=delete;};int main(){S s;std::initializer_list<S>x{s};}',
}
if __name__=='__main__':sys.exit(0 if runner.run(Path(sys.argv[1]).resolve(),Path(sys.argv[2])) else 1)
